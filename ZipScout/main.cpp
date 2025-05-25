#include "mainwindow.h"
#include <QCommandLineParser>
#include <QDebug>
#include <QApplication>

#include "ZipWordSearcher.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCommandLineParser parser;
    parser.addOption({"test", "Test mod"});

    parser.addOption({"nogui", "Console mod"});
    parser.addOption({"source", "Sorce file", "path"});
    parser.addOption({"destination", "Destination file", "path"});
    parser.addOption({"filter", "Filter, word to search", "string"});


    parser.process(a);

    if (parser.isSet("nogui")) {
        qDebug() << "Console mod";

        if (!parser.isSet("source") || !parser.isSet("destination")) {
            qCritical() << "Error: --source --destination and --filter must be provided for Console mod";
            return 1;
        }

        QString source = parser.value("source");
        QString dest = parser.value("destination");
        QString filter = parser.value("filter");

        qDebug() << "Source:" << source;
        qDebug() << "Destination:" << dest;
        qDebug() << "Filter:" << filter;

        ZipWordSearcher searcher;
        auto result = searcher.findFilesWithWord(source, filter);
        qDebug() << result;

        return 0;
    }

    if (parser.isSet("test")) {
        qDebug() << "Test mod";
    }

    MainWindow w;
    w.show();
    return a.exec();
}
