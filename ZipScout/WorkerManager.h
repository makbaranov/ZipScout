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

    QProcess m_process;
};
