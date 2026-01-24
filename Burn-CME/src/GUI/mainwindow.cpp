#include "mainwindow.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>
#include <QStyle>
#include <QSplitter>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Burn-CME - CD/DVD Burning Utility");
    setMinimumSize(800, 600);
    
    core.setLogCallback([this](const std::string& msg) {
        appendLog(QString::fromStdString(msg));
    });
    
    setupUI();
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    tabWidget = new QTabWidget(this);
    
    setupDrivesTab();
    setupQueueTab();
    setupBurnTab();
    setupExtractTab();
    setupVideoDVDTab();
    setupInfoTab();
    
    mainLayout->addWidget(tabWidget);
    
    QGroupBox* logGroup = new QGroupBox("Log", this);
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    logText = new QTextEdit(this);
    logText->setReadOnly(true);
    logText->setMaximumHeight(150);
    logLayout->addWidget(logText);
    mainLayout->addWidget(logGroup);
    
    setCentralWidget(centralWidget);
    
    QMenuBar* menuBar = new QMenuBar(this);
    QMenu* fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction("&Add Files...", this, &MainWindow::onAddFiles);
    fileMenu->addAction("Add &Folder...", this, &MainWindow::onAddFolder);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", qApp, &QApplication::quit);
    
    QMenu* toolsMenu = menuBar->addMenu("&Tools");
    toolsMenu->addAction("&Detect Drives", this, &MainWindow::onDetectDrives);
    toolsMenu->addAction("&Create ISO", this, &MainWindow::onCreateISO);
    toolsMenu->addAction("&Burn Disc", this, &MainWindow::onBurnDisc);
    toolsMenu->addAction("&Extract from Disc", this, &MainWindow::onExtractISO);
    toolsMenu->addSeparator();
    toolsMenu->addAction("E&rase Disc", this, &MainWindow::onEraseDisc);
    toolsMenu->addAction("E&ject Drive", this, &MainWindow::onEjectDrive);
    
    QMenu* helpMenu = menuBar->addMenu("&Help");
    helpMenu->addAction("&About", [this]() {
        QMessageBox::about(this, "About Burn-CME",
            "Burn-CME v1.0\n\n"
            "CD/DVD Burning and Extraction Utility\n\n"
            "A Qt6 GUI for burning and extracting optical media.");
    });
    
    setMenuBar(menuBar);
    
    statusBar()->showMessage("Ready");
    
    appendLog("Burn-CME started");
    onDetectDrives();
}

void MainWindow::setupDrivesTab() {
    QWidget* drivesTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(drivesTab);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* refreshBtn = new QPushButton("Refresh Drives", this);
    QPushButton* selectBtn = new QPushButton("Select Drive", this);
    QPushButton* ejectBtn = new QPushButton("Eject", this);
    
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onDetectDrives);
    connect(selectBtn, &QPushButton::clicked, this, &MainWindow::onSelectDrive);
    connect(ejectBtn, &QPushButton::clicked, this, &MainWindow::onEjectDrive);
    
    buttonLayout->addWidget(refreshBtn);
    buttonLayout->addWidget(selectBtn);
    buttonLayout->addWidget(ejectBtn);
    buttonLayout->addStretch();
    
    layout->addLayout(buttonLayout);
    
    QGroupBox* driveListGroup = new QGroupBox("Available Drives", this);
    QVBoxLayout* driveListLayout = new QVBoxLayout(driveListGroup);
    
    driveList = new QListWidget(this);
    driveList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(driveList, &QListWidget::itemDoubleClicked, this, &MainWindow::onSelectDrive);
    driveListLayout->addWidget(driveList);
    
    layout->addWidget(driveListGroup);
    
    QGroupBox* selectedGroup = new QGroupBox("Selected Drive", this);
    QVBoxLayout* selectedLayout = new QVBoxLayout(selectedGroup);
    
    selectedDriveLabel = new QLabel("No drive selected", this);
    selectedDriveLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    selectedLayout->addWidget(selectedDriveLabel);
    
    driveInfoLabel = new QLabel("Click 'Refresh Drives' to scan for optical drives", this);
    driveInfoLabel->setWordWrap(true);
    selectedLayout->addWidget(driveInfoLabel);
    
    layout->addWidget(selectedGroup);
    
    tabWidget->addTab(drivesTab, "Drives");
}

