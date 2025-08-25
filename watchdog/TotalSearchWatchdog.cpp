#include "TotalSearchWatchdog.h"
#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QJsonParseError>
#include <QDebug>
#include <QThread>

TotalSearchWatchdog::TotalSearchWatchdog(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_statusLabel(nullptr)
    , m_lastUpdateLabel(nullptr)
    , m_logDisplay(nullptr)
    , m_startButton(nullptr)
    , m_stopButton(nullptr)
    , m_restartButton(nullptr)
    , m_crashTimeoutLabel(nullptr)
    , m_stuckTimeoutLabel(nullptr)
    , m_crashTimeoutSpinBox(nullptr)
    , m_stuckTimeoutSpinBox(nullptr)
    , m_appStatusLamp(nullptr)
    , m_appThreadLamp(nullptr)
    , m_monitorTimer(nullptr)
    , m_isMonitoring(false)
    , m_crashTimeout(5000)
    , m_stuckTimeout(10000)
{
    setWindowTitle("TotalSearch Watchdog");
    setFixedSize(600, 500);
    
    // Set status file path
    m_statusFilePath = QDir::currentPath() + "/build/Release/status.json";
    
    setupUI();
    
    // Initialize timer
    m_monitorTimer = new QTimer(this);
    connect(m_monitorTimer, &QTimer::timeout, this, &TotalSearchWatchdog::checkApplicationStatus);
    
    logMessage("Watchdog initialized. Status file: " + m_statusFilePath);
}

TotalSearchWatchdog::~TotalSearchWatchdog()
{
    if (m_monitorTimer) {
        m_monitorTimer->stop();
    }
}

void TotalSearchWatchdog::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Status lamps
    QHBoxLayout *lampLayout = new QHBoxLayout();
    
    m_appStatusLamp = new QLabel("TotalSearch: Not Running");
    m_appStatusLamp->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; padding: 8px; border: 2px solid #ccc; border-radius: 4px; background-color: #f5f5f5; }");
    m_appStatusLamp->setMinimumWidth(200);
    
    m_appThreadLamp = new QLabel("Thread: None");
    m_appThreadLamp->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; padding: 8px; border: 2px solid #ccc; border-radius: 4px; background-color: #f5f5f5; }");
    m_appThreadLamp->setMinimumWidth(150);
    
    lampLayout->addWidget(m_appStatusLamp);
    lampLayout->addWidget(m_appThreadLamp);
    lampLayout->addStretch();
    
    m_mainLayout->addLayout(lampLayout);
    
    // Status display
    m_statusLabel = new QLabel("Status: Not Monitoring");
    m_statusLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; padding: 10px; border: 2px solid #ccc; border-radius: 5px; }");
    m_mainLayout->addWidget(m_statusLabel);
    
    // Last update display
    m_lastUpdateLabel = new QLabel("Last Update: Never");
    m_lastUpdateLabel->setStyleSheet("QLabel { font-size: 12px; color: #666; }");
    m_mainLayout->addWidget(m_lastUpdateLabel);
    
    // Timeout controls
    QHBoxLayout *timeoutLayout = new QHBoxLayout();
    
    m_crashTimeoutLabel = new QLabel("Crash Timeout (seconds):");
    m_crashTimeoutSpinBox = new QSpinBox();
    m_crashTimeoutSpinBox->setRange(1, 60);
    m_crashTimeoutSpinBox->setValue(5);
    m_crashTimeoutSpinBox->setSuffix("s");
    
    m_stuckTimeoutLabel = new QLabel("Stuck Timeout (seconds):");
    m_stuckTimeoutSpinBox = new QSpinBox();
    m_stuckTimeoutSpinBox->setRange(1, 120);
    m_stuckTimeoutSpinBox->setValue(10);
    m_stuckTimeoutSpinBox->setSuffix("s");
    
    timeoutLayout->addWidget(m_crashTimeoutLabel);
    timeoutLayout->addWidget(m_crashTimeoutSpinBox);
    timeoutLayout->addWidget(m_stuckTimeoutLabel);
    timeoutLayout->addWidget(m_stuckTimeoutSpinBox);
    timeoutLayout->addStretch();
    
    m_mainLayout->addLayout(timeoutLayout);
    
    // Control buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_startButton = new QPushButton("Start Monitoring");
    m_startButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px 16px; border: none; border-radius: 4px; font-weight: bold; }");
    connect(m_startButton, &QPushButton::clicked, this, &TotalSearchWatchdog::startMonitoring);
    buttonLayout->addWidget(m_startButton);
    
    m_stopButton = new QPushButton("Stop Monitoring");
    m_stopButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 8px 16px; border: none; border-radius: 4px; font-weight: bold; }");
    m_stopButton->setEnabled(false);
    connect(m_stopButton, &QPushButton::clicked, this, &TotalSearchWatchdog::stopMonitoring);
    buttonLayout->addWidget(m_stopButton);
    
    m_restartButton = new QPushButton("Restart TotalSearch");
    m_restartButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 8px 16px; border: none; border-radius: 4px; font-weight: bold; }");
    connect(m_restartButton, &QPushButton::clicked, this, &TotalSearchWatchdog::restartTotalSearch);
    buttonLayout->addWidget(m_restartButton);
    
    m_mainLayout->addLayout(buttonLayout);
    
    // Log display
    m_logDisplay = new QTextEdit();
    m_logDisplay->setReadOnly(true);
    m_logDisplay->setStyleSheet("QTextEdit { background-color: #f5f5f5; border: 1px solid #ccc; border-radius: 4px; font-family: 'Consolas', monospace; font-size: 11px; }");
    m_mainLayout->addWidget(m_logDisplay);
}

