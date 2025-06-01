#include "ZipArchiveCreator.h"

ZipArchiveCreator::ZipArchiveCreator(QObject *parent)
    : QObject(parent)
{
}

bool ZipArchiveCreator::createResultArchive(const QString& sourceZipPath, const QStringList& filePaths,
                                          const QString& resultZipPath)
{
    m_lastError.clear();

    QuaZip sourceZip(sourceZipPath);
    if (!sourceZip.open(QuaZip::mdUnzip)) {
        m_lastError = QString("Can not open source archive: %1").arg(sourceZip.getZipError());
        emit errorOccurred(m_lastError);
        return false;
    }

    QuaZip destZip(resultZipPath);
    if (!destZip.open(QuaZip::mdCreate)) {
        m_lastError = QString("Can not open destination archive: %1").arg(destZip.getZipError());
        emit errorOccurred(m_lastError);
        sourceZip.close();
        return false;
    }

    int totalFiles = filePaths.size();
    int processedFiles = 0;
    bool success = true;

    for (const QString& filePath : filePaths) {
        if (!copyFileToArchive(sourceZip, destZip, filePath)) {
            success = false;
            break;
        }

        processedFiles++;
        emit archiveProgress(processedFiles, totalFiles);
    }

    destZip.close();
    if (destZip.getZipError() != UNZ_OK) {
        m_lastError = "Close file Error";
        return false;
    }

    sourceZip.close();

    emit archiveFinished(success);
    return success;
}

bool ZipArchiveCreator::copyFileToArchive(QuaZip& sourceZip, QuaZip& destZip, const QString& fileName)
{
    if (!sourceZip.setCurrentFile(fileName)) {
        m_lastError = QString("File not found in sorce archive: %1").arg(fileName);
        emit errorOccurred(m_lastError);
        return false;
    }

    QuaZipFile sourceFile(&sourceZip);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        m_lastError = QString("Can not open file for reading: %1").arg(fileName);
        emit errorOccurred(m_lastError);
        return false;
    }

    QuaZipFileInfo fileInfo;
    if (!sourceFile.getFileInfo(&fileInfo)) {
        m_lastError = QString("Can not get file info: %1").arg(fileName);
        emit errorOccurred(m_lastError);
        sourceFile.close();
        return false;
    }

    QuaZipFile destFile(&destZip);
    QuaZipFileInfo newFileInfo;
    newFileInfo.name = fileName;
    newFileInfo.dateTime = fileInfo.dateTime;
    newFileInfo.externalAttr = fileInfo.externalAttr;
    newFileInfo.uncompressedSize = fileInfo.uncompressedSize;

    if (!destFile.open(QIODevice::WriteOnly, newFileInfo)) {
        m_lastError = QString("Can not create file in destination archive: %1").arg(fileName);
        emit errorOccurred(m_lastError);
        sourceFile.close();
        return false;
    }

    QByteArray buffer;
    buffer.resize(10 * 1024); //10 kb max file size

    while (!sourceFile.atEnd()) {
        qint64 bytesRead = sourceFile.read(buffer.data(), buffer.size());
        if (bytesRead <= 0) {
            break;
        }

        if (destFile.write(buffer.data(), bytesRead) != bytesRead) {
            m_lastError = QString("Error file writing: %1").arg(fileName);
            emit errorOccurred(m_lastError);
            destFile.close();
            sourceFile.close();
            return false;
        }
    }

    destFile.close();
    sourceFile.close();

    return true;
}
