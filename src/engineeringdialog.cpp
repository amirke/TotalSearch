#include "engineeringdialog.h"
#include "mainwindow.h"
#include <QGroupBox>
#include <QGridLayout>
#include <QMessageBox>
#include <QPainter>
#include "logger.h"

EngineeringDialog::EngineeringDialog(MainWindow *mainWindow, QWidget *parent)
    : QDialog(parent), m_mainWindow(mainWindow)
{
    setWindowTitle("Engineering Tools");
    setModal(true);
    resize(300, 150);
    
    setupUI();
    
    // Connect signals
    connect(tryButton, &QPushButton::clicked, this, &EngineeringDialog::onTryButtonClicked);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    LOG_INFO("EngineeringDialog: Created engineering tools dialog");
}

void EngineeringDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Title label
    QLabel *titleLabel = new QLabel("Engineering Tools Access");
    titleLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; color: #2c3e50; padding: 10px; }");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Description
    QLabel *descLabel = new QLabel("Access advanced engineering and testing tools");
    descLabel->setStyleSheet("QLabel { font-size: 11px; color: #6c757d; padding: 5px; }");
    descLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(descLabel);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    tryButton = new QPushButton("Try");
    tryButton->setToolTip("Open engineering tools window");
    tryButton->setStyleSheet("QPushButton { font-weight: bold; padding: 8px 16px; }");
    
    closeButton = new QPushButton("Apply");
    closeButton->setToolTip("Apply and close this dialog");
    // Create check mark icon using Unicode character
    QPixmap checkPixmap(16, 16);
    checkPixmap.fill(Qt::transparent);
    QPainter painter(&checkPixmap);
    painter.setPen(QPen(Qt::white, 2));
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(checkPixmap.rect(), Qt::AlignCenter, "✓");
    closeButton->setIcon(QIcon(checkPixmap));
    closeButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #28a745, stop:1 #20c997);"
        "    border: none;"
        "    color: white;"
        "    padding: 8px 16px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    border-radius: 6px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #20c997, stop:1 #17a2b8);"
        "    transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #17a2b8, stop:1 #138496);"
        "    transform: translateY(1px);"
        "}"
    );
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(tryButton);
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
}

void EngineeringDialog::onTryButtonClicked()
{
    LOG_INFO("EngineeringDialog: Try button clicked, opening tools dialog");
    
    // Create and show the engineering tools dialog
    EngineeringToolsDialog *toolsDialog = new EngineeringToolsDialog(m_mainWindow, this);
    toolsDialog->exec();
    toolsDialog->deleteLater();
}



// EngineeringToolsDialog implementation
EngineeringToolsDialog::EngineeringToolsDialog(MainWindow *mainWindow, QWidget *parent)
    : QDialog(parent), m_mainWindow(mainWindow)
{
    setWindowTitle("Engineering Tools");
    setModal(true);
    resize(500, 400);
    
    setupUI();
    
    // Connect all the tool buttons to their respective MainWindow methods
    connect(openFile2Button, &QPushButton::clicked, this, &EngineeringToolsDialog::onOpenFile2Clicked);
    connect(findInFileButton, &QPushButton::clicked, this, &EngineeringToolsDialog::onFindInFileClicked);
    connect(kMapButton, &QPushButton::clicked, this, &EngineeringToolsDialog::onKMapClicked);
    connect(clockTestButton, &QPushButton::clicked, this, &EngineeringToolsDialog::onClockTestClicked);
    connect(testPaneButton, &QPushButton::clicked, this, &EngineeringToolsDialog::onTestPaneClicked);
    connect(addSearchResultsButton, &QPushButton::clicked, this, &EngineeringToolsDialog::onAddSearchResultsClicked);
    connect(bulkReadButton, &QPushButton::clicked, this, &EngineeringToolsDialog::onBulkReadClicked);
    connect(sequentialReadButton, &QPushButton::clicked, this, &EngineeringToolsDialog::onSequentialReadClicked);
    connect(sequentialReadSyncButton, &QPushButton::clicked, this, &EngineeringToolsDialog::onSequentialReadSyncClicked);
    connect(completeCleanupButton, &QPushButton::clicked, this, &EngineeringToolsDialog::onCompleteCleanupClicked);
    connect(memoryStatusButton, &QPushButton::clicked, this, &EngineeringToolsDialog::onMemoryStatusClicked);
    connect(scrollToLineButton, &QPushButton::clicked, this, &EngineeringToolsDialog::onScrollToLineClicked);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    LOG_INFO("EngineeringToolsDialog: Created engineering tools dialog with all buttons");
}

void EngineeringToolsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Title
    QLabel *titleLabel = new QLabel("Engineering Tools");
    titleLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; color: #2c3e50; padding: 10px; }");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // File Operations Group
    QGroupBox *fileGroup = new QGroupBox("File Operations");
    QGridLayout *fileLayout = new QGridLayout(fileGroup);
    
    openFile2Button = new QPushButton("Open File2");
    openFile2Button->setToolTip("Open a secondary file for comparison or analysis");
    
    findInFileButton = new QPushButton("Find in File");
    findInFileButton->setToolTip("Advanced find functionality within the current file");
    
    fileLayout->addWidget(openFile2Button, 0, 0);
    fileLayout->addWidget(findInFileButton, 0, 1);
    
    // Testing Tools Group
    QGroupBox *testGroup = new QGroupBox("Testing & Analysis Tools");
    QGridLayout *testLayout = new QGridLayout(testGroup);
    
    kMapButton = new QPushButton("KMap");
    kMapButton->setToolTip("Karnaugh Map analysis tool");
    
    clockTestButton = new QPushButton("Clock Test");
    clockTestButton->setToolTip("Performance timing and clock testing");
    
    testPaneButton = new QPushButton("Test Pane");
    testPaneButton->setToolTip("Test pane functionality and operations");
    
    addSearchResultsButton = new QPushButton("addSearchResults");
    addSearchResultsButton->setToolTip("Test addSearchResults function directly");
    addSearchResultsButton->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #28a745; color: white; border: none; border-radius: 4px; font-weight: bold; }");
    
    testLayout->addWidget(kMapButton, 0, 0);
    testLayout->addWidget(clockTestButton, 0, 1);
    testLayout->addWidget(testPaneButton, 1, 0);
    testLayout->addWidget(addSearchResultsButton, 1, 1);
    
    // Data Processing Group
    QGroupBox *dataGroup = new QGroupBox("Data Processing");
    QGridLayout *dataLayout = new QGridLayout(dataGroup);
    
    bulkReadButton = new QPushButton("Bulk Read");
    bulkReadButton->setToolTip("Bulk data reading operations");
    
    sequentialReadButton = new QPushButton("Sequential Read");
    sequentialReadButton->setToolTip("Sequential data reading test");
    
    sequentialReadSyncButton = new QPushButton("Seq Read Sync");
    sequentialReadSyncButton->setToolTip("Synchronous sequential reading test");
    
    dataLayout->addWidget(bulkReadButton, 0, 0);
    dataLayout->addWidget(sequentialReadButton, 0, 1);
    dataLayout->addWidget(sequentialReadSyncButton, 1, 0);
    
    // Maintenance Group
    QGroupBox *maintenanceGroup = new QGroupBox("Maintenance");
    QHBoxLayout *maintenanceLayout = new QHBoxLayout(maintenanceGroup);
    
    completeCleanupButton = new QPushButton("FN: KCompleteCleanUp");
    completeCleanupButton->setToolTip("Complete system cleanup and maintenance");
    
    memoryStatusButton = new QPushButton("📊 Get Memory Status");
    memoryStatusButton->setToolTip("Get current memory usage and display in log and popup");
    memoryStatusButton->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #17a2b8; color: white; border: none; border-radius: 4px; }");
    
    maintenanceLayout->addWidget(completeCleanupButton);
    maintenanceLayout->addWidget(memoryStatusButton);
    maintenanceLayout->addStretch();
    
    // Add all groups to main layout
    mainLayout->addWidget(fileGroup);
    mainLayout->addWidget(testGroup);
    mainLayout->addWidget(dataGroup);
    mainLayout->addWidget(maintenanceGroup);
    
    // Action buttons
    QHBoxLayout *actionLayout = new QHBoxLayout();
    
    scrollToLineButton = new QPushButton("📍 Scroll to Line");
    scrollToLineButton->setToolTip("Scroll to a specific line number in the current file");
    scrollToLineButton->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #2196F3; color: white; border: none; border-radius: 4px; }");
    
    closeButton = new QPushButton("Apply");
    // Create check mark icon using Unicode character
    QPixmap checkPixmap(16, 16);
    checkPixmap.fill(Qt::transparent);
    QPainter painter(&checkPixmap);
    painter.setPen(QPen(Qt::white, 2));
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(checkPixmap.rect(), Qt::AlignCenter, "✓");
    closeButton->setIcon(QIcon(checkPixmap));
    closeButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #28a745, stop:1 #20c997);"
        "    border: none;"
        "    color: white;"
        "    padding: 8px 16px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    border-radius: 6px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #20c997, stop:1 #17a2b8);"
        "    transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #17a2b8, stop:1 #138496);"
        "    transform: translateY(1px);"
        "}"
    );
    
    actionLayout->addWidget(scrollToLineButton);
    actionLayout->addStretch();
    actionLayout->addWidget(closeButton);
    
    mainLayout->addLayout(actionLayout);
}

