#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->selectFileButton, &QPushButton::clicked, this, &MainWindow::onSelectFileClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::onSaveClicked);

    connect(&m_workerManager, &WorkerManager::searchCompleted, this, &MainWindow::handleSearchResults);
    connect(&m_workerManager, &WorkerManager::archiveCreated, this, &MainWindow::handleArchiveCreated);
    connect(&m_workerManager, &WorkerManager::searchStarted, this, &MainWindow::handleSearchStarted);
    connect(&m_workerManager, &WorkerManager::fileProcessed, this, &MainWindow::handleFileProcessed);

    m_workerManager.init();

    setButtonsState(false);
    m_totalFiles = 0;
    logMessage("ZipScout is ready");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSelectFileClicked()
{
    QFileDialog fileDialog(this);
    fileDialog.setWindowTitle("Choose ZIP-file");
    fileDialog.setNameFilter("ZIP Archives (*.zip)");
    fileDialog.setFileMode(QFileDialog::ExistingFile);

    if (fileDialog.exec()) {
        m_currentArchivePath = fileDialog.selectedFiles().first();
        QString fileName = QFileInfo(m_currentArchivePath).fileName();

        ui->fileLabel->setText("Analysing archive: " + fileName);
        logMessage("Analysis started: " + fileName);

        setButtonsState(true);
        ui->progressBar->setValue(0);

        m_workerManager.searchInArchive(m_currentArchivePath, "secret"); //TODO find filter except f hardcode
    }
}

void MainWindow::handleSearchStarted(int totalFiles)
{
    qDebug() << "handleSearchStarted";
    logMessage("Search started");
    m_totalFiles = totalFiles;
    ui->progressBar->setMaximum(totalFiles);
    ui->progressBar->setFormat("%v of %m files frocessed");
}

void MainWindow::handleFileProcessed(int current)
{
    qDebug() << "handleFileProcessed";
    ui->progressBar->setValue(current);
    logMessage(QString("Processed file %1/%2").arg(current).arg(m_totalFiles));
}

void MainWindow::handleSearchResults(const QStringList& files)
{
    qDebug() << "handleSearchResults";
    m_foundFiles = files;
    logMessage(QString("%1 files found").arg(m_foundFiles.size()));
    setButtonsState(false);

    //TODO fill table of files
}

void MainWindow::handleArchiveCreated(bool success)
{
    if (success) {
        logMessage("Archive succesfully created");
    } else {
        logMessage("Error occured while creating archive");
    }
}

void MainWindow::onCancelClicked()
{
    logMessage("Operation cancelled");
    m_workerManager.stopWorker();
    setButtonsState(false);
}

void MainWindow::onClearClicked()
{
    ui->fileLabel->setText("Analysing archive:: N/A");
    ui->progressBar->setValue(0);
    ui->logTextEdit->clear();
    m_totalFiles = 0;
    logMessage("Cleared");
}

void MainWindow::onSaveClicked()
{
    if (!m_foundFiles.isEmpty()) {
        QString savePath = QFileDialog::getSaveFileName(this, "Save results", "", "ZIP Archives (*.zip)");

        if (!savePath.isEmpty()) {
            logMessage("Creating archive...");
            setButtonsState(true);
            m_workerManager.createArchive(m_currentArchivePath, m_foundFiles, savePath);
        }
    }
}

void MainWindow::logMessage(const QString& msg)
{
    ui->logTextEdit->appendPlainText(QDateTime::currentDateTime().toString() + ": " + msg);
}

void MainWindow::setButtonsState(bool isWorking)
{
    ui->selectFileButton->setEnabled(!isWorking);
    ui->cancelButton->setEnabled(isWorking);
    ui->clearButton->setEnabled(!isWorking);
    ui->saveButton->setEnabled(!isWorking && m_foundFiles.size() > 0);
}
