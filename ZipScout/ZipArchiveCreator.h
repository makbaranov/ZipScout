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

class ZipArchiveCreator: public QObject
{
    Q_OBJECT

public:
    explicit ZipArchiveCreator(QObject *parent = nullptr);

    bool createResultArchive(const QString& sourceZipPath,
                             const QStringList& filePaths,
                             const QString& resultZipPath);

private:
    bool copyFileToArchive(QuaZip& sourceZip, QuaZip& destZip, const QString& fileName);

    zmq::context_t m_ctx;
    zmq::socket_t m_progressSocket;
};
