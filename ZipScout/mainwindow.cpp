#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->filesTableView->setModel(&m_filesModel);
    ui->filesTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->filesTableView->horizontalHeader()->setStretchLastSection(true);
    ui->filesTableView->sortByColumn(FoundFilesModel::COLUMN_FILENAME, Qt::AscendingOrder);

    connect(ui->selectFileButton, &QPushButton::clicked, this, &MainWindow::onSelectFileClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::onSaveClicked);

    connect(&m_workerManager, &WorkerManager::searchCompleted, this, &MainWindow::handleSearchResults);
    connect(&m_workerManager, &WorkerManager::archiveCreated, this, &MainWindow::handleArchiveCreated);
    connect(&m_workerManager, &WorkerManager::searchStarted, this, &MainWindow::handleSearchStarted);
    connect(&m_workerManager, &WorkerManager::fileProcessed, this, &MainWindow::handleFileProcessed);

    m_workerManager.init();

    setButtonsState(Ready);
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

        setButtonsState(InProgress);
        ui->progressBar->setValue(0);

        m_workerManager.searchInArchive(m_currentArchivePath, "secret"); //TODO find filter instead of hardcode
    }
}

void MainWindow::handleSearchStarted(int totalFiles)
{
    logMessage("Search started");
    m_totalFiles = totalFiles;
    ui->progressBar->setMaximum(totalFiles);
    ui->progressBar->setFormat("%v of %m files frocessed");
}

void MainWindow::handleFileProcessed(int current)
{
    ui->progressBar->setValue(current);
    logMessage(QString("Processed file %1/%2").arg(current).arg(m_totalFiles));
}

void MainWindow::handleSearchResults(const QStringList& files)
{
    qDebug() << "handleSearchResults";
    m_foundFiles = files;
    logMessage(QString("%1 files found").arg(m_foundFiles.size()));
    setButtonsState(m_foundFiles.isEmpty() ? Ready : Done);

    QVector<FoundFile> foundFiles;
    foundFiles.reserve(files.size());

    for (const QString& filePath : files) {
        FoundFile file;
        file.include = true;
        file.filePath = filePath;
        file.size = 1024; //TODO replace dummy
        file.modified = QDateTime::currentDateTime(); //TODO replace dummy

        foundFiles.append(file);
    }

    m_filesModel.updateData(foundFiles);

    ui->filesTableView->resizeColumnsToContents();
    ui->filesTableView->horizontalHeader()->setSectionResizeMode(
        FoundFilesModel::COLUMN_FILENAME, QHeaderView::Stretch);
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
    setButtonsState(Ready);
}

void MainWindow::onClearClicked()
{
    ui->fileLabel->setText("Analysing archive:: N/A");
    ui->progressBar->setValue(0);
    ui->logTextEdit->clear();
    m_totalFiles = 0;

    m_filesModel.updateData(QVector<FoundFile>());
    m_currentArchivePath.clear();
    m_foundFiles.clear();

    ui->tabWidget->setCurrentIndex(0);
    setButtonsState(Ready);

    logMessage("Cleared");
}

void MainWindow::onSaveClicked()
{
    QStringList filesToSave = m_filesModel.getCheckedFiles();

    if (filesToSave.isEmpty()) {
        logMessage("No files selected for saving");
        return;
    }

    QString savePath = QFileDialog::getSaveFileName(this, "Save results", "", "ZIP Archives (*.zip)");

    if (!savePath.isEmpty()) {
        logMessage(QString("Creating archive with %1 files...").arg(filesToSave.size()));
        setButtonsState(Ready);
        m_workerManager.createArchive(m_currentArchivePath, filesToSave, savePath);
    }
}

void MainWindow::logMessage(const QString& msg)
{
    ui->logTextEdit->appendPlainText(QDateTime::currentDateTime().toString() + ": " + msg);
}

void MainWindow::setButtonsState(AppState state)
{
    ui->progressBar->setVisible(state != Ready);
    ui->fileLabel->setVisible(state != Ready);

    ui->selectFileButton->setEnabled(state == Ready);
    ui->cancelButton->setEnabled(state == InProgress);
    ui->clearButton->setEnabled(state != InProgress);
    ui->saveButton->setEnabled(state == Done && !m_foundFiles.isEmpty());
}
