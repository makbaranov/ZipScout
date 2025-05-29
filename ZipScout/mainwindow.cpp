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
    connect(&m_workerManager, &WorkerManager::progressUpdated, this, &MainWindow::handleProgressUpdate);

    setButtonsState(false);
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

void MainWindow::handleSearchResults(const QStringList& files)
{
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

void MainWindow::handleProgressUpdate(int progress)
{
    ui->progressBar->setValue(progress);
    logMessage(QString("Progress: %1%").arg(progress));
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
