#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>

#include "WorkerManager.h"
#include "FoundFilesModel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum AppState {
        Ready,
        InProgress,
        Done
    };

private slots:
    void onSelectFileClicked();
    void onCancelClicked();
    void onClearClicked();
    void onSaveClicked();

    void handleSearchResults(const QStringList& files);
    void handleArchiveCreated(bool success);

    void handleSearchStarted(int totalFiles);
    void handleFileProcessed(int current);

private:
    Ui::MainWindow *ui;
    FoundFilesModel m_filesModel;

    WorkerManager m_workerManager;
    QString m_currentArchivePath;
    QStringList m_foundFiles;
    int m_totalFiles;

    void setButtonsState(AppState state);
    void logMessage(const QString& msg);
};
#endif // MAINWINDOW_H
