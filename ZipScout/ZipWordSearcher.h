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

class ZipWordSearcher : public QObject
{
    Q_OBJECT

public:
    explicit ZipWordSearcher(QObject *parent = nullptr);

    QStringList findFilesWithWord(const QString& zipPath, const QString& searchWord);
    QFuture<QStringList> findFilesWithWordAsync(const QString& zipPath, const QString& searchWord);

    QString lastError() const { return m_lastError; }

signals:
    void fileProcessed(const QString& fileName);
    void searchProgress(int processed, int total);
    void searchFinished(const QStringList& foundFiles);
    void errorOccurred(const QString& error);

private:
    bool searchWordInFile(QuaZipFile& file, const QString& searchWord);

    QString m_lastError;
};