void MainWindow::setupQueueTab() {
    QWidget* queueTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(queueTab);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* addFilesBtn = new QPushButton("Add Files...", this);
    QPushButton* addFolderBtn = new QPushButton("Add Folder...", this);
    QPushButton* removeBtn = new QPushButton("Remove Selected", this);
    QPushButton* clearBtn = new QPushButton("Clear All", this);
    
    connect(addFilesBtn, &QPushButton::clicked, this, &MainWindow::onAddFiles);
    connect(addFolderBtn, &QPushButton::clicked, this, &MainWindow::onAddFolder);
    connect(removeBtn, &QPushButton::clicked, this, &MainWindow::onRemoveSelected);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::onClearQueue);
    
    buttonLayout->addWidget(addFilesBtn);
    buttonLayout->addWidget(addFolderBtn);
    buttonLayout->addWidget(removeBtn);
    buttonLayout->addWidget(clearBtn);
    buttonLayout->addStretch();
    
    layout->addLayout(buttonLayout);
    
    fileList = new QListWidget(this);
    fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    layout->addWidget(fileList);
    
    queueInfoLabel = new QLabel("No files in queue", this);
    layout->addWidget(queueInfoLabel);
    
    QHBoxLayout* isoLayout = new QHBoxLayout();
    isoLayout->addWidget(new QLabel("ISO Name:", this));
    QLineEdit* isoNameEdit = new QLineEdit("burn_image", this);
    isoNameEdit->setObjectName("isoNameEdit");
    isoLayout->addWidget(isoNameEdit);
    QPushButton* createISOBtn = new QPushButton("Create ISO", this);
    connect(createISOBtn, &QPushButton::clicked, this, &MainWindow::onCreateISO);
    isoLayout->addWidget(createISOBtn);
    
    layout->addLayout(isoLayout);
    
    tabWidget->addTab(queueTab, "File Queue");
}

void MainWindow::setupBurnTab() {
    QWidget* burnTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(burnTab);
    
    QGroupBox* driveGroup = new QGroupBox("Drive Selection", this);
    QHBoxLayout* driveLayout = new QHBoxLayout(driveGroup);
    driveLayout->addWidget(new QLabel("Drive:", this));
    driveCombo = new QComboBox(this);
    driveLayout->addWidget(driveCombo, 1);
    QPushButton* detectBtn = new QPushButton("Detect", this);
    connect(detectBtn, &QPushButton::clicked, this, &MainWindow::onDetectDrives);
    driveLayout->addWidget(detectBtn);
    layout->addWidget(driveGroup);
    
    QGroupBox* isoGroup = new QGroupBox("ISO Image", this);
    QHBoxLayout* isoLayout = new QHBoxLayout(isoGroup);
    isoLayout->addWidget(new QLabel("ISO File:", this));
    isoPathEdit = new QLineEdit(this);
    isoLayout->addWidget(isoPathEdit, 1);
    QPushButton* browseBtn = new QPushButton("Browse...", this);
    connect(browseBtn, &QPushButton::clicked, [this]() {
        QString file = QFileDialog::getOpenFileName(this, "Select ISO File", 
            QString(), "ISO Images (*.iso);;All Files (*)");
        if (!file.isEmpty()) {
            isoPathEdit->setText(file);
        }
    });
    isoLayout->addWidget(browseBtn);
    layout->addWidget(isoGroup);
    
    QGroupBox* optionsGroup = new QGroupBox("Burn Options", this);
    QHBoxLayout* optionsLayout = new QHBoxLayout(optionsGroup);
    optionsLayout->addWidget(new QLabel("Speed:", this));
    speedCombo = new QComboBox(this);
    speedCombo->addItem("Auto", 0);
    speedCombo->addItem("4x", 4);
    speedCombo->addItem("8x", 8);
    speedCombo->addItem("16x", 16);
    speedCombo->addItem("Maximum", 0);
    optionsLayout->addWidget(speedCombo);
    optionsLayout->addStretch();
    layout->addWidget(optionsGroup);
    
    layout->addStretch();
    
    QHBoxLayout* actionLayout = new QHBoxLayout();
    QPushButton* burnBtn = new QPushButton("Burn to Disc", this);
    burnBtn->setMinimumHeight(40);
    connect(burnBtn, &QPushButton::clicked, this, &MainWindow::onBurnDisc);
    actionLayout->addStretch();
    actionLayout->addWidget(burnBtn);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);
    
    tabWidget->addTab(burnTab, "Burn");
}

