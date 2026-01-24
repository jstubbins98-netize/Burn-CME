#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include "../common/core.h"
#include "progressdialog.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    void onAddFiles();
    void onAddFolder();
    void onRemoveSelected();
    void onClearQueue();
    void onCreateISO();
    void onDetectDrives();
    void onSelectDrive();
    void onBurnDisc();
    void onExtractISO();
    void onEraseDisc();
    void onRefreshInfo();
    void onEjectDrive();
    void updateQueueInfo();
    void updateDriveInfo();
    void onAddVideoFiles();
    void onCreateVideoDVD();
    void onExtractDVDVideo();

private:
    void setupUI();
    void setupDrivesTab();
    void setupQueueTab();
    void setupBurnTab();
    void setupExtractTab();
    void setupInfoTab();
    void setupVideoDVDTab();
    void appendLog(const QString& msg);

    BurnCMECore core;
    
    QTabWidget* tabWidget;
    
    QListWidget* driveList;
    QLabel* driveInfoLabel;
    QLabel* selectedDriveLabel;
    
    QListWidget* fileList;
    QLabel* queueInfoLabel;
    
    QComboBox* driveCombo;
    QLineEdit* isoPathEdit;
    QComboBox* speedCombo;
    
    QLineEdit* extractPathEdit;
    
    QTextEdit* infoText;
    QTextEdit* logText;
    
    QListWidget* videoFileList;
    QLineEdit* videoDVDOutputEdit;
    QLineEdit* videoExtractOutputEdit;
    
    std::vector<DriveInfo> detectedDrives;
};

#endif
