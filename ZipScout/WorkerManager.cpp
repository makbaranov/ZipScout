#include "WorkerManager.h"

void WorkerManager::searchInArchive(const QString& zipPath, const QString& filter) {
    QString cmd = QString("SEARCH|||%1|||%2").arg(zipPath).arg(filter);
    sendCommand(cmd);
}

void WorkerManager::createArchive(const QString& sourceZip, const QStringList& files, const QString& destZip) {
    QString cmd = QString("CREATE_ARCHIVE|||%1|||%2|||%3")
    .arg(sourceZip)
        .arg(files.join(";"))
        .arg(destZip);
    sendCommand(cmd);
}

WorkerManager::WorkerManager(QObject *parent) : QObject(parent) {
    connect(&m_process, &QProcess::started, []() {
        qDebug() << "Worker process started";
    });
    connect(&m_process, &QProcess::finished,
            [](int exitCode, QProcess::ExitStatus status) {
                qDebug() << "Worker finished, code:" << exitCode
                         << "status:" << status;
            });
}

void WorkerManager::startWorker() {
    if (m_process.state() == QProcess::NotRunning) {
        m_process.start("./ZipScoutWorker");
        QTimer::singleShot(500, this, &WorkerManager::testConnection);
    }
}

void WorkerManager::stopWorker() {
    sendCommand("STOP");
    m_process.waitForFinished(1000);
}

void WorkerManager::sendCommand(const QString& cmd) {
    zmq::context_t ctx;
    zmq::socket_t socket(ctx, zmq::socket_type::req);
    socket.connect("tcp://localhost:5555");
    socket.send(zmq::buffer(cmd.toStdString()), zmq::send_flags::none);

    zmq::message_t reply;
    if (socket.recv(reply)) {
        QString response = QString::fromStdString(
            std::string(static_cast<char*>(reply.data()), reply.size()));

        qDebug() << "Worker response:" << response;

        if (cmd.startsWith("SEARCH")) {
            emit searchCompleted(response.split(";"));
        }
        else if (cmd.startsWith("CREATE_ARCHIVE")) {
            emit archiveCreated(response == "OK");
        }
    }
}

void WorkerManager::testConnection() {
    sendCommand("PING");
}
