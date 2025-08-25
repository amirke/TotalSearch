#include <QApplication>
#include <QStyleFactory>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QString>
#include <QByteArray>
#include <QFileInfo>
#include <exception>
#include "mainwindow.h"
#include "logger.h"

// Global exception handler
void globalExceptionHandler() {
    LOG_ERROR("Main: Unhandled exception caught by global handler");
    // Force exit to prevent further damage
    exit(1);
}

int main(int argc, char* argv[]) {
    // Set global exception handler
    std::set_terminate(globalExceptionHandler);
    qDebug() << "main: Starting application";
    
    // Set Qt library paths before creating QApplication
    QString appDir = QCoreApplication::applicationDirPath();
    QString qtLibPath = appDir;  // Qt DLLs are in the same directory
    QString qtPluginPath = appDir + "/plugins";
    
    // Set environment variables for Qt to find libraries
    qputenv("QT_PLUGIN_PATH", qtPluginPath.toLocal8Bit());
    qputenv("QT_QPA_PLATFORM_PLUGIN_PATH", (qtPluginPath + "/platforms").toLocal8Bit());
    
    // Set PATH environment variable to include Qt library directory
    QString currentPath = qgetenv("PATH");
    QString newPath = qtLibPath + ";" + currentPath;
    qputenv("PATH", newPath.toLocal8Bit());
    
    qDebug() << "main: Qt library path set to:" << qtLibPath;
    qDebug() << "main: Qt plugin path set to:" << qtPluginPath;
    qDebug() << "main: PATH updated to include Qt libraries";
    
    QApplication app(argc, argv);
    qDebug() << "main: QApplication created";
    
    // Set application properties
    app.setApplicationName("TotalSearch");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("TotalSearch");
    qDebug() << "main: Application properties set";
    
    // Set application icon
    app.setWindowIcon(QIcon(":/icons/app_icon.ico"));
    qDebug() << "main: Application icon set";
    
    // Set modern style
    app.setStyle(QStyleFactory::create("Fusion"));
    qDebug() << "main: Style set to Fusion";
    
    // Create and show main window
    qDebug() << "main: About to create MainWindow";
    MainWindow window;
    qDebug() << "main: MainWindow created successfully";
    
    qDebug() << "main: About to show window";
    window.show();
    qDebug() << "main: Window shown successfully";
    
    // If a file was passed as argument, open it
    if (argc > 1) {
        QString filePath = QString::fromLocal8Bit(argv[1]);
        qDebug() << "main: File argument provided:" << filePath;
        // TODO: Open file directly
    }
    
    qDebug() << "main: About to start event loop";
    return app.exec();
} 