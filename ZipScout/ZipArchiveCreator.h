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

    QString lastError() const { return m_lastError; }

signals:
    void archiveProgress(int processed, int total);
    void archiveFinished(bool success);
    void errorOccurred(const QString& error);

private:
    bool copyFileToArchive(QuaZip& sourceZip, QuaZip& destZip, const QString& fileName);
    QString m_lastError;
};