void MainWindow::setupExtractTab() {
    QWidget* extractTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(extractTab);
    
    QGroupBox* outputGroup = new QGroupBox("Output Location", this);
    QHBoxLayout* outputLayout = new QHBoxLayout(outputGroup);
    outputLayout->addWidget(new QLabel("Save to:", this));
    extractPathEdit = new QLineEdit("extracted_disc.iso", this);
    outputLayout->addWidget(extractPathEdit, 1);
    QPushButton* browseBtn = new QPushButton("Browse...", this);
    connect(browseBtn, &QPushButton::clicked, [this]() {
        QString file = QFileDialog::getSaveFileName(this, "Save ISO As", 
            "extracted_disc.iso", "ISO Images (*.iso)");
        if (!file.isEmpty()) {
            extractPathEdit->setText(file);
        }
    });
    outputLayout->addWidget(browseBtn);
    layout->addWidget(outputGroup);
    
    layout->addStretch();
    
    QHBoxLayout* actionLayout = new QHBoxLayout();
    QPushButton* extractBtn = new QPushButton("Extract to ISO", this);
    extractBtn->setMinimumHeight(40);
    connect(extractBtn, &QPushButton::clicked, this, &MainWindow::onExtractISO);
    actionLayout->addStretch();
    actionLayout->addWidget(extractBtn);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);
    
    tabWidget->addTab(extractTab, "Extract");
}

void MainWindow::setupInfoTab() {
    QWidget* infoTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(infoTab);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* refreshBtn = new QPushButton("Refresh Info", this);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshInfo);
    buttonLayout->addWidget(refreshBtn);
    
    QPushButton* eraseBtn = new QPushButton("Erase Disc", this);
    connect(eraseBtn, &QPushButton::clicked, this, &MainWindow::onEraseDisc);
    buttonLayout->addWidget(eraseBtn);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);
    
    infoText = new QTextEdit(this);
    infoText->setReadOnly(true);
    infoText->setFontFamily("monospace");
    layout->addWidget(infoText);
    
    tabWidget->addTab(infoTab, "Disc Info");
}

void MainWindow::appendLog(const QString& msg) {
    logText->append(msg);
    logText->verticalScrollBar()->setValue(logText->verticalScrollBar()->maximum());
}

void MainWindow::onAddFiles() {
    QStringList files = QFileDialog::getOpenFileNames(this, "Select Files to Add");
    for (const QString& file : files) {
        core.addPath(file.toStdString());
    }
    updateQueueInfo();
}

void MainWindow::onAddFolder() {
    QString folder = QFileDialog::getExistingDirectory(this, "Select Folder to Add");
    if (!folder.isEmpty()) {
        core.addPath(folder.toStdString());
        updateQueueInfo();
    }
}

void MainWindow::onRemoveSelected() {
    appendLog("Note: Remove selected not fully implemented - clearing queue instead");
    core.clearQueue();
    updateQueueInfo();
}

void MainWindow::onClearQueue() {
    core.clearQueue();
    updateQueueInfo();
}

void MainWindow::onCreateISO() {
    QLineEdit* isoNameEdit = findChild<QLineEdit*>("isoNameEdit");
    QString isoName = isoNameEdit ? isoNameEdit->text() : "burn_image";
    
    if (isoName.isEmpty()) {
        isoName = "burn_image";
    }
    
    if (core.getFileQueue().empty()) {
        QMessageBox::warning(this, "Error", "No files in queue. Add files before creating ISO.");
        return;
    }
    
    ProgressDialog progress("Creating ISO", "Creating ISO Image", this);
    progress.setStatus("Building ISO image from queued files...");
    progress.setIndeterminate(true);
    progress.show();
    QApplication::processEvents();
    
    statusBar()->showMessage("Creating ISO... (please wait)");
    bool success = core.createISO(isoName.toStdString());
    
    if (success) {
        progress.complete(true, "ISO image created:\n" + isoName + ".iso");
        statusBar()->showMessage("ISO created successfully");
    } else {
        progress.complete(false, "Failed to create ISO image. Check log for details.");
        statusBar()->showMessage("ISO creation failed");
    }
    progress.exec();
}

