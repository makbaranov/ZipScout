#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QIODevice>
#include <QTextStream>
#include <QFuture>
#include <QtConcurrent>

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
};
