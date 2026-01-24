#include <QApplication>
#include <QIcon>
#include <QDir>
#include <QFileInfo>
#include "mainwindow.h"

QString findIconPath() {
    QStringList searchPaths = {
        QApplication::applicationDirPath() + "/../images/burn-cme-icon.png",
        QApplication::applicationDirPath() + "/../../images/burn-cme-icon.png",
        QDir::currentPath() + "/images/burn-cme-icon.png",
        "/usr/share/icons/burn-cme-icon.png",
        "/usr/local/share/icons/burn-cme-icon.png",
        "/usr/share/pixmaps/burn-cme.png"
    };
    
    for (const QString& path : searchPaths) {
        if (QFileInfo::exists(path)) {
            return path;
        }
    }
    return QString();
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setApplicationName("Burn-CME");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Burn-CME");
    
    QString iconPath = findIconPath();
    if (!iconPath.isEmpty()) {
        app.setWindowIcon(QIcon(iconPath));
    }
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