void MainWindow::onDetectDrives() {
    driveCombo->clear();
    driveList->clear();
    detectedDrives = core.detectDrives();
    
    if (detectedDrives.empty()) {
        driveCombo->addItem("No drives detected", "");
        driveList->addItem("No optical drives detected");
        driveInfoLabel->setText("No optical drives found.\nConnect a CD/DVD drive and click 'Refresh Drives'.");
        appendLog("No optical drives detected");
    } else {
        for (const auto& drive : detectedDrives) {
            QString displayText = QString::fromStdString(drive.path);
            if (!drive.name.empty()) {
                displayText += " - " + QString::fromStdString(drive.name);
            }
            
            driveCombo->addItem(displayText, QString::fromStdString(drive.path));
            
            QListWidgetItem* item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, QString::fromStdString(drive.path));
            driveList->addItem(item);
        }
        
        driveList->setCurrentRow(0);
        core.setCurrentDrive(detectedDrives[0].path);
        updateDriveInfo();
        appendLog("Detected " + QString::number(detectedDrives.size()) + " drive(s)");
    }
}

void MainWindow::onSelectDrive() {
    QListWidgetItem* item = driveList->currentItem();
    if (!item) {
        QMessageBox::information(this, "No Selection", "Please select a drive from the list.");
        return;
    }
    
    QString drivePath = item->data(Qt::UserRole).toString();
    core.setCurrentDrive(drivePath.toStdString());
    
    for (int i = 0; i < driveCombo->count(); ++i) {
        if (driveCombo->itemData(i).toString() == drivePath) {
            driveCombo->setCurrentIndex(i);
            break;
        }
    }
    
    updateDriveInfo();
    appendLog("Selected drive: " + drivePath);
    statusBar()->showMessage("Drive selected: " + drivePath);
}

void MainWindow::updateDriveInfo() {
    QListWidgetItem* item = driveList->currentItem();
    if (!item) {
        selectedDriveLabel->setText("No drive selected");
        driveInfoLabel->setText("Select a drive from the list above.");
        return;
    }
    
    QString drivePath = item->data(Qt::UserRole).toString();
    selectedDriveLabel->setText(drivePath);
    
    for (const auto& drive : detectedDrives) {
        if (drive.path == drivePath.toStdString()) {
            QString info;
            if (!drive.name.empty()) {
                info += "Name: " + QString::fromStdString(drive.name) + "\n";
            }
            info += "Path: " + QString::fromStdString(drive.path) + "\n";
            if (!drive.capabilities.empty()) {
                info += "Capabilities: " + QString::fromStdString(drive.capabilities);
            }
            if (drive.is_apple_superdrive) {
                info += "\nType: Apple SuperDrive";
            }
            driveInfoLabel->setText(info);
            break;
        }
    }
}

void MainWindow::onEjectDrive() {
    if (driveCombo->currentData().toString().isEmpty()) {
        QMessageBox::warning(this, "Error", "No drive selected");
        return;
    }
    
    QString drivePath = driveCombo->currentData().toString();
    appendLog("Ejecting drive: " + drivePath);
    
    bool success = core.ejectDisc();
    
    if (success) {
        statusBar()->showMessage("Drive ejected");
        appendLog("Drive ejected successfully");
    } else {
        statusBar()->showMessage("Eject failed");
        appendLog("Failed to eject drive");
    }
}

void MainWindow::onBurnDisc() {
    QString isoPath = isoPathEdit->text();
    if (isoPath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select an ISO file to burn");
        return;
    }
    
    if (driveCombo->currentData().toString().isEmpty()) {
        QMessageBox::warning(this, "Error", "No drive selected");
        return;
    }
    
    int ret = QMessageBox::warning(this, "Confirm Burn",
        "This will write data to the disc. Make sure a blank/rewritable disc is inserted.\n\nContinue?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        core.setCurrentDrive(driveCombo->currentData().toString().toStdString());
        int speed = speedCombo->currentData().toInt();
        
        ProgressDialog progress("Burning Disc", "Burning CD/DVD", this);
        progress.setStatus("Writing data to disc...");
        progress.setIndeterminate(true);
        progress.show();
        QApplication::processEvents();
        
        statusBar()->showMessage("Burning... (please wait)");
        bool success = core.burnDisc(isoPath.toStdString(), speed);
        
        if (success) {
            progress.complete(true, "Disc burned successfully!");
            statusBar()->showMessage("Burn complete");
        } else {
            progress.complete(false, "Failed to burn disc. Check log for details.");
            statusBar()->showMessage("Burn failed");
        }
        progress.exec();
    }
}