// Slot implementations - delegate to MainWindow methods
void EngineeringToolsDialog::onOpenFile2Clicked()
{
    LOG_INFO("EngineeringToolsDialog: Open File2 clicked");
    if (m_mainWindow) {
        m_mainWindow->openFile2();
    }
}

void EngineeringToolsDialog::onFindInFileClicked()
{
    LOG_INFO("EngineeringToolsDialog: Find in File clicked");
    if (m_mainWindow) {
        m_mainWindow->KFindInFile();
    }
}

void EngineeringToolsDialog::onKMapClicked()
{
    LOG_INFO("EngineeringToolsDialog: KMap clicked");
    if (m_mainWindow) {
        m_mainWindow->KKMap();
    }
}

void EngineeringToolsDialog::onClockTestClicked()
{
    LOG_INFO("EngineeringToolsDialog: Clock Test clicked");
    if (m_mainWindow) {
        m_mainWindow->KClockTest();
    }
}

void EngineeringToolsDialog::onTestPaneClicked()
{
    LOG_INFO("EngineeringToolsDialog: Test Pane clicked");
    if (m_mainWindow) {
        m_mainWindow->KTestPane();
    }
}

void EngineeringToolsDialog::onAddSearchResultsClicked()
{
    LOG_INFO("EngineeringToolsDialog: addSearchResults clicked");
    if (m_mainWindow) {
        // Test data for addSearchResults function
        QString testPattern = "test";
        QString testJsonData = R"({"type":"summary","data":{"elapsed_total":{"secs":0,"nanos":123456},"stats":{"bytes_searched":1024,"bytes_printed":512,"matches":5,"searches_with_match":2,"searches":3}}})";
        QString testSearchPath = "C:/test/path";
        
        LOG_INFO("EngineeringToolsDialog: Calling addSearchResults with test data");
        if (m_mainWindow->collapsibleSearchResults) {
            m_mainWindow->collapsibleSearchResults->addSearchResults(testPattern, testJsonData, testSearchPath);
            LOG_INFO("EngineeringToolsDialog: addSearchResults completed");
        } else {
            LOG_ERROR("EngineeringToolsDialog: collapsibleSearchResults is null");
        }
    }
}

void EngineeringToolsDialog::onBulkReadClicked()
{
    LOG_INFO("EngineeringToolsDialog: Bulk Read clicked");
    if (m_mainWindow) {
        m_mainWindow->KDisplayFile_BulkRead();
    }
}

void EngineeringToolsDialog::onSequentialReadClicked()
{
    LOG_INFO("EngineeringToolsDialog: Sequential Read clicked");
    if (m_mainWindow) {
        m_mainWindow->KDisplayFile_SequentialRead_Optimized();
    }
}

void EngineeringToolsDialog::onSequentialReadSyncClicked()
{
    LOG_INFO("EngineeringToolsDialog: Sequential Read Sync clicked");
    if (m_mainWindow) {
        m_mainWindow->KDisplayFile_SequentialRead_Sync();
    }
}

void EngineeringToolsDialog::onCompleteCleanupClicked()
{
    LOG_INFO("EngineeringToolsDialog: Complete Cleanup clicked");
    if (m_mainWindow) {
        m_mainWindow->getKSearch()->KCompleteCleanUp();
    }
}

void EngineeringToolsDialog::onMemoryStatusClicked()
{
    LOG_INFO("EngineeringToolsDialog: Get Memory Status clicked");
    if (m_mainWindow && m_mainWindow->getKSearch()) {
        qint64 memoryUsage = m_mainWindow->getKSearch()->getCurrentMemoryUsage();
        double memoryMB = memoryUsage / 1024.0 / 1024.0;
        
        // Log the memory usage
        LOG_INFO("EngineeringToolsDialog: Current memory usage: " + QString::number(memoryUsage) + " bytes (" + QString::number(memoryMB, 'f', 2) + " MB)");
        
        // Show popup with memory information
        QString message = QString("Current Memory Usage:\n\n"
                                "Raw: %1 bytes\n"
                                "MB: %2 MB\n\n"
                                "Details logged to console.").arg(memoryUsage).arg(memoryMB, 0, 'f', 2);
        
        QMessageBox::information(this, "Memory Status", message);
        LOG_INFO("EngineeringToolsDialog: Memory status displayed in popup");
    } else {
        LOG_ERROR("EngineeringToolsDialog: Cannot get memory status - MainWindow or KSearch is null");
        QMessageBox::warning(this, "Error", "Cannot get memory status - MainWindow or KSearch is null");
    }
}

void EngineeringToolsDialog::onScrollToLineClicked()
{
    LOG_INFO("EngineeringToolsDialog: Scroll to Line clicked");
    if (m_mainWindow) {
        m_mainWindow->KScrollToLine(-1);  // -1 means prompt for line number
    }
}
