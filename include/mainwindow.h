#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logdata.h"
#include <QMainWindow>
#include <QTextEdit>
#include <QProgressBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>

// Main window for the basic KLOGG viewer
class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
private slots:
    void openFile();
    void onLoadingProgressed(int percent);
    void onLoadingFinished(bool success);
    void onIndexingProgressed(int percent);
    void onIndexingFinished(bool success);
    
private:
    void setupUI();
    void updateStatus(const QString& message);
    void displayFileContent();
    void displayLines(LineNumber firstLine, LinesCount count);
    
    // UI components
    QWidget* centralWidget_;
    QVBoxLayout* mainLayout_;
    QHBoxLayout* buttonLayout_;
    QPushButton* openButton_;
    QPushButton* reloadButton_;
    QTextEdit* textView_;
    QProgressBar* progressBar_;
    QLabel* statusLabel_;
    
    // Data
    LogData* logData_;
    QString currentFilePath_;
    bool isFileLoaded_;
    
    // Display settings
    static constexpr int DisplayChunkSize = 1000; // Lines to display at once
    LineNumber currentFirstLine_;
    LinesCount currentLineCount_;
};

#endif // MAINWINDOW_H 