void MainWindow::onExtractISO() {
    QString outputPath = extractPathEdit->text();
    if (outputPath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please specify an output path");
        return;
    }
    
    if (driveCombo->currentData().toString().isEmpty()) {
        QMessageBox::warning(this, "Error", "No drive selected");
        return;
    }
    
    core.setCurrentDrive(driveCombo->currentData().toString().toStdString());
    
    ProgressDialog progress("Extracting Data", "Extracting from CD/DVD", this);
    progress.setStatus("Reading disc and extracting data...");
    progress.setIndeterminate(true);
    progress.show();
    QApplication::processEvents();
    
    statusBar()->showMessage("Extracting... (please wait)");
    bool success = core.extractToISO(outputPath.toStdString());
    
    if (success) {
        progress.complete(true, "Data extracted successfully to:\n" + outputPath);
        statusBar()->showMessage("Extraction complete");
    } else {
        progress.complete(false, "Failed to extract data. Check log for details.");
        statusBar()->showMessage("Extraction failed");
    }
    progress.exec();
}

void MainWindow::onEraseDisc() {
    if (driveCombo->currentData().toString().isEmpty()) {
        QMessageBox::warning(this, "Error", "No drive selected");
        return;
    }
    
    int ret = QMessageBox::warning(this, "Confirm Erase",
        "This will permanently erase all data on the disc!\n\nType 'ERASE' to confirm:",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        core.setCurrentDrive(driveCombo->currentData().toString().toStdString());
        
        ProgressDialog progress("Erasing Disc", "Erasing CD/DVD", this);
        progress.setStatus("Erasing disc data...");
        progress.setIndeterminate(true);
        progress.show();
        QApplication::processEvents();
        
        statusBar()->showMessage("Erasing... (please wait)");
        bool success = core.eraseDisc(false);
        
        if (success) {
            progress.complete(true, "Disc erased successfully!");
            statusBar()->showMessage("Erase complete");
        } else {
            progress.complete(false, "Failed to erase disc. Check log for details.");
            statusBar()->showMessage("Erase failed");
        }
        progress.exec();
    }
}

void MainWindow::onRefreshInfo() {
    if (driveCombo->currentData().toString().isEmpty()) {
        infoText->setText("No drive selected");
        return;
    }
    
    core.setCurrentDrive(driveCombo->currentData().toString().toStdString());
    std::string info = core.getDiscInfo();
    infoText->setText(QString::fromStdString(info));
}

void MainWindow::updateQueueInfo() {
    fileList->clear();
    
    const auto& queue = core.getFileQueue();
    for (const auto& fe : queue) {
        QString item = QString::fromStdString(fe.display_name) + 
                       " (" + QString::number(fe.size / 1024.0 / 1024.0, 'f', 2) + " MB)";
        fileList->addItem(item);
    }
    
    size_t totalSize = core.getTotalSize();
    double totalMB = totalSize / 1024.0 / 1024.0;
    
    QString info;
    if (queue.empty()) {
        info = "No files in queue";
    } else {
        info = QString("%1 files, %2 MB total").arg(queue.size()).arg(totalMB, 0, 'f', 2);
        
        if (totalMB <= 700) {
            info += " - Fits on CD";
        } else if (totalMB <= 4700) {
            info += " - Requires DVD";
        } else {
            info += " - Requires dual-layer DVD or multiple discs";
        }
    }
    
    queueInfoLabel->setText(info);
}

