#ifndef ENGINEERINGDIALOG_H
#define ENGINEERINGDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

// Forward declaration
class MainWindow;

class EngineeringDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EngineeringDialog(MainWindow *mainWindow, QWidget *parent = nullptr);

private slots:
    void onTryButtonClicked();

private:
    void setupUI();
    
    MainWindow *m_mainWindow;
    QPushButton *tryButton;
    QPushButton *closeButton;
};

class EngineeringToolsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EngineeringToolsDialog(MainWindow *mainWindow, QWidget *parent = nullptr);

private slots:
    void onOpenFile2Clicked();
    void onFindInFileClicked();
    void onKMapClicked();
    void onClockTestClicked();
    void onTestPaneClicked();
    void onAddSearchResultsClicked();
    void onBulkReadClicked();
    void onSequentialReadClicked();
    void onSequentialReadSyncClicked();
    void onCompleteCleanupClicked();
    void onMemoryStatusClicked();
    void onScrollToLineClicked();

private:
    void setupUI();
    
    MainWindow *m_mainWindow;
    
    // Tool buttons
    QPushButton *openFile2Button;
    QPushButton *findInFileButton;
    QPushButton *kMapButton;
    QPushButton *clockTestButton;
    QPushButton *testPaneButton;
    QPushButton *addSearchResultsButton;
    QPushButton *bulkReadButton;
    QPushButton *sequentialReadButton;
    QPushButton *sequentialReadSyncButton;
    QPushButton *completeCleanupButton;
    QPushButton *memoryStatusButton;
    QPushButton *scrollToLineButton;
    QPushButton *closeButton;
};

#endif // ENGINEERINGDIALOG_H
