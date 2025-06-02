#include <QCoreApplication>
#include <QDebug>
#include <zmq.hpp>

#include "ZipWordSearcher.h"
#include "ZipArchiveCreator.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    zmq::context_t ctx(1);
    zmq::socket_t socket(ctx, zmq::socket_type::rep);
    socket.bind("tcp://*:5555");

    qDebug() << "Worker ready. Listening on tcp://*:5555";

    ZipWordSearcher searcher;

    while (true) {
        zmq::message_t request;
        auto res = socket.recv(request);
        if (!res) {
            qWarning() << "Receive failed";
            continue;
        }

        QString msg = QString::fromStdString(std::string(static_cast<char*>(request.data()), request.size()));

        qDebug() << "Received command:" << msg;

        if (msg == "PING") {
            socket.send(zmq::buffer("PONG"), zmq::send_flags::none);
        }
        else if (msg == "STOP") {
            socket.send(zmq::buffer("STOPED"), zmq::send_flags::none);
            QMetaObject::invokeMethod(&app, "quit", Qt::QueuedConnection);
            break;
        }
        else if (msg == "CANCEL") {
            socket.send(zmq::buffer("CANCELED"), zmq::send_flags::none);
            searcher.cancel();
        }
        else if (msg.startsWith("SEARCH")) {
            auto parts = msg.split("|||");
            if (parts.size() == 3) {
                searcher.unpackFiles(parts[1]);
                QString response("STARTED|||" + QString::number(searcher.getTotalFilesCount()));
                socket.send(zmq::buffer(response.toStdString()), zmq::send_flags::none);
                searcher.findFilesWithWordAsync(parts[2]);
            }
        }
        else if (msg.startsWith("CREATE_ARCHIVE")) {
            auto parts = msg.split("|||");
            if (parts.size() == 4) {
                ZipArchiveCreator creator;
                socket.send(zmq::buffer("STARTED"), zmq::send_flags::none);
                creator.createResultArchive(parts[1], parts[2].split(";"), parts[3]);
            }
        }
        else {
            socket.send(zmq::buffer("Unknown command"), zmq::send_flags::none);
        }
    }

    qDebug() << "Worker shutting down";
    return 0;
}
