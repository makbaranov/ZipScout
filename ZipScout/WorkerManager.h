#pragma once

#include <QDebug>
#include <QTimer>
#include <QProcess>
#include <QThread>
#include <QFuture>
#include <QtConcurrent>
#include <zmq.hpp>

class WorkerManager : public QObject {
    Q_OBJECT
public:
    explicit WorkerManager(QObject *parent = nullptr);
    ~WorkerManager();

    bool init();
    void progressListening();
    void startWorker();
    void stopWorker();
    void killWorker();
    void cancelOperation();
    void sendCommand(const QString& cmd);
    void handleResponse(const QString& cmd, const QString& response);
    void testConnection();

public slots:
    void searchInArchive(const QString& zipPath, const QString& filter);
    void createArchive(const QString& sourceZip, const QStringList& files, const QString& destZip);

signals:
    void searchStarted(int totalFiles);
    void creatingStarted();
    void fileProcessed(const QStringList& batchFiles);
    void creatingProcessed(int filesProcessed);
    void searchCompleted();
    void archiveCreated();
    void workerFailed(const QString& error);


private:
    QProcess m_process;
    zmq::context_t m_ctx;
    zmq::socket_t m_cmdSocket;
    zmq::socket_t m_progressSocket;
    bool m_connected = false;

    QFuture<void> m_progressFuture;
    std::atomic<int> m_processedFiles{0};
    std::atomic<bool> m_running{false};
    bool m_initialized = false;
};
