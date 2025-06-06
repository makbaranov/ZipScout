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
    connect(ui->abortButton, &QPushButton::clicked, this, &MainWindow::onAbortClicked);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::onSaveClicked);

    connect(&m_workerManager, &WorkerManager::searchCompleted, this, &MainWindow::handleSearchCompleted);
    connect(&m_workerManager, &WorkerManager::archiveCreated, this, &MainWindow::handleArchiveCreated);
    connect(&m_workerManager, &WorkerManager::searchStarted, this, &MainWindow::handleSearchStarted);
    connect(&m_workerManager, &WorkerManager::searchProgressStarted, this, &MainWindow::handleProgressStarted);
    connect(&m_workerManager, &WorkerManager::creatingStarted, this, &MainWindow::handleCreatingStarted);
    connect(&m_workerManager, &WorkerManager::fileProcessed, this, &MainWindow::handleFileProcessed);
    connect(&m_workerManager, &WorkerManager::creatingProcessed, this, &MainWindow::handleCreatingProcessed);

    connect(&m_workerManager, &WorkerManager::workerFailed, this, &MainWindow::handleWorkerFailed);



    m_workerManager.init();
    m_workerManager.startWorker();

    setButtonsState(Ready);
    m_totalFiles = 0;
    m_currentFile = 0;
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

        m_workerManager.searchInArchive(m_currentArchivePath, ui->searchWordEdit->text());
        logMessage("Search started");
    }
}

void MainWindow::handleCreatingStarted()
{
    handleProgressStarted(m_numberFilesToSave);
}


void MainWindow::handleSearchStarted()
{
    m_currentFile = 0;
    ui->progressBar->setMaximum(0);
    ui->progressBar->setFormat("%v of %m files processed");
}


void MainWindow::handleProgressStarted(int totalFiles)
{
    ui->progressBar->setMaximum(totalFiles);
    m_totalFiles = totalFiles;
}

void MainWindow::handleCreatingProcessed(int filesProcessed) {
    m_currentFile = filesProcessed;
    ui->progressBar->setValue(m_currentFile);
    logMessage(QString("Processed file %1/%2").arg(m_currentFile).arg(m_numberFilesToSave));
}

void MainWindow::handleFileProcessed(const QStringList& batchFiles)
{
    m_currentFile = batchFiles[0].toInt();
    auto filesWithMetadata = batchFiles[1].split(";");

    QVector<FoundFile> foundFiles;
    foundFiles.reserve(filesWithMetadata.size());

    for (const QString& item : filesWithMetadata) {
        auto metadata = item.split(",");
        FoundFile file;
        file.include = true;
        file.filePath = metadata[0];
        file.size = metadata[1].toLongLong();
        file.modified = QDateTime::fromString(metadata[2], Qt::ISODate);

        foundFiles.append(file);
    }

    m_filesModel.addFiles(foundFiles);
    m_foundFiles.append(filesWithMetadata);

    ui->filesTableView->resizeColumnsToContents();
    ui->filesTableView->horizontalHeader()->setSectionResizeMode(
        FoundFilesModel::COLUMN_FILENAME, QHeaderView::Stretch);

    ui->progressBar->setValue(m_currentFile);
    logMessage(QString("Processed file %1/%2").arg(m_currentFile).arg(m_totalFiles));
}


void MainWindow::handleSearchCompleted()
{
    logMessage(QString("Files found: %1").arg(m_foundFiles.size()));
    setButtonsState(m_foundFiles.isEmpty() ? Ready : Done);
}

void MainWindow::handleArchiveCreated()
{
    logMessage("Archive succesfully created");
    setButtonsState(Done);
    ui->fileLabel->setText(ui->fileLabel->text() + " is done!");
}

void MainWindow::handleWorkerFailed(const QString& error)
{
    logMessage("Worker Error: Worker process failed:\n" + error + "\nTry restarting the application.");
    setButtonsState(Ready);

    m_workerManager.killWorker();
    QTimer::singleShot(1000, &m_workerManager, &WorkerManager::startWorker);
}

void MainWindow::onAbortClicked()
{
    logMessage("Operation aborted");
    setButtonsState(Aborted);
    m_workerManager.abortOperation();
}

void MainWindow::onClearClicked()
{
    ui->fileLabel->setText("Analysing archive:: N/A");
    ui->progressBar->setValue(0);
    ui->logTextEdit->clear();
    m_totalFiles = 0;

    m_filesModel.clear();
    m_currentArchivePath.clear();
    m_foundFiles.clear();

    ui->tabWidget->setCurrentIndex(0);
    setButtonsState(Ready);

    logMessage("Cleared");
}

void MainWindow::onSaveClicked()
{
    QStringList filesToSave = m_filesModel.getCheckedFiles();
    m_numberFilesToSave = filesToSave.size();

    if (filesToSave.isEmpty()) {
        logMessage("No files selected for saving");
        return;
    }

    QString savePath = QFileDialog::getSaveFileName(this, "Save results", "", "ZIP Archives (*.zip)");

    if (!savePath.isEmpty()) {
        logMessage(QString("Creating archive with %1 files...").arg(filesToSave.size()));
        setButtonsState(InProgress);

        ui->fileLabel->setText("Creating archive: " + savePath);
        ui->progressBar->setValue(0);

        m_workerManager.createArchive(m_currentArchivePath, filesToSave, savePath);
    }
}

void MainWindow::logMessage(const QString& msg)
{
    ui->logTextEdit->appendPlainText(QDateTime::currentDateTime().toString() + ": " + msg);
}

void MainWindow::setButtonsState(AppState state)
{
    ui->progressBar->setVisible(state == Done || state == InProgress);
    ui->fileLabel->setVisible(state == Done || state == InProgress);

    ui->selectFileButton->setEnabled(state == Ready);
    ui->abortButton->setEnabled(state == InProgress || state == Aborted);
    ui->clearButton->setEnabled(state != InProgress);
    ui->saveButton->setEnabled(state == Done && !m_foundFiles.isEmpty());
}
