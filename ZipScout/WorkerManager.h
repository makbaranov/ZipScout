#pragma once

#include <QDebug>
#include <QTimer>
#include <QProcess>

#include <zmq.hpp>

class WorkerManager : public QObject {
    Q_OBJECT
public:
    explicit WorkerManager(QObject *parent = nullptr);

    void startWorker();
    void stopWorker();
    void sendCommand(const QString& cmd);
    void testConnection();

public slots:
    void searchInArchive(const QString& zipPath, const QString& filter);
    void createArchive(const QString& sourceZip, const QStringList& files, const QString& destZip);

signals:
    void searchCompleted(const QStringList& foundFiles);
    void archiveCreated(bool success);

private:
    QProcess m_process;
};
