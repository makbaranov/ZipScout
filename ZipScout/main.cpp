#include "mainwindow.h"
#include <QCommandLineParser>
#include <QDebug>
#include <QApplication>
#include <QEventLoop>
#include <QObject>

#include "WorkerManager.h"

#include <zmq.hpp>


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
        qDebug() << "Console mode";

        if (!parser.isSet("source") || !parser.isSet("destination") || !parser.isSet("filter")) {
            qCritical() << "Error: --source, --destination and --filter must be provided for Console mode";
            return 1;
        }

        WorkerManager manager;
        if (!manager.init()) {
            qCritical() << "Failed to initialize worker";
            return 1;
        }
        manager.startWorker();

        QString source = parser.value("source");
        QString dest = parser.value("destination");
        QString filter = parser.value("filter");

        qDebug() << "Source:" << source;
        qDebug() << "Destination:" << dest;
        qDebug() << "Filter:" << filter;

        QEventLoop loop;
        QStringList foundFiles;
        bool operationSuccess = false;

        QObject::connect(&manager, &WorkerManager::searchStarted, [](int totalFiles) {
            qDebug() << "Search started. Total files:" << totalFiles;
        });

        QObject::connect(&manager, &WorkerManager::creatingProcessed, [&foundFiles](int filesProcessed) {
            qDebug() << "Archived" << filesProcessed << "/" << foundFiles.size() << "files";
        });

        QObject::connect(&manager, &WorkerManager::searchCompleted, [&]() {
            qDebug() << "Search completed. Starting archive creation...";
            manager.createArchive(source, foundFiles, dest);
        });

        QObject::connect(&manager, &WorkerManager::archiveCreated, [&]() {
            qDebug() << "Archive created successfully";
            operationSuccess = true;
            loop.quit();
        });

        QObject::connect(&manager, &WorkerManager::workerFailed, [&](const QString& error) {
            qCritical() << "Error:" << error;
            loop.quit();
        });

        QObject::connect(&manager, &WorkerManager::fileProcessed, [&](const QStringList& batch) {
            auto progress = batch[0].toInt();
            auto files = batch[1].split(";");
            auto foundInBatch = files.size();
            for (auto &file : files) {
                file = file.split(",")[0];
            }
            foundFiles.append(files);
            qDebug() << "Processed:" << progress << "files | Found:" << foundInBatch << "in batch";
        });

        manager.searchInArchive(source, filter);
        loop.exec();

        if (!operationSuccess) {
            qCritical() << "Operation failed";
            return 1;
        }

        qDebug() << "Done";
        return 0;
    }

    if (parser.isSet("test")) {
        qDebug() << "Test mode";
    }

    MainWindow w;
    w.show();
    return a.exec();
}
