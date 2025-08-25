#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include "DetachablePane.h"
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QComboBox>
#include <QListView>
#include <QCompleter>
#include <QStringListModel>
#include <QElapsedTimer>
#include <QColor>
#include <QScrollArea>
#include <QTextEdit>
#include <QFontMetrics>
#include <QPainter>
#include <QApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QProgressBar>
#include "filelinemodel.h"
#include "scintillaedit.h"
#include "LogDataWorker.h"
#include "LogMainView.h"
#include "rgsearchdialog.h"
#include "CollapsibleSearchResults.h"
#include "highlightdialog.h"
#include "engineeringdialog.h"
#include "configurationdialog.h"
#include "KSearch.h"

// Forward declaration
class LineNumberDelegate;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    friend class KSearch;
    
    // KSearch instance
    KSearch* m_kSearch;
    
public:
    // Public accessor for KSearch
    KSearch* getKSearch() const { return m_kSearch; }
    
public:


    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    // Engineering tools - public access for engineering dialog
    void openFile2();
    void KFindInFile();
    void KKMap();
    void KClockTest();
    void KTestPane();
    void KDisplayFile_BulkRead();
    void KDisplayFile_SequentialRead_Optimized();
    void KDisplayFile_SequentialRead_Sync();

    void KScrollToLine(int lineNumber = -1);

private slots:
    void openFile();
    void saveFile();
    void searchText();
    void stopSearch();  // Stop search functionality
    void clearSearch();
    void updateScrollPosition(int value);
    void showPreferences();
    void showSearchParameters();
    void loadSettings();
    void saveToSearchHistory(const QString &searchTerm);
    void saveToFolderHistory(const QString &folderPath);
    QStringList getSearchHistory();
    QStringList getFolderHistory();
    void setupAutoComplete();                    // Setup auto-complete for pattern and path fields
    void addToPatternHistory(const QString &pattern);  // Add pattern to history and update auto-complete
    void addToPathHistory(const QString &path);        // Add path to history and update auto-complete
    void updatePatternCompleter();              // Update pattern auto-complete suggestions
    void updatePathCompleter();                 // Update path auto-complete suggestions
    void saveHistoryToFile();                   // Save history to local file
    void loadHistoryFromFile();                 // Load history from local file
    void saveSettings(const QString &searchResultsPosition, bool showLogWindow, 
                     const QString &searchEngine, bool caseSensitive, bool useRegex, 
                     bool wholeWord, bool ignoreHidden, bool followSymlinks, 
                     const QString &fileTypes, int maxDepth, const QString &excludePatterns, 
                     bool smartCase, bool multiline, bool dotAll, bool noIgnore, 
                     int tabWidth, bool useSpacesForTabs, int contextLines, 
                     QColor highlightColor, int chunkSize, int firstChunkSize, bool showLineNumbers);
    void saveFindInFileSettings(const QString &pattern, const QString &engine, const QString &type, bool caseSensitive,
                               bool inverse, bool boolean, bool plainText, bool autoRefresh,
                               int startLine, int endLine, QColor highlightColor);
    void applyFont(const QFont &font);
    void applyLayoutSettings();
    void applyViewerSettings();
    void onSearchResultSelected(QListWidgetItem *item);
    void performRipgrepSearch(const QString &searchTerm, const QString &startTime);
    void performBuiltinSearch(const QString &searchTerm, const QString &startTime);
    void logDebugMessage(const QString &message);
    void onFileLoaded(const QString &filePath);
    void onScintillaFileLoadError(const QString &error);
    void onFileLineNumberChanged(int fileLine);
    void onLoadingProgress(int chunksLoaded, int totalChunks);
    void onCollapsibleResultSelected(const QString &filePath, int lineNumber);

signals:
    void sequentialReadCompleted(const QString& fileContent);
    void sequentialReadError(const QString& error);

