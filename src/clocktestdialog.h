#ifndef CLOCKTESTDIALOG_H
#define CLOCKTESTDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QElapsedTimer>
#include <QTextEdit>
#include "logger.h"

class ClockTestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClockTestDialog(QWidget *parent = nullptr);

private slots:
    void onStartStopClicked();
    void onResetClicked();

private:
    void updateDisplay();
    void logTimeMeasurement(qint64 elapsedMs);

    QPushButton *startStopButton;
    QPushButton *resetButton;
    QLabel *timeLabel;
    QTextEdit *logDisplay;
    
    QElapsedTimer timer;
    bool isRunning;
    qint64 lastMeasurement;
    
    QString startTime;
    QString stopTime;
};

#endif // CLOCKTESTDIALOG_H 