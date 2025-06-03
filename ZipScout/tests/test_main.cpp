#include <gtest/gtest.h>
#include <QApplication>
#include <QMetaObject>
#include <QMetaMethod>
#include <zmq.hpp>

#include "../ZipWordSearcher.h"
#include "../ZipArchiveCreator.h"
#include "../WorkerManager.h"

class QtTestFixture : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        if (!QApplication::instance()) {
            int argc = 1;
            char* argv[] = {const_cast<char*>("test")};
            app = new QApplication(argc, argv);
        }
    }

    static void TearDownTestSuite() {
    }

    static QApplication* app;
};

QApplication* QtTestFixture::app = nullptr;

TEST_F(QtTestFixture, ZipWordSearcher_Constructor) {
    ZipWordSearcher searcher;
    EXPECT_EQ(searcher.getTotalFilesCount(), 0);
}

TEST_F(QtTestFixture, ZipWordSearcher_AbortMethod) {
    ZipWordSearcher searcher;
    EXPECT_NO_THROW(searcher.abort());
}

TEST_F(QtTestFixture, ZipArchiveCreator_Constructor) {
    ZipArchiveCreator creator;
    EXPECT_NO_THROW(creator.abort());
}

class WorkerManagerTestFixture : public QtTestFixture {
protected:
    void SetUp() override {
        manager = std::make_unique<WorkerManager>();
        manager->init();
        manager->startWorker();
    }

    void TearDown() override {
        manager.reset();
    }

    std::unique_ptr<WorkerManager> manager;
};

TEST_F(WorkerManagerTestFixture, WorkerManager_Start) {
    EXPECT_NO_THROW(manager->abortOperation());
}

TEST_F(WorkerManagerTestFixture, WorkerManager_Commands) {
    EXPECT_NO_THROW(manager->searchInArchive("test.zip", "search_word"));

    QStringList testFiles;
    testFiles << "file1.txt" << "file2.txt";
    EXPECT_NO_THROW(manager->createArchive("source.zip", testFiles, "dest.zip"));
}

TEST_F(WorkerManagerTestFixture, WorkerManager_Signals) {
    const QMetaObject* metaObj = manager->metaObject();

    bool hasSearchStartedSignal = false;
    bool hasWorkerFailedSignal = false;
    bool hasArchiveCreatedSignal = false;

    for (int i = metaObj->methodOffset(); i < metaObj->methodCount(); ++i) {
        QMetaMethod method = metaObj->method(i);
        if (method.methodType() == QMetaMethod::Signal) {
            QString signalName = method.name();
            if (signalName == "searchStarted") hasSearchStartedSignal = true;
            if (signalName == "workerFailed") hasWorkerFailedSignal = true;
            if (signalName == "archiveCreated") hasArchiveCreatedSignal = true;
        }
    }

    EXPECT_TRUE(hasSearchStartedSignal);
    EXPECT_TRUE(hasWorkerFailedSignal);
    EXPECT_TRUE(hasArchiveCreatedSignal);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