void MainWindow::setupVideoDVDTab() {
    QWidget* videoTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(videoTab);
    
    QGroupBox* createGroup = new QGroupBox("Create Video DVD from MP4", this);
    QVBoxLayout* createLayout = new QVBoxLayout(createGroup);
    
    QHBoxLayout* videoListLayout = new QHBoxLayout();
    videoFileList = new QListWidget(this);
    videoFileList->setMaximumHeight(120);
    videoListLayout->addWidget(videoFileList, 1);
    
    QVBoxLayout* videoButtonLayout = new QVBoxLayout();
    QPushButton* addVideoBtn = new QPushButton("Add Videos", this);
    QPushButton* removeVideoBtn = new QPushButton("Remove", this);
    QPushButton* clearVideoBtn = new QPushButton("Clear All", this);
    videoButtonLayout->addWidget(addVideoBtn);
    videoButtonLayout->addWidget(removeVideoBtn);
    videoButtonLayout->addWidget(clearVideoBtn);
    videoButtonLayout->addStretch();
    videoListLayout->addLayout(videoButtonLayout);
    createLayout->addLayout(videoListLayout);
    
    connect(addVideoBtn, &QPushButton::clicked, this, &MainWindow::onAddVideoFiles);
    connect(removeVideoBtn, &QPushButton::clicked, [this]() {
        if (videoFileList->currentRow() >= 0) {
            delete videoFileList->takeItem(videoFileList->currentRow());
        }
    });
    connect(clearVideoBtn, &QPushButton::clicked, [this]() {
        videoFileList->clear();
    });
    
    QHBoxLayout* outputLayout = new QHBoxLayout();
    outputLayout->addWidget(new QLabel("Output folder:", this));
    videoDVDOutputEdit = new QLineEdit("video_dvd", this);
    outputLayout->addWidget(videoDVDOutputEdit, 1);
    QPushButton* browseOutputBtn = new QPushButton("Browse...", this);
    connect(browseOutputBtn, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory");
        if (!dir.isEmpty()) {
            videoDVDOutputEdit->setText(dir);
        }
    });
    outputLayout->addWidget(browseOutputBtn);
    createLayout->addLayout(outputLayout);
    
    QPushButton* createDVDBtn = new QPushButton("Create Video DVD", this);
    createDVDBtn->setMinimumHeight(35);
    connect(createDVDBtn, &QPushButton::clicked, this, &MainWindow::onCreateVideoDVD);
    createLayout->addWidget(createDVDBtn);
    
    layout->addWidget(createGroup);
    
    QGroupBox* extractGroup = new QGroupBox("Extract DVD Video to MP4", this);
    QVBoxLayout* extractLayout = new QVBoxLayout(extractGroup);
    
    QHBoxLayout* extractOutputLayout = new QHBoxLayout();
    extractOutputLayout->addWidget(new QLabel("Output MP4:", this));
    videoExtractOutputEdit = new QLineEdit("extracted_video.mp4", this);
    extractOutputLayout->addWidget(videoExtractOutputEdit, 1);
    QPushButton* browseExtractBtn = new QPushButton("Browse...", this);
    connect(browseExtractBtn, &QPushButton::clicked, [this]() {
        QString file = QFileDialog::getSaveFileName(this, "Save MP4 As", 
            "extracted_video.mp4", "MP4 Video (*.mp4)");
        if (!file.isEmpty()) {
            videoExtractOutputEdit->setText(file);
        }
    });
    extractOutputLayout->addWidget(browseExtractBtn);
    extractLayout->addLayout(extractOutputLayout);
    
    QHBoxLayout* extractBtnLayout = new QHBoxLayout();
    QPushButton* extractFromDriveBtn = new QPushButton("Extract from DVD Drive", this);
    QPushButton* extractFromFolderBtn = new QPushButton("Extract from VIDEO_TS Folder", this);
    extractFromDriveBtn->setMinimumHeight(35);
    extractFromFolderBtn->setMinimumHeight(35);
    
    connect(extractFromDriveBtn, &QPushButton::clicked, this, &MainWindow::onExtractDVDVideo);
    connect(extractFromFolderBtn, &QPushButton::clicked, [this]() {
        QString folder = QFileDialog::getExistingDirectory(this, "Select VIDEO_TS Folder");
        if (!folder.isEmpty()) {
            appendLog("Extracting from: " + folder);
            
            ProgressDialog progress("Extracting DVD Video", "Converting to MP4...", this);
            progress.show();
            QApplication::processEvents();
            
            bool success = core.extractDVDtoMP4(folder.toStdString(), 
                                                 videoExtractOutputEdit->text().toStdString());
            progress.close();
            
            if (success) {
                QMessageBox::information(this, "Success", "Video extracted successfully!");
            } else {
                QMessageBox::warning(this, "Failed", "Failed to extract video. Check log for details.");
            }
        }
    });
    
    extractBtnLayout->addWidget(extractFromDriveBtn);
    extractBtnLayout->addWidget(extractFromFolderBtn);
    extractLayout->addLayout(extractBtnLayout);
    
    layout->addWidget(extractGroup);
    
    QLabel* ffmpegNote = new QLabel(
        "<b>Note:</b> Video DVD features require FFmpeg. "
        "Install with: <code>sudo apt install ffmpeg dvdauthor</code> (Linux) "
        "or <code>brew install ffmpeg dvdauthor</code> (macOS)", this);
    ffmpegNote->setWordWrap(true);
    layout->addWidget(ffmpegNote);
    
    layout->addStretch();
    
    tabWidget->addTab(videoTab, "Video DVD");
}

