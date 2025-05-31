#include "WorkerManager.h"

WorkerManager::WorkerManager(QObject *parent) :
    QObject(parent),
    m_ctx(1),
    m_cmdSocket(m_ctx, zmq::socket_type::req),
    m_progressSocket(m_ctx, zmq::socket_type::pull),
    m_processedFiles(0)
{
    qDebug() << "WorkerManager conscructor called";
}



WorkerManager::~WorkerManager() {
    qDebug() << "WorkerManager destructor called";
    m_running = false;
    m_progressFuture.waitForFinished();
    m_progressSocket.close();
}

void WorkerManager::init()
{
    connect(&m_process, &QProcess::started, []() {
        qDebug() << "Worker process started";
    });
    connect(&m_process, &QProcess::finished,
            [](int exitCode, QProcess::ExitStatus status) {
                qDebug() << "Worker finished, code:" << exitCode
                         << "status:" << status;
            });
    ;

    m_cmdSocket.connect("tcp://localhost:5555");


    //TODO exclude socket creating
    int port = 5556;
    const int maxPort = 5570;
    while (port <= maxPort) {
        try {
            m_progressSocket.set(zmq::sockopt::linger, 0);
            m_progressSocket.bind("tcp://*:" + std::to_string(port));
            qDebug() << "Successfully bound to port" << port;
            m_connected = true;
            break;
        } catch (const zmq::error_t& e) {
            qWarning() << "Port" << port << "unavailable:" << e.what();
            port++;
        }
    }

    if (!m_connected) {
        qCritical() << "Failed to bind to any port (5556-5560)";
    }

    m_running = true;
    m_progressFuture = QtConcurrent::run([this]() {
        while (m_running) {
            zmq::message_t msg;
            if (m_progressSocket.recv(msg, zmq::recv_flags::dontwait)) {
                QString message = QString::fromStdString(std::string(static_cast<char*>(msg.data()), msg.size()));
                QMetaObject::invokeMethod(QCoreApplication::instance(),
                                          [this, message]() {
                                              auto parts = message.split("|||");
                                              if (parts[0] == "TOTAL_FILES") {
                                                  emit searchStarted(parts[1].toInt());
                                              }
                                              else if (parts[0] == "FILE_PROCESSED") {
                                                  emit fileProcessed(++m_processedFiles);
                                              }
                                          },
                                          Qt::QueuedConnection);
            }
            QThread::msleep(100);
        }
    });
}

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
    m_cmdSocket.send(zmq::buffer(cmd.toStdString()), zmq::send_flags::none);

    zmq::message_t reply;
    if (m_cmdSocket.recv(reply)) {
        QString response = QString::fromStdString(std::string(static_cast<char*>(reply.data()), reply.size()));
        handleResponse(cmd, response);
    }
}

void WorkerManager::handleResponse(const QString& cmd, const QString& response) {
    qDebug() << "Worker response:" << response;

    if (cmd.startsWith("SEARCH")) {
        emit searchCompleted(response.split(";"));
    }
    else if (cmd.startsWith("CREATE_ARCHIVE")) {
        emit archiveCreated(response == "OK");
    }
}

void WorkerManager::testConnection() {
    sendCommand("PING");
}
