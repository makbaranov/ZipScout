#include "ZipWordSearcher.h"

ZipWordSearcher::ZipWordSearcher(QObject *parent) :
    QObject(parent),
    m_ctx(1),
    m_progressSocket(m_ctx, zmq::socket_type::push)
{
    m_progressSocket.connect("tcp://localhost:5556");
}

QStringList ZipWordSearcher::findFilesWithWord(const QString& zipPath, const QString& searchWord)
{
    QStringList foundFiles;
    m_lastError.clear();

    QuaZip zip(zipPath);
    if (!zip.open(QuaZip::mdUnzip)) {
        m_lastError = QString("Couldnot open arhive: %1").arg(zip.getZipError());
        emit errorOccurred(m_lastError);
        return foundFiles;
    }

    QStringList fileNames = zip.getFileNameList();
    int totalFiles = fileNames.size();
    m_progressSocket.send(zmq::buffer("TOTAL_FILES|||" + std::to_string(totalFiles)));

    int processedFiles = 0;

    for (const QString& fileName : fileNames) {
        if (!zip.setCurrentFile(fileName)) {
            continue;
        }

        QuaZipFile file(&zip);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }

        if (searchWordInFile(file, searchWord)) {
            foundFiles.append(fileName);
        }

        file.close();

        processedFiles++;
        m_progressSocket.send(zmq::buffer(("FILE_PROCESSED|||" + fileName).toStdString()),  zmq::send_flags::none);

        emit fileProcessed(fileName);
        emit searchProgress(processedFiles, totalFiles);
    }

    zip.close();
    emit searchFinished(foundFiles);
    return foundFiles;
}

QFuture<QStringList> ZipWordSearcher::findFilesWithWordAsync(const QString& zipPath,
                                                             const QString& searchWord)
{
    return QtConcurrent::run([this, zipPath, searchWord]() {
        return findFilesWithWord(zipPath, searchWord);
    });
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
