#include "WorkerManager.h"

WorkerManager::WorkerManager(QObject *parent) :
    QObject(parent),
    m_ctx(1),
    m_cmdSocket(m_ctx, zmq::socket_type::req),
    m_progressSocket(m_ctx, zmq::socket_type::pull)
{
    qDebug() << "WorkerManager conscructor called";
}



WorkerManager::~WorkerManager() {
    qDebug() << "WorkerManager destructor called";
    stopWorker();
    m_running = false;
    m_progressFuture.waitForFinished();
    m_progressSocket.close();
}

bool WorkerManager::init()
{
    if (m_initialized) {
        qWarning() << "Already initialized";
        return true;
    }

    try {
        m_cmdSocket.connect("tcp://localhost:5555");
    } catch (const zmq::error_t& e) {
        qCritical() << "Command socket error:" << e.what();
        return false;
    }

    int port = 5556;
    while (port <= 5570) {
        try {
            m_progressSocket.bind("tcp://*:" + std::to_string(port));
            qDebug() << "Successfully bound to port" << port;
            break;
        } catch (const zmq::error_t& e) {
            qWarning() << "Port" << port << "unavailable:" << e.what();
            port++;
        }
    }
    if (port > 5570) {
        qCritical() << "Failed to bind progress socket";
        return false;
    }


    connect(&m_process, &QProcess::started, []() {
        qDebug() << "Worker process started";
    });
    connect(&m_process, &QProcess::finished,
            [](int exitCode, QProcess::ExitStatus status) {
                qDebug() << "Worker finished, code:" << exitCode << "status:" << status;
            });

    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        qCritical() << "Worker process error:" << error;
        emit workerFailed("Process error: " + QString::number(error));
    });

    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this](int exitCode, QProcess::ExitStatus status) {
                if (status == QProcess::CrashExit) {
                    qCritical() << "Worker crashed! Exit code:" << exitCode;
                    emit workerFailed("Worker crashed with code: " + QString::number(exitCode));
                }
            });

    progressListening();

    m_initialized = true;
    return true;
}

void WorkerManager::progressListening()
{
    m_running = true;
    m_processedFiles.store(0);

    m_progressFuture = QtConcurrent::run([this]() {
        zmq::pollitem_t items[] = {{static_cast<void*>(m_progressSocket), 0, ZMQ_POLLIN, 0}};

        while (m_running.load()) {
            zmq::poll(items, 1, 100);

            if (items[0].revents & ZMQ_POLLIN) {
                zmq::message_t msg;
                if (m_progressSocket.recv(msg)) {
                    QString message = QString::fromStdString(
                        std::string(static_cast<char*>(msg.data()), msg.size()));

                    auto parts = message.split("|||");
                    if (parts[0] == "FILES_PROCESSED") {
                        emit fileProcessed(parts[1].split("##"));
                    }
                    else if (parts[0].contains("FINISHED")) {
                        emit searchCompleted();
                    }
                    else if (parts[0].contains("CREATING_PROGRESS")) {
                        emit creatingProcessed(parts[1].toInt());
                    }
                    else if (parts[0].contains("CREATING_DONE")) {
                        emit archiveCreated();
                    }
                }
            }
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

        if (!m_process.waitForStarted(3000)) {
            qCritical() << "Failed to start worker process";
            emit workerFailed("Process failed to start");
            return;
        }

        QTimer::singleShot(500, this, &WorkerManager::testConnection);
    }
}


void WorkerManager::stopWorker() {
    sendCommand("STOP");
    m_process.waitForFinished(1000);
}

void WorkerManager::killWorker() {
    if (m_process.state() == QProcess::Running) {
        m_process.terminate();
        if (!m_process.waitForFinished(1000)) {
            m_process.kill();
        }
    }
}

void WorkerManager::cancelOperation() {
    qDebug() << "cancelOpearation";
    sendCommand("CANCEL");
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
        auto message = response.split("|||");
        if (message[0] == "STARTED" && !message[1].isEmpty()){
            emit searchStarted(message[1].toInt());
        }
    }
    else if (cmd.startsWith("CREATE_ARCHIVE")) {
        if (response.contains("STARTED"))
            emit creatingStarted();
    }
}

void WorkerManager::testConnection() {
    sendCommand("HELLO");
}