private:
    void setupUI();
    void setupMenuBar();
    void loadPaneSizes();
    void savePaneSizes();
    void setupToolBar();
    void setupCentralWidget();
    int parseRipgrepResults(const QString &jsonOutput);
    QString extractJsonValue(const QString &jsonLine, const QString &key, const QString &subKey = QString());
    QString decodeJsonString(const QString &jsonString);
    
    // New K-functions for file operations
    void KClean();
    bool KOpenFile(const QString& fileName);
    void KDisplayFile();
    void KRGSearch();
    void KUpdateFileViewer(const QString &filePath, int lineNumber, const QString &lineText);
    void KUpdateFileViewer2(const QString &filePath, int lineNumber);
    void KResultChoose(const QString &filePath, int lineNumber);
public:

    void logFunctionStart(const QString& functionName);
    void logFunctionEnd(const QString& functionName);
    void onSearchFinished(bool success, int resultCount, const QString& pattern, qint64 searchTime, const QColor& highlightColor);
    
    // Display search results in the Result Pane
    void KDisplayResultWin(const QString &pattern, const QMap<QString, QVector<Match>> &results);
    void KDisplayResultWin(const QString &pattern, const QString &jsonData, const QString &searchPath = QString());  // New overload for JSON data
    
    // New bulk read methods
    void KDisplayFile_SequentialRead();
    void KDisplayFile_SequentialRead_Background();
    void KDisplayFile_SequentialRead_WithPath(const QString& filePath); // New function with path parameter
    QString sanitizeForDisplay(const QByteArray& data);
    
    // Cleanup function
    void cleanupThreadAndMemory();
    
    // Helper function to update filename display
    void updateFilenameDisplay(const QString &filePath);
    
    // Helper function to update cache status display
    void updateCacheStatusDisplay();
    
    // Status lamp methods
    void setSearchResultsLamp(bool active);
    void setParsingLamp(bool active, int percentage = 0);

    void setOpenFileLamp(bool active);
    void setScrollHighlightLamp(bool active);
    void setHighlightExtraLamp(bool active);
    void setCleanLamp(bool active);
    void setBackgroundHighlightLamp(bool active, int progress = 0);
    
    // Status file for watchdog
    void writeStatusFile(const QString &state, int progress = 0, const QString &description = QString());
    
    // Cache integrity verification
    void verifyCacheIntegrity();
    
    // Fast file opening methods
    bool kOpenFileTransfetToContentFast(const QString& filePath);
    
    // Ultra-fast scroll and highlight function
  //  void FastScrollHighlight(int lineNumber, const QColor& highlightColor = QColor());
    
    // Fast in-file search
    void performFastInFileSearch(const QString &pattern, bool caseSensitive = false, bool wholeWord = false, bool useRegex = false);
    
    // Highlight in file
    void showHighlightDialog();
    void showEngineeringDialog();
    void showSearchParamsDialog();  // Renamed from RG Search dialog
    void performSearchWithCurrentParams();
    void browseForPath();
    void showConfigurationDialog();  // Configuration dialog
    ConfigurationDialog *m_configDialog;  // Configuration dialog instance
    
    // Search history management
    void addToSearchHistory(const QString &pattern, const QString &path);
    void loadSearchHistory();
    void saveSearchHistory();
    void updateSearchFieldsFromHistory();
    void clearAllSearchHistory();                      // Clear all pattern and path history
    
    // Search result line highlighting
    void highlightSearchResultLine(const QString &filePath, int lineNumber);

    void applyExtraHighlightsWithRG(const QString& filePath);
    void applyExtraHighlightParamsRules(); // New function
    
    // Method to apply background highlighting settings from Configuration dialog
    void applyBackgroundHighlightingSettingsFromConfig(bool backgroundEnabled, bool progressEnabled);
    
    // Method to apply layout settings from Configuration dialog
    void applyLayoutSettingsFromConfig(bool enableLogWindow, bool isUpDownLayout);
    
    // Helper method to apply horizontal layout
    void applyHorizontalLayout();
    
    // Helper method to apply vertical layout
    void applyVerticalLayout();
    
    // Search button state management


    
    // Background highlighting slots
    void onBackgroundHighlightProgress(int percentage);
    void onBackgroundHighlightCompleted();
    
    // Parsing progress slot
    void onParsingProgressUpdate(int percentage, int files);
    void onParsingCompleted(int totalMatches, int totalFiles);
    void onParsingError(const QString &error);
    
    // Progress bar control methods
    void showSearchProgress();
    void hideSearchProgress();
    void updateSearchProgress(int value);
    void updateParsingProgress(int percentage);
    void resetParsingProgress();
    
    // Event filter for mouse hover auto-complete
    bool eventFilter(QObject *obj, QEvent *event) override;
    
    // Heartbeat timer for watchdog
    void heartbeatTimer();
    
    // Parsing timeout detection
    void checkParsingTimeout();
    
    // Detachable pane slots
    void onSearchResultsPaneDetached(DetachablePane* pane);
    void onSearchResultsPaneAttached(DetachablePane* pane);
    void onSearchResultsPaneClosed(DetachablePane* pane);
    void onFileViewerPaneDetached(DetachablePane* pane);
    void onFileViewerPaneAttached(DetachablePane* pane);
    void onFileViewerPaneClosed(DetachablePane* pane);
    
    // UI Components
    QWidget *centralWidget;
    
    // Search area
    QLineEdit *patternEdit;      // Pattern field (moved from RG Search dialog)
    QLineEdit *pathEdit;         // Path field (moved from RG Search dialog)
    QPushButton *browseButton;   // Browse button for path selection
    QPushButton *searchButton;   // Search button (moved from RG Search dialog)
    QPushButton *stopButton;     // Stop search button
    QPushButton *rgSearchButton;
    QPushButton *highlightButton;
    QPushButton *engineeringButton;
    QPushButton *configButton;   // Configuration button
    QListWidget *searchResults;
    CollapsibleSearchResults *collapsibleSearchResults;
    
    // Detachable panes
    DetachablePane *searchResultsPane;
    DetachablePane *fileViewerPane;
    
    // File content area
    ScintillaEdit *fileContentView;
    QLabel *statusLamp;     // Status lamp indicator (blue=idle, green=working, red=error)
    QLabel *cacheStatusLabel; // Cache status indicator in status bar
    QLabel *searchResultsLamp; // Search results status lamp
    QLabel *parsingLamp; // Parsing status lamp
    QLabel *openFileLamp; // Open file status lamp
    QLabel *scrollHighlightLamp; // Scroll and highlight status lamp
    QLabel *highlightExtraLamp; // Highlight extra status lamp
    QLabel *cleanLamp; // Clean process status lamp
    QLabel *backgroundHighlightLamp; // Background highlighting progress lamp

    QTextEdit *logWidget;
    QTimer *m_parsingTimeoutTimer; // Timer to detect stuck parsing
    QSplitter *mainSplitter;  // Main splitter for pane sizes
    QString logFilePath;
    QString selectedFolderPath;
    QString m_currentFilePath;
    QString m_currentFilePath4NonCached;         // Current file path in non-cache mode (for optimization)
    
    // Search parameters (loaded from preferences)
    QString m_searchEngine;
    bool m_caseSensitive;
    bool m_useRegex;
    bool m_wholeWord;
    bool m_ignoreHidden;
    bool m_followSymlinks;
    QString m_fileTypes;
    int m_maxDepth;
    QString m_excludePatterns;
    QString m_rgSearchPattern;  // Store RG Search pattern for Pattern 0
    
    // Search history and auto-complete
    QStringList m_searchPatternHistory;  // History of search patterns
    QStringList m_searchPathHistory;     // History of search paths
    int m_maxHistorySize;                // Maximum history size
    QCompleter *m_patternCompleter;      // Auto-completer for pattern field
    QCompleter *m_pathCompleter;         // Auto-completer for path field
    QStringListModel *m_patternModel;    // Model for pattern suggestions
    QStringListModel *m_pathModel;       // Model for path suggestions
    
    // Search result line highlighting
    int m_currentSearchResultLine;     // Currently highlighted search result line
    QString m_currentSearchResultFile; // File path of current search result line
    QColor m_searchResultHighlightColor; // Color for search result line highlighting
    
    // Search state management
    bool m_isSearching;                // Track if a search is currently in progress
    
    bool m_smartCase;
    bool m_multiline;
    bool m_dotAll;
    bool m_noIgnore;
    
    // Layout settings
    QString m_searchResultsPosition;
    bool m_showLogWindow;
    bool m_isUpDownLayout;  // Track if layout is Up/Down or Side by Side
    
    // Viewer settings
    int m_contextLines;
    int m_tabWidth;
    bool m_useSpacesForTabs;
    QColor m_highlightColor;
    QColor m_rgSearchHighlightColor;  // Highlight color from RG search dialog
    int m_chunkSize;
    int m_firstChunkSize;
    bool m_showLineNumbers;
    
    // File passing tracking
    int m_filePassCount;
    
    // Buffer reuse and memory management
    QByteArray m_reusableBuffer;
    QThread* m_workerThread;
    QTimer* m_cleanupTimer;
    QTimer* m_progressTimer;
    QStringList m_contentChunks;
    int m_currentChunkIndex;
    bool m_isProcessing;
    QElapsedTimer m_filePassTimer;
    
    // Find in File settings
    QString m_findInFilePattern;
    QString m_findInFileEngine;
    QString m_findInFileType;
    bool m_findInFileCaseSensitive;
    bool m_findInFileInverse;
    bool m_findInFileBoolean;
    bool m_findInFilePlainText;
    bool m_findInFileAutoRefresh;
    int m_findInFileStartLine;
    int m_findInFileEndLine;
    QColor m_findInFileHighlightColor;
    
    // LogDataWorker for KLOGG-style file handling
    LogDataWorker *m_logDataWorker;
    
    // Separate search objects to avoid race conditions
    KSearchBun *m_searchBun;           // For search operations
    KSearchBun *m_parseBun;            // For parsing operations
    
    // Global state tracking
    SearchState m_currentState;
    
    // Track which files have been mapped in the current search session with their line offsets
    QMap<QString, QVector<long>> m_mappedFiles;
    
    // File cache for keeping multiple files in memory
    QMap<QString, LogDataWorker*> m_fileCache;  // Cache of loaded files
    QString m_currentActiveFile;                 // Currently displayed file
    bool m_keepFilesInCache;                     // Whether to keep files in cache (from RG search dialog)
    mutable QMutex m_cacheMutex;                 // Protect cache operations and ScintillaEdit transfers
    
    // New simple cache mechanism
    QStringList m_fileAlreadyOpen4CacheMode;     // List of file paths that are already open in cache mode
    QMap<QString, QString> m_cachedFileContent;  // Map file path to sanitized content
    
    // Connection handle for indexingFinished signal to prevent cleanup crashes
    QMetaObject::Connection m_indexingFinishedConnection;
    
    // Cache configuration - no limit on number of files
    // static const int MAX_CACHE_SIZE = 3;      // Removed - no longer limiting cache size
    
    // Pending scroll line for sequential read completion
    int m_pendingScrollLine;
    
    // Search session tracking for thread persistence
    QString m_currentSearchSession;  // Unique identifier for current search session
    
    // Extra highlighting system
    QList<HighlightRule> m_extraHighlightRules;
    bool m_highlightCaseSensitive;
    bool m_highlightSentence;
    bool m_useRGHighlight;
    bool m_useQtHighlight;  // Use Qt regex for highlighting (default)
    bool m_useViewportHighlight;   // Use viewport-only highlighting for large files
    
    // Heartbeat timer for watchdog
    QTimer *m_heartbeatTimer;
    
    // Progress bar for search operations
    QProgressBar *m_searchProgressBar;
    
    // Parsing progress tracking
    int m_totalExpectedMatches;
    bool m_isFirstParsingCall;
};

#endif // MAINWINDOW_H 