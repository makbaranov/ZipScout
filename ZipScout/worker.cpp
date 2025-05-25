#include <QCoreApplication>
#include <zmq.hpp>
#include <QDebug>

int main(int argc, char *argv[])
{
    zmq::context_t ctx(1); // 1 - number of IO streams
    zmq::socket_t socket(ctx, zmq::socket_type::rep);
    socket.bind("tcp://*:5555");

    qDebug() << "Worker started";


    return 0;
}
