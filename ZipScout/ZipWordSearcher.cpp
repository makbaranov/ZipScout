#include "ZipWordSearcher.h"

ZipWordSearcher::ZipWordSearcher(QObject *parent) :
    QObject(parent),
    m_ctx(1),
    m_progressSocket(m_ctx, zmq::socket_type::push)
{
    m_progressSocket.connect("tcp://localhost:5556");
}

void ZipWordSearcher::unpackFiles(const QString& zipPath)
{
    m_zip.setZipName(zipPath);
    if (!m_zip.open(QuaZip::mdUnzip)) {
        m_lastError = QString("Couldnot open arhive: %1").arg(m_zip.getZipError());
        emit errorOccurred(m_lastError);
        return;
    }

    m_fileNames = m_zip.getFileNameList();
}

int ZipWordSearcher::getTotalFilesCount() {
    return m_fileNames.size();
}

QStringList ZipWordSearcher::findFilesWithWord(const QString& searchWord)
{
    QStringList foundFiles;
    m_lastError.clear();

    int totalFiles = getTotalFilesCount();
    int processedFiles = 0;

    for (const QString& fileName : m_fileNames) {
        if (!m_zip.setCurrentFile(fileName)) {
            continue;
        }

        QuaZipFile file(&m_zip);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }

        if (searchWordInFile(file, searchWord)) {
            foundFiles.append(fileName);
        }

        file.close();

        processedFiles++;
        m_progressSocket.send(zmq::buffer(("FILE_PROCESSED|||" + fileName).toStdString()),  zmq::send_flags::none);
    }

    m_zip.close();

    m_progressSocket.send(zmq::buffer("FINISHED"), zmq::send_flags::none);
    return foundFiles;
}

bool ZipWordSearcher::searchWordInFile(QuaZipFile& file, const QString& searchWord)
{
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.contains(searchWord, Qt::CaseSensitive)) {
            return true;
        }
    }

    return false;
}
