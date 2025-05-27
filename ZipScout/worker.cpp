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


    // ZipWordSearcher searcher;
    // auto result = searcher.findFilesWithWord(source, filter);
    // qDebug() << result;

    // ZipArchiveCreator creator;
    // creator.createResultArchive(source, result, dest);

    while (true) {
        zmq::message_t request;
        auto res = socket.recv(request);
        if (!res) {
            qWarning() << "Receive failed";
            continue;
        }

        std::string msg(static_cast<char*>(request.data()), request.size());
        qDebug() << "Received command:" << QString::fromStdString(msg);

        if (msg == "PING") {
            socket.send(zmq::buffer("PONG"), zmq::send_flags::none);
        }
        else if (msg == "STOP") {
            socket.send(zmq::buffer("STOP_ACK"), zmq::send_flags::none);
            QMetaObject::invokeMethod(&app, "quit", Qt::QueuedConnection);
            break;
        }
        else {
            socket.send(zmq::buffer("Unknown command"), zmq::send_flags::none);
        }
    }

    qDebug() << "Worker shutting down";
    return 0;
}
