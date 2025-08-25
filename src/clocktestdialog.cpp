#include "clocktestdialog.h"
#include <QDateTime>
#include <QFont>
#include <QApplication>

ClockTestDialog::ClockTestDialog(QWidget *parent)
    : QDialog(parent)
    , isRunning(false)
    , lastMeasurement(0)
{
    setWindowTitle("Clock Test - Time Measurement");
    setFixedSize(400, 300);
    
    // Create main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Create time display label
    timeLabel = new QLabel("00:00:00.000");
    timeLabel->setAlignment(Qt::AlignCenter);
    timeLabel->setFont(QFont("Arial", 24, QFont::Bold));
    timeLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 10px; border: 2px solid #ccc; }");
    mainLayout->addWidget(timeLabel);
    
    // Create button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    // Create Start/Stop button
    startStopButton = new QPushButton("Start");
    startStopButton->setFont(QFont("Arial", 12, QFont::Bold));
    startStopButton->setStyleSheet("QPushButton { padding: 10px; background-color: #4CAF50; color: white; border: none; }");
    buttonLayout->addWidget(startStopButton);
    
    // Create Reset button
    resetButton = new QPushButton("Reset");
    resetButton->setFont(QFont("Arial", 12));
    resetButton->setStyleSheet("QPushButton { padding: 10px; background-color: #f44336; color: white; border: none; }");
    buttonLayout->addWidget(resetButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Create log display
    logDisplay = new QTextEdit();
    logDisplay->setReadOnly(true);
    logDisplay->setFont(QFont("Consolas", 10));
    logDisplay->setStyleSheet("QTextEdit { background-color: #f8f8f8; border: 1px solid #ccc; }");
    mainLayout->addWidget(logDisplay);
    
    // Connect signals
    connect(startStopButton, &QPushButton::clicked, this, &ClockTestDialog::onStartStopClicked);
    connect(resetButton, &QPushButton::clicked, this, &ClockTestDialog::onResetClicked);
    
    // Initialize display
    updateDisplay();
    
    // Add initial log entry
    logDisplay->append("Clock Test initialized. Click 'Start' to begin timing.");
    LOG_INFO("ClockTestDialog: Clock Test window opened");
}

void ClockTestDialog::onStartStopClicked()
{
    if (!isRunning) {
        // Start timing
        timer.start();
        isRunning = true;
        startTime = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        
        startStopButton->setText("Stop");
        startStopButton->setStyleSheet("QPushButton { padding: 10px; background-color: #f44336; color: white; border: none; }");
        
        logDisplay->append("⏱️  Started timing at: " + startTime);
        LOG_INFO("ClockTestDialog: Started timing at " + startTime);
        
    } else {
        // Stop timing
        lastMeasurement = timer.elapsed();
        isRunning = false;
        stopTime = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        
        startStopButton->setText("Start");
        startStopButton->setStyleSheet("QPushButton { padding: 10px; background-color: #4CAF50; color: white; border: none; }");
        
        // Log the measurement
        logTimeMeasurement(lastMeasurement);
        
        logDisplay->append("⏹️  Stopped timing at: " + stopTime);
        LOG_INFO("ClockTestDialog: Stopped timing at " + stopTime);
    }
    
    updateDisplay();
}

void ClockTestDialog::onResetClicked()
{
    timer.invalidate();
    isRunning = false;
    lastMeasurement = 0;
    startTime.clear();
    stopTime.clear();
    
    startStopButton->setText("Start");
    startStopButton->setStyleSheet("QPushButton { padding: 10px; background-color: #4CAF50; color: white; border: none; }");
    
    updateDisplay();
    
    logDisplay->append("🔄 Reset timer");
    LOG_INFO("ClockTestDialog: Timer reset");
}

void ClockTestDialog::updateDisplay()
{
    qint64 elapsed = isRunning ? timer.elapsed() : lastMeasurement;
    
    // Convert to hours:minutes:seconds.milliseconds
    qint64 hours = elapsed / 3600000;
    qint64 minutes = (elapsed % 3600000) / 60000;
    qint64 seconds = (elapsed % 60000) / 1000;
    qint64 milliseconds = elapsed % 1000;
    
    QString timeText = QString("%1:%2:%3.%4")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
        .arg(milliseconds, 3, 10, QChar('0'));
    
    timeLabel->setText(timeText);
}

void ClockTestDialog::logTimeMeasurement(qint64 elapsedMs)
{
    // Convert to different time units
    qint64 hours = elapsedMs / 3600000;
    qint64 minutes = (elapsedMs % 3600000) / 60000;
    qint64 seconds = (elapsedMs % 60000) / 1000;
    qint64 milliseconds = elapsedMs % 1000;
    
    QString timeText = QString("%1:%2:%3.%4")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
        .arg(milliseconds, 3, 10, QChar('0'));
    
    QString logMessage = QString("📊 Time Measurement: %1 (%2 ms)")
        .arg(timeText)
        .arg(elapsedMs);
    
    logDisplay->append(logMessage);
    LOG_INFO("ClockTestDialog: " + logMessage);
} 