void TotalSearchWatchdog::startMonitoring()
{
    // Get timeout values from UI
    m_crashTimeout = m_crashTimeoutSpinBox->value() * 1000; // Convert to milliseconds
    m_stuckTimeout = m_stuckTimeoutSpinBox->value() * 1000; // Convert to milliseconds
    
    m_isMonitoring = true;
    m_monitorTimer->start(1000); // 1 second interval
    
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    
    updateStatusDisplay("Monitoring Started", "green");
    logMessage("Started monitoring TotalSearch application");
    logMessage("Crash timeout: " + QString::number(m_crashTimeout/1000) + "s, Stuck timeout: " + QString::number(m_stuckTimeout/1000) + "s");
}

void TotalSearchWatchdog::stopMonitoring()
{
    m_isMonitoring = false;
    m_monitorTimer->stop();
    
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    
    updateStatusDisplay("Monitoring Stopped", "orange");
    logMessage("Stopped monitoring TotalSearch application");
}

void TotalSearchWatchdog::checkApplicationStatus()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    m_lastUpdateLabel->setText("Last Update: " + currentTime.toString("hh:mm:ss"));
    
    // 1) Check if TotalSearch is running and print it
    bool isRunning = isTotalSearchRunning();
    logMessage("isTotalSearchRunning: " + QString(isRunning ? "YES" : "NO"));
    
    // 2.1) Read JSON and parse
    QString currentStatus = getTotalSearchStatus();
    if (currentStatus.isEmpty()) {
        updateStatusDisplay("❌ CRASH DETECTED - No status file", "red");
        updateAppStatusLamp("App Stopped", "red");
        updateAppThreadLamp("None");
        logMessage("CRASH DETECTED: No status file found");
        return;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(currentStatus.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        updateStatusDisplay("⚠️ Invalid JSON format", "orange");
        updateAppStatusLamp("App Stopped", "red");
        updateAppThreadLamp("None");
        logMessage("Warning: Invalid JSON format");
        return;
    }
    
    QJsonObject statusObj = doc.object();
    QString state = statusObj["state"].toString();
    QString timestampStr = statusObj["timestamp"].toString();
    
    QDateTime statusTime = QDateTime::fromString(timestampStr, "yyyy-MM-dd hh:mm:ss.zzz");
    if (!statusTime.isValid()) {
        updateStatusDisplay("⚠️ Invalid timestamp format", "orange");
        updateAppStatusLamp("App Stopped", "red");
        updateAppThreadLamp("None");
        logMessage("Warning: Invalid timestamp format");
        return;
    }
    
    // 2.2) Check if state is the same for 10 seconds - app stuck (beside IDLE)
    if (state == m_lastState && state != "IDLE") {
        qint64 timeSinceLastStateChange = m_lastStateTime.msecsTo(currentTime);
        if (timeSinceLastStateChange > m_stuckTimeout) {
            updateStatusDisplay("⚠️ STUCK DETECTED - State unchanged for " + QString::number(timeSinceLastStateChange/1000) + "s", "orange");
            updateAppStatusLamp("Stuck", "red");
            updateAppThreadLamp(state);
            logMessage("STUCK DETECTED: State '" + state + "' unchanged for " + QString::number(timeSinceLastStateChange/1000) + " seconds");
            return;
        }
    } else {
        // State changed, update tracking
        m_lastState = state;
        m_lastStateTime = currentTime;
    }
    
    // 2.3) Check if timestamp is not updated for 5 seconds - then app crashed
    qint64 timeSinceLastUpdate = statusTime.msecsTo(currentTime);
    if (timeSinceLastUpdate > m_crashTimeout) {
        // Double check if process is still running
        bool stillRunning = isTotalSearchRunning();
        if (!stillRunning) {
            updateStatusDisplay("❌ CRASH DETECTED - Process not running", "red");
            updateAppStatusLamp("App Stopped", "red");
            updateAppThreadLamp("None");
            logMessage("CRASH DETECTED: Process not running, timestamp old by " + QString::number(timeSinceLastUpdate/1000) + "s");
        } else {
            updateStatusDisplay("❌ CRASH DETECTED - No timestamp updates for " + QString::number(timeSinceLastUpdate/1000) + "s", "red");
            updateAppStatusLamp("App Stopped but in Background", "orange");
            updateAppThreadLamp("Unknown");
            logMessage("CRASH DETECTED: No timestamp updates for " + QString::number(timeSinceLastUpdate/1000) + " seconds");
        }
        return;
    }
    
    // All good
    QString statusMessage = "✅ OK - " + state;
    QString color = (state == "IDLE") ? "green" : "blue";
    updateStatusDisplay(statusMessage, color);
    updateAppStatusLamp("Running", "green");
    updateAppThreadLamp(state);
    logMessage("Status: " + state + " (timestamp age: " + QString::number(timeSinceLastUpdate/1000) + "s)");
}

