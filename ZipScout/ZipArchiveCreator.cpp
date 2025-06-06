#include "ZipArchiveCreator.h"

ZipArchiveCreator::ZipArchiveCreator(QObject *parent) :
    QObject(parent),
    m_ctx(1),
    m_progressSocket(m_ctx, zmq::socket_type::push)
{
    m_progressSocket.connect("tcp://localhost:5556");
}

bool ZipArchiveCreator::createResultArchive(const QString& sourceZipPath, const QStringList& filePaths,
                                          const QString& resultZipPath)
{
    QuaZip sourceZip(sourceZipPath);
    if (!sourceZip.open(QuaZip::mdUnzip)) {
        qDebug() << "Can not open source archive: " << sourceZip.getZipError();
        return false;
    }

    QuaZip destZip(resultZipPath);
    if (!destZip.open(QuaZip::mdCreate)) {
        qDebug() << "Can not open destination archive: " << destZip.getZipError();
        sourceZip.close();
        return false;
    }

    const int BATCH_SIZE = 5000;
    int totalFiles = filePaths.size();
    int processedFiles = 0;
    bool success = true;

    m_aborted.store(false);
    for (const QString& filePath : filePaths) {
        if (m_aborted.load()) {
            break;
        }

        if (!copyFileToArchive(sourceZip, destZip, filePath)) {
            success = false;
            break;
        }

        processedFiles++;

        if (processedFiles % BATCH_SIZE == 0 || processedFiles == totalFiles) {
            QString progressMsg = QString("CREATING_PROGRESS|||%1").arg(processedFiles);
            m_progressSocket.send(zmq::buffer(progressMsg.toStdString()), zmq::send_flags::dontwait);
        }
    }

    destZip.close();
    if (destZip.getZipError() != UNZ_OK) {
        qDebug() << "Close file Error";
        return false;
    }

    sourceZip.close();

    m_progressSocket.send(zmq::buffer("CREATING_DONE"), zmq::send_flags::dontwait);
    return success;
}

bool ZipArchiveCreator::copyFileToArchive(QuaZip& sourceZip, QuaZip& destZip, const QString& fileName)
{
    if (!sourceZip.setCurrentFile(fileName)) {
        qDebug() << "File not found in sorce archive: " << fileName;
        return false;
    }

    QuaZipFile sourceFile(&sourceZip);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Can not open file for reading: " << fileName;
        return false;
    }

    QuaZipFileInfo fileInfo;
    if (!sourceFile.getFileInfo(&fileInfo)) {
        qDebug() << "Can not get file info: " << fileName;
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
        qDebug() <<"Can not create file in destination archive: " << fileName;
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
            qDebug() << "Error file writing: " << fileName;
            destFile.close();
            sourceFile.close();
            return false;
        }
    }

    destFile.close();
    sourceFile.close();

    return true;
}

QFuture<bool> ZipArchiveCreator::createResultArchiveAsync(const QString& sourceZipPath,
                                                          const QStringList& filePaths,
                                                          const QString& resultZipPath)
{
    return QtConcurrent::run([this, sourceZipPath, filePaths, resultZipPath]() {
        return createResultArchive(sourceZipPath, filePaths, resultZipPath);
    });
}

void ZipArchiveCreator::abort()
{
    qDebug() << " called";
    m_aborted.store(true);
}
