#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>

#include "WorkerManager.h"

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

private slots:
    void onSelectFileClicked();
    void onCancelClicked();
    void onClearClicked();
    void onSaveClicked();

    void handleSearchResults(const QStringList& files);
    void handleArchiveCreated(bool success);
    void handleProgressUpdate(int progress);

private:
    Ui::MainWindow *ui;
    WorkerManager m_workerManager;
    QString m_currentArchivePath;
    QStringList m_foundFiles;

    void setButtonsState(bool isWorking);
    void logMessage(const QString& msg);
};
#endif // MAINWINDOW_H