void TotalSearchWatchdog::updateStatusDisplay(const QString &status, const QString &color)
{
    QString styleSheet = QString("QLabel { font-size: 16px; font-weight: bold; padding: 10px; border: 2px solid %1; border-radius: 5px; background-color: %2; }")
                        .arg(color == "red" ? "#d32f2f" : color == "orange" ? "#f57c00" : color == "green" ? "#388e3c" : color == "blue" ? "#1976d2" : "#ccc")
                        .arg(color == "red" ? "#ffebee" : color == "orange" ? "#fff3e0" : color == "green" ? "#e8f5e8" : color == "blue" ? "#e3f2fd" : "#f5f5f5");
    
    m_statusLabel->setStyleSheet(styleSheet);
    m_statusLabel->setText(status);
}

void TotalSearchWatchdog::updateAppStatusLamp(const QString &status, const QString &color)
{
    QString styleSheet = QString("QLabel { font-size: 14px; font-weight: bold; padding: 8px; border: 2px solid %1; border-radius: 4px; background-color: %2; }")
                        .arg(color == "red" ? "#d32f2f" : color == "orange" ? "#f57c00" : color == "green" ? "#388e3c" : "#ccc")
                        .arg(color == "red" ? "#ffebee" : color == "orange" ? "#fff3e0" : color == "green" ? "#e8f5e8" : "#f5f5f5");
    
    m_appStatusLamp->setStyleSheet(styleSheet);
    m_appStatusLamp->setText("TotalSearch: " + status);
}

void TotalSearchWatchdog::updateAppThreadLamp(const QString &threadName)
{
    m_appThreadLamp->setText("Thread: " + threadName);
}

void TotalSearchWatchdog::logMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString logEntry = QString("[%1] %2").arg(timestamp, message);
    
    m_logDisplay->append(logEntry);
    
    // Auto-scroll to bottom
    QTextCursor cursor = m_logDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logDisplay->setTextCursor(cursor);
}

bool TotalSearchWatchdog::isTotalSearchRunning()
{
    QProcess process;
    process.start("tasklist", QStringList() << "/FI" << "IMAGENAME eq TotalSearch.exe");
    process.waitForFinished();
    
    QString output = process.readAllStandardOutput();
    return output.contains("TotalSearch.exe");
}

QString TotalSearchWatchdog::getTotalSearchStatus()
{
    QFile statusFile(m_statusFilePath);
    
    if (!statusFile.exists()) {
        return QString();
    }
    
    if (!statusFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    
    QString content = statusFile.readAll();
    statusFile.close();
    
    return content.trimmed();
}

void TotalSearchWatchdog::restartTotalSearch()
{
    logMessage("Attempting to restart TotalSearch...");
    
    // Kill existing process
    QProcess::execute("taskkill", QStringList() << "/F" << "/IM" << "TotalSearch.exe");
    
    // Wait a moment
    QThread::msleep(1000);
    
    // Start new process
    QString exePath = QDir::currentPath() + "/build/Release/TotalSearch.exe";
    QProcess::startDetached(exePath);
    
    logMessage("TotalSearch restart initiated");
    updateStatusDisplay("🔄 Restarting TotalSearch...", "blue");
}
