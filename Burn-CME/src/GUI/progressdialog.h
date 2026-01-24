#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QMovie>
#include <QPushButton>
#include <QApplication>
#include <QDir>
#include <QFileInfo>

class ProgressDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProgressDialog(const QString& title, const QString& operation, QWidget* parent = nullptr)
        : QDialog(parent) {
        setWindowTitle(title);
        setFixedSize(400, 350);
        setModal(true);
        setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
        
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignCenter);
        layout->setSpacing(20);
        
        operationLabel = new QLabel(operation, this);
        operationLabel->setAlignment(Qt::AlignCenter);
        operationLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
        layout->addWidget(operationLabel);
        
        gifLabel = new QLabel(this);
        gifLabel->setAlignment(Qt::AlignCenter);
        gifLabel->setFixedSize(150, 150);
        
        QString gifPath = findGifPath();
        if (!gifPath.isEmpty()) {
            movie = new QMovie(gifPath, QByteArray(), this);
            movie->setScaledSize(QSize(150, 150));
            gifLabel->setMovie(movie);
            movie->start();
        } else {
            gifLabel->setText("[CD Animation]");
            gifLabel->setStyleSheet("border: 2px solid gray; border-radius: 75px; background: #e0e0e0;");
        }
        layout->addWidget(gifLabel, 0, Qt::AlignCenter);
        
        statusLabel = new QLabel("Initializing...", this);
        statusLabel->setAlignment(Qt::AlignCenter);
        statusLabel->setWordWrap(true);
        layout->addWidget(statusLabel);
        
        progressBar = new QProgressBar(this);
        progressBar->setMinimum(0);
        progressBar->setMaximum(100);
        progressBar->setValue(0);
        progressBar->setTextVisible(true);
        progressBar->setFixedWidth(350);
        layout->addWidget(progressBar, 0, Qt::AlignCenter);
        
        cancelButton = new QPushButton("Cancel", this);
        cancelButton->setEnabled(false);
        connect(cancelButton, &QPushButton::clicked, this, &ProgressDialog::onCancel);
        layout->addWidget(cancelButton, 0, Qt::AlignCenter);
        
        cancelled = false;
    }
    
    ~ProgressDialog() {
        if (movie) {
            movie->stop();
        }
    }
    
    void setProgress(int value) {
        progressBar->setValue(value);
        QApplication::processEvents();
    }
    
    void setStatus(const QString& status) {
        statusLabel->setText(status);
        QApplication::processEvents();
    }
    
    void setIndeterminate(bool indeterminate) {
        if (indeterminate) {
            progressBar->setMaximum(0);
        } else {
            progressBar->setMaximum(100);
        }
        QApplication::processEvents();
    }
    
    void enableCancel(bool enable) {
        cancelButton->setEnabled(enable);
    }
    
    bool wasCancelled() const {
        return cancelled;
    }
    
    void complete(bool success, const QString& message) {
        if (movie) {
            movie->stop();
        }
        progressBar->setValue(success ? 100 : 0);
        statusLabel->setText(message);
        cancelButton->setText("Close");
        cancelButton->setEnabled(true);
        disconnect(cancelButton, &QPushButton::clicked, this, &ProgressDialog::onCancel);
        connect(cancelButton, &QPushButton::clicked, this, &QDialog::accept);
    }

private slots:
    void onCancel() {
        cancelled = true;
        statusLabel->setText("Cancelling...");
        cancelButton->setEnabled(false);
    }

private:
    QString findGifPath() {
        QStringList searchPaths = {
            QApplication::applicationDirPath() + "/../images/cd-spinning.gif",
            QApplication::applicationDirPath() + "/../../images/cd-spinning.gif",
            QDir::currentPath() + "/images/cd-spinning.gif",
            QDir::currentPath() + "/Burn-CME/images/cd-spinning.gif"
        };
        
        for (const QString& path : searchPaths) {
            if (QFileInfo::exists(path)) {
                return path;
            }
        }
        return QString();
    }
    
    QLabel* operationLabel;
    QLabel* gifLabel;
    QLabel* statusLabel;
    QProgressBar* progressBar;
    QPushButton* cancelButton;
    QMovie* movie = nullptr;
    bool cancelled;
};

#endif
