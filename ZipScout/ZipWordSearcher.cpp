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
        qDebug() << "Couldnot open arhive: " << m_zip.getZipError();
        return;
    }

    m_fileNames = m_zip.getFileNameList();
}

int ZipWordSearcher::getTotalFilesCount() {
    return m_fileNames.size();
}

void ZipWordSearcher::findFilesWithWord(const QString& searchWord)
{
    QStringList foundFilesWithMetaData;

    const int BATCH_SIZE = 10000;
    int totalFiles = m_fileNames.size();
    int processedFiles = 0;
    int batchProcessed = 0;

    m_canceled.store(false);
    for (const QString& fileName : m_fileNames) {
        if (m_canceled.load()) {
            break;
        }

        if (!m_zip.setCurrentFile(fileName)) {
            continue;
        }

        QuaZipFile file(&m_zip);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }

        bool isFound = searchWordInFile(file, searchWord);

        if (isFound) {
            QuaZipFileInfo fileInfo;
            file.getFileInfo(&fileInfo);
            QString fileMetadata = QString("%1,%2,%3")
                                       .arg(fileName)
                                       .arg(fileInfo.uncompressedSize) // Размер файла
                                       .arg(fileInfo.dateTime.toString(Qt::ISODate)); // Время в ISO формате

            foundFilesWithMetaData.append(fileMetadata);
        }

        file.close();

        processedFiles++;
        batchProcessed++;

        if (batchProcessed >= BATCH_SIZE || processedFiles == totalFiles) {
            QString message = QString("FILES_PROCESSED|||%1##%2")
                .arg(processedFiles)
                .arg(foundFilesWithMetaData.join(";"));

            m_progressSocket.send(
                zmq::buffer(message.toStdString()),
                zmq::send_flags::none
            );

            batchProcessed = 0;
            foundFilesWithMetaData.clear();
        }
    }

    m_zip.close();

    m_progressSocket.send(
        zmq::buffer(QString("FINISHED").toStdString()),
        zmq::send_flags::none
    );
}

QFuture<void> ZipWordSearcher::findFilesWithWordAsync(const QString& searchWord)
{
    return QtConcurrent::run([this, searchWord]() {
        return findFilesWithWord(searchWord);
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

void ZipWordSearcher::cancel()
{
    qDebug() << "cancel called";
    m_canceled.store(true);
}

