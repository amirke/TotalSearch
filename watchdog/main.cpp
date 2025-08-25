#include <QApplication>
#include <QDir>
#include "TotalSearchWatchdog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("TotalSearch Watchdog");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("TotalSearch");
    
    // Create and show the watchdog window
    TotalSearchWatchdog watchdog;
    watchdog.show();
    
    return app.exec();
}
