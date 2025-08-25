#ifndef KSEARCH_H
#define KSEARCH_H

#include <QString>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QListWidget>
#include <QTreeWidget>
#include <QSettings>
#include <QElapsedTimer>
#include <QDateTime>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include <QStatusBar>
#include <QThread>
#include <QMutex>
#include <QVector>
#include <QMap>
#include <QColor>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QMetaObject>
#include <QMutexLocker>
#include "KSearchBun.h"  // For RGSearchParams
#include "highlightdialog.h"  // For HighlightRule

// Forward declarations
class MainWindow;
class CollapsibleSearchResults;
class KSearchBun;
class ScintillaEdit;
class LogDataWorker;

// Enums
enum class SearchButtonState {
    IDLE,           // Button shows "🔍 Search", enabled
    SEARCHING,      // Button shows "⏳ Searching...", disabled
    PROCESSING      // Button shows "⚙️ Processing...", disabled
};

enum class StopButtonState {
    DISABLED,       // Button shows "⏹️ Stop", disabled
    ENABLED,        // Button shows "⏹️ Stop", enabled
    STOPPING        // Button shows "🛑 Stopping...", disabled
};

enum class SearchState {
    IDLE,
    SEARCHING,
    PARSING_MAIN_SEARCH,
    MAPPING_IN_BACKGROUND,
    NAVIGATING_FILES,
    ERROR,
    STOP
};

// Global search state variable
extern SearchState g_currentSearchState;

// KSearch class to contain all search-related functions
class KSearch : public QObject
{
    Q_OBJECT

public:
    explicit KSearch(MainWindow* mainWindow, QObject* parent = nullptr);
    ~KSearch();

    // Main search workflow functions
    void KSsearchDo();

    // Button and UI state management
    void setSearchButton(SearchButtonState state);
    void setStopButton(StopButtonState state);  // Set stop button state

    // Status lamp functions
    void setSearchResultsLamp(bool active);

    // Cleanup and state management
    void KCompleteCleanUp();
    void updateSearchState(SearchState newState);

    // History management
    void addToSearchHistory(const QString &pattern, const QString &path);
    void addToPatternHistory(const QString &pattern);
    void addToPathHistory(const QString &path);

    // Highlighting functions
    void applyExtraHighlightParamsRules();
    void applyExtraHighlights();

    // Logging utilities
    void logFunctionStart(const QString& functionName);
    void logFunctionEnd(const QString& functionName);

    // Timer access
    qint64 getTotalSearchTime() const { return m_totalSearchTimer.elapsed(); }
    
    // Memory tracking
    qint64 getCurrentMemoryUsage() const;
    
    // Access to search components
    KSearchBun* getSearchBun() const;
    
    // Stop search functionality
    void stopSearch();
    
    // Process state querying
    QString getProcessStateInfo(QProcess* process) const;
    
    // Process termination
    void KKillProcess(QProcess* process, const QString& processName = "Unknown");
    
    // Thread termination
    void KKillThread(QThread* thread, const QString& threadName = "Unknown");
    
    // Rule 1 update
    void updateRule1Pattern(const QString &pattern);



private:
    MainWindow* m_mainSearch;
    QElapsedTimer m_functionTimer;
    QElapsedTimer m_totalSearchTimer;  // Timer for total search duration
};

#endif // KSEARCH_H
