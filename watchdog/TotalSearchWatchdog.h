#ifndef TOTALSEARCHWATCHDOG_H
#define TOTALSEARCHWATCHDOG_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QDateTime>
#include <QSpinBox>

class TotalSearchWatchdog : public QMainWindow
{
    Q_OBJECT

public:
    explicit TotalSearchWatchdog(QWidget *parent = nullptr);
    ~TotalSearchWatchdog();

private slots:
    void checkApplicationStatus();
    void startMonitoring();
    void stopMonitoring();
    void restartTotalSearch();

private:
    void setupUI();
    void updateStatusDisplay(const QString &status, const QString &color = "black");
    void updateAppStatusLamp(const QString &status, const QString &color);
    void updateAppThreadLamp(const QString &threadName);
    void logMessage(const QString &message);
    bool isTotalSearchRunning();
    QString getTotalSearchStatus();
    
    // UI Components
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QLabel *m_statusLabel;
    QLabel *m_lastUpdateLabel;
    QTextEdit *m_logDisplay;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_restartButton;
    
    // Status lamps
    QLabel *m_appStatusLamp;
    QLabel *m_appThreadLamp;
    
    // Timeout controls
    QLabel *m_crashTimeoutLabel;
    QLabel *m_stuckTimeoutLabel;
    QSpinBox *m_crashTimeoutSpinBox;
    QSpinBox *m_stuckTimeoutSpinBox;
    
    // Monitoring
    QTimer *m_monitorTimer;
    bool m_isMonitoring;
    
    // Status file path
    QString m_statusFilePath;
    
    // Timeout parameters (user configurable)
    int m_crashTimeout;    // 5 seconds for crash detection
    int m_stuckTimeout;    // 10 seconds for stuck detection
    
    // Monitoring state
    QString m_lastState;
    QDateTime m_lastStateTime;
    QDateTime m_lastTimestampTime;
};

#endif // TOTALSEARCHWATCHDOG_H