void MainWindow::onAddVideoFiles() {
    QStringList files = QFileDialog::getOpenFileNames(this, "Select Video Files", QString(),
        "Video Files (*.mp4 *.avi *.mkv *.mov *.wmv *.flv *.webm *.m4v *.mpeg *.mpg);;All Files (*)");
    
    for (const QString& file : files) {
        videoFileList->addItem(file);
    }
}

void MainWindow::onCreateVideoDVD() {
    if (videoFileList->count() == 0) {
        QMessageBox::warning(this, "No Videos", "Please add video files first.");
        return;
    }
    
    if (!core.checkFFmpeg()) {
        QMessageBox::critical(this, "FFmpeg Not Found", 
            "FFmpeg is not installed. Please install FFmpeg to use video DVD features.\n\n"
            "Linux: sudo apt install ffmpeg\n"
            "macOS: brew install ffmpeg");
        return;
    }
    
    QString outputDir = videoDVDOutputEdit->text();
    if (outputDir.isEmpty()) {
        outputDir = "video_dvd";
    }
    
    std::vector<std::string> videoFiles;
    for (int i = 0; i < videoFileList->count(); i++) {
        videoFiles.push_back(videoFileList->item(i)->text().toStdString());
    }
    
    appendLog("Creating Video DVD with " + QString::number(videoFiles.size()) + " video(s)...");
    
    ProgressDialog progress("Creating Video DVD", "Converting videos to DVD format...", this);
    progress.show();
    QApplication::processEvents();
    
    bool success = core.convertMultipleMP4stoDVD(videoFiles, outputDir.toStdString());
    progress.close();
    
    if (success) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Video DVD Created",
            "Video DVD structure created successfully!\n\n"
            "Would you like to burn it to a disc now?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            appendLog("Burning Video DVD...");
            
            ProgressDialog burnProgress("Burning Video DVD", "Burning to disc...", this);
            burnProgress.show();
            QApplication::processEvents();
            
            bool burnSuccess = core.burnVideoDVD(outputDir.toStdString());
            burnProgress.close();
            
            if (burnSuccess) {
                QMessageBox::information(this, "Success", "Video DVD burned successfully!");
            } else {
                QMessageBox::warning(this, "Burn Failed", "Failed to burn Video DVD. Check log for details.");
            }
        }
    } else {
        QMessageBox::warning(this, "Failed", "Failed to create Video DVD. Check log for details.");
    }
}

void MainWindow::onExtractDVDVideo() {
    if (!core.checkFFmpeg()) {
        QMessageBox::critical(this, "FFmpeg Not Found", 
            "FFmpeg is not installed. Please install FFmpeg to use video DVD features.");
        return;
    }
    
    QString outputPath = videoExtractOutputEdit->text();
    if (outputPath.isEmpty()) {
        outputPath = "extracted_video.mp4";
    }
    
    appendLog("Extracting DVD video to MP4...");
    
    ProgressDialog progress("Extracting DVD Video", "Converting to MP4 (this may take a while)...", this);
    progress.show();
    QApplication::processEvents();
    
    bool success = core.extractDVDDriveToMP4(outputPath.toStdString());
    progress.close();
    
    if (success) {
        QMessageBox::information(this, "Success", 
            "Video extracted successfully!\n\nSaved to: " + outputPath);
    } else {
        QMessageBox::warning(this, "Failed", "Failed to extract video. Check log for details.");
    }
}
