#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QIODevice>
#include <QTextStream>
#include <QFuture>
#include <QtConcurrent>

#include <zmq.hpp>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

class ZipWordSearcher : public QObject
{
    Q_OBJECT

public:
    explicit ZipWordSearcher(QObject *parent = nullptr);

    void unpackFiles(const QString& zipPath);
    int getTotalFilesCount();
    void findFilesWithWord(const QString& searchWord);
    QFuture<void> findFilesWithWordAsync(const QString& searchWord);
    void abort();

private:
    bool searchWordInFile(QuaZipFile& file, const QString& searchWord);

    QStringList m_fileNames;
    zmq::context_t m_ctx;
    zmq::socket_t m_progressSocket;
    QuaZip m_zip;
    std::atomic<bool> m_aborted;
};
