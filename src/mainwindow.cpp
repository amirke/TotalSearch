#include "mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include "preferencesdialog.h"
#include "logger.h"
#include "searchdialog.h"
#include "clocktestdialog.h"
#include <QThread>
#include <QElapsedTimer>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QScrollBar>
#include <QRegularExpression>
#include <QFontDialog>
#include <QTimer>
#include <QEventLoop>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QJsonParseError>
#include <QDateTime>
#include <QStringConverter>
#include <QThreadPool>
#include <QFileInfo>
#include <QTextStream>
#include <QMetaObject>
#include <QScrollArea>
#include <QPainter>
#include <QFontMetrics>
#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QStatusBar>
#include <QSplitter>
#include <QGroupBox>
#include <QCheckBox>
#include <QTextEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_contextLines(5)
    , m_tabWidth(4)
    , m_useSpacesForTabs(true)
    , m_highlightColor(Qt::yellow)
    , m_rgSearchHighlightColor(Qt::yellow)
    , m_chunkSize(5)  // Default 5MB like KLOGG
    , m_firstChunkSize(16)  // Default 16MB like KLOGG
    , m_showLineNumbers(true)  // Default show line numbers
    , m_filePassCount(0)  // Initialize file pass counter
    , logWidget(nullptr)
    , fileContentView(nullptr)

    , m_logDataWorker(nullptr)
    , m_searchBun(nullptr)
    , m_currentState(SearchState::IDLE)
    , logFilePath("")
    , m_workerThread(nullptr)
    , m_cleanupTimer(nullptr)
    , m_progressTimer(nullptr)
    , m_heartbeatTimer(nullptr)
    , m_currentChunkIndex(0)
    , m_isProcessing(false)
    , m_keepFilesInCache(false)  // Initialize cache setting to false (disabled by default)
    , m_fileAlreadyOpen4CacheMode()  // Initialize empty list for cache mode
    , m_cachedFileContent()  // Initialize empty map for cached content
    , m_extraHighlightRules()  // Initialize empty list for extra highlight rules
    , m_highlightCaseSensitive(false)  // Initialize case sensitivity to false
    , m_highlightSentence(false)  // Initialize sentence highlighting to false
    , m_useRGHighlight(false)  // Initialize RG highlighting to false
    , m_currentFilePath4NonCached("")  // Initialize non-cache file path tracking
    , m_currentSearchResultLine(-1)  // No search result line highlighted initially
    , m_currentSearchResultFile("")  // No search result file initially
    , m_searchResultHighlightColor(QColor(130, 130, 130))  // Default grey from search params
    , m_maxHistorySize(20)  // Maximum 20 items in history
    , m_isSearching(false)  // No search in progress initially
    , m_configDialog(nullptr)  // Configuration dialog instance
    , m_patternCompleter(nullptr)  // Auto-completer for pattern field
    , m_pathCompleter(nullptr)     // Auto-completer for path field
    , m_patternModel(nullptr)      // Model for pattern suggestions
    , m_pathModel(nullptr)         // Model for path suggestions
{
    // Initialize logger with log file in '../data/logs' subfolder
    QString appDir = QCoreApplication::applicationDirPath();
    QString logDir = appDir + "/data/logs";
    QDir().mkpath(logDir); // Create log directory if it doesn't exist
    QString logFile = logDir + "/totalsearch.log";
    
    Logger::instance().initialize(nullptr, logFile);
    LOG_INFO("TotalSearch application started");
    
    setupUI();
    setupMenuBar();
    setupToolBar();
    
    // Initialize persistent search object for async operations
    m_searchBun = new KSearchBun(this);
    m_searchBun->setMainWindow(this);
    
    // Initialize separate parse object for parsing operations
    m_parseBun = new KSearchBun(this);
    m_parseBun->setMainWindow(this);


    // Create KSearch instance
    m_kSearch = new KSearch(this, this);
    
    // Lambda function for search action (reusable for button click and Enter key)
    auto performSearch = [this]() {
        // Reset progress bar and parsing progress
        showSearchProgress();
        resetParsingProgress();
        
        try {
            m_kSearch->KSsearchDo();
        } catch (const std::exception& e) {
            LOG_ERROR("Search exception: " + QString(e.what()));
            QMessageBox::critical(this, "Error", "Search failed: " + QString(e.what()));
            hideSearchProgress();
        } catch (...) {
            LOG_ERROR("Search unknown exception");
            QMessageBox::critical(this, "Error", "Search failed with unknown error");
            hideSearchProgress();
        }
    };
    
    // Connect search button
    connect(searchButton, &QPushButton::clicked, performSearch);
    
    // Connect Enter key in pattern field to trigger search
    connect(patternEdit, &QLineEdit::returnPressed, performSearch);
    
    connect(stopButton, &QPushButton::clicked, [this]() {
        try {
            stopSearch();
            hideSearchProgress();
        } catch (const std::exception& e) {
            LOG_ERROR("Stop button exception: " + QString(e.what()));
            QMessageBox::critical(this, "Error", "Stop search failed: " + QString(e.what()));
            hideSearchProgress();
        } catch (...) {
            LOG_ERROR("Stop button unknown exception");
            QMessageBox::critical(this, "Error", "Stop search failed with unknown error");
            hideSearchProgress();
        }
    });
    connect(browseButton, &QPushButton::clicked, [this]() {
        browseForPath();
    });
    connect(rgSearchButton, &QPushButton::clicked, [this]() {
        showSearchParamsDialog();
    });
    connect(highlightButton, &QPushButton::clicked, [this]() {
        showHighlightDialog();
    });
    connect(engineeringButton, &QPushButton::clicked, [this]() {
        showEngineeringDialog();
    });
    connect(configButton, &QPushButton::clicked, [this]() {
        showConfigurationDialog();
    });



    connect(searchResults, &QListWidget::itemClicked, this, &MainWindow::onSearchResultSelected);
    connect(fileContentView, &ScintillaEdit::fileLoadError, this, &MainWindow::onScintillaFileLoadError);
    connect(fileContentView, &ScintillaEdit::fileLineNumberChanged, this, &MainWindow::onFileLineNumberChanged);
    connect(fileContentView, &ScintillaEdit::loadingProgress, this, &MainWindow::onLoadingProgress);
    
    loadSettings();
    
    // Apply saved layout setting on startup
    if (!m_isUpDownLayout) {
        LOG_INFO("MainWindow: Applying saved Side by Side layout on startup");
        applyHorizontalLayout();
    } else {
        LOG_INFO("MainWindow: Using default Up/Down layout on startup");
    }
    
    loadSearchHistory();
    
    // Initialize heartbeat timer for watchdog
    m_heartbeatTimer = new QTimer(this);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &MainWindow::heartbeatTimer);
    m_heartbeatTimer->start(1000); // 1 second interval
    
    // Initialize parsing progress tracking
    m_totalExpectedMatches = 0;
    m_isFirstParsingCall = true;
    
    updateSearchFieldsFromHistory();
    setupAutoComplete();
    applyLayoutSettings();
    applyViewerSettings();
    
    setWindowTitle("TotalSearch - Advanced File Search and Viewer");
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
    // Cleanup thread-related resources
    if (m_workerThread) {
        if (m_workerThread->isRunning()) {
            m_workerThread->quit();
            m_workerThread->wait(1000);
        }
        delete m_workerThread;
        m_workerThread = nullptr;
    }
    
    if (m_cleanupTimer) {
        m_cleanupTimer->stop();
        delete m_cleanupTimer;
        m_cleanupTimer = nullptr;
    }
    
    if (m_progressTimer) {
        m_progressTimer->stop();
        delete m_progressTimer;
        m_progressTimer = nullptr;
    }
    
    if (m_heartbeatTimer) {
        m_heartbeatTimer->stop();
        delete m_heartbeatTimer;
        m_heartbeatTimer = nullptr;
    }
    
    // Cleanup search objects
    if (m_searchBun) {
        delete m_searchBun;
        m_searchBun = nullptr;
    }
    
    if (m_parseBun) {
        delete m_parseBun;
        m_parseBun = nullptr;
    }
    
    // Cleanup configuration dialog
    if (m_configDialog) {
        delete m_configDialog;
        m_configDialog = nullptr;
    }
    
    saveSettings(m_searchResultsPosition, m_showLogWindow, m_searchEngine, m_caseSensitive, m_useRegex, 
                m_wholeWord, m_ignoreHidden, m_followSymlinks, m_fileTypes, m_maxDepth, m_excludePatterns, 
                m_smartCase, m_multiline, m_dotAll, m_noIgnore, m_tabWidth, m_useSpacesForTabs, m_contextLines, 
                m_highlightColor, m_chunkSize, m_firstChunkSize, m_showLineNumbers);
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Main vertical layout: all panes stacked vertically
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Search Controls (Top)
    QVBoxLayout *searchControlsLayout = new QVBoxLayout();
    
    // Search Controls label removed as requested
    
    // Pattern and Path fields (moved from RG Search dialog)
    QHBoxLayout *fieldsLayout = new QHBoxLayout();
    
    // Pattern field with history dropdown
    QLabel *patternLabel = new QLabel("Pattern:");
    patternLabel->setStyleSheet("QLabel { font-weight: 600; color: #374151; font-family: 'Segoe UI', 'Inter', -apple-system, sans-serif; }");
    
    // Create pattern field container with dropdown button
    QWidget *patternContainer = new QWidget();
    QHBoxLayout *patternLayout = new QHBoxLayout(patternContainer);
    patternLayout->setContentsMargins(0, 0, 0, 0);
    patternLayout->setSpacing(0);
    
    patternEdit = new QLineEdit();
    patternEdit->setPlaceholderText("Enter search pattern...");
    patternEdit->setToolTip("Search pattern (regex or literal text)\n\nExamples:\n- function.*main (regex)\n- TODO: (literal text)\n- \\b(error|warning)\\b (word boundaries)\n\nPress Ctrl+Space to show history dropdown");
    patternEdit->setStyleSheet(
        "QLineEdit {"
        "    border: 2px solid #e5e7eb;"
        "    border-radius: 8px 0px 0px 8px;"
        "    padding: 12px 16px;"
        "    font-size: 14px;"
        "    background-color: #ffffff;"
        "    color: #374151;"
        "    selection-background-color: #2563eb;"
        "    font-family: 'Segoe UI', 'Inter', -apple-system, sans-serif;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #2563eb;"
        "    box-shadow: 0 0 0 3px rgba(37, 99, 235, 0.1);"
        "    outline: none;"
        "}"
        "QLineEdit:hover {"
        "    border-color: #d1d5db;"
        "}"
    );
    
    // Pattern history dropdown button
    QPushButton *patternHistoryBtn = new QPushButton("▼");
    patternHistoryBtn->setToolTip("Show pattern history (Ctrl+Space)");
    patternHistoryBtn->setStyleSheet(
        "QPushButton {"
        "    border: 2px solid #e5e7eb;"
        "    border-left: none;"
        "    border-radius: 0px 8px 8px 0px;"
        "    padding: 12px 8px;"
        "    font-size: 10px;"
        "    background-color: #f9fafb;"
        "    color: #6b7280;"
        "    min-width: 20px;"
        "    max-width: 20px;"
        "    font-family: 'Segoe UI', 'Inter', -apple-system, sans-serif;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f3f4f6;"
        "    color: #374151;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #e5e7eb;"
        "}"
    );
    
    // Connect pattern history button
    connect(patternHistoryBtn, &QPushButton::clicked, [this]() {
        if (patternEdit && patternEdit->completer()) {
            patternEdit->completer()->complete();
            LOG_INFO("MainWindow: Pattern history button clicked");
        }
    });
    
    patternLayout->addWidget(patternEdit);
    patternLayout->addWidget(patternHistoryBtn);
    
    // Path field with history dropdown
    QLabel *pathLabel = new QLabel("Path:");
    pathLabel->setStyleSheet("QLabel { font-weight: 600; color: #374151; font-family: 'Segoe UI', 'Inter', -apple-system, sans-serif; }");
    
    // Create path field container with dropdown button
    QWidget *pathContainer = new QWidget();
    QHBoxLayout *pathLayout = new QHBoxLayout(pathContainer);
    pathLayout->setContentsMargins(0, 0, 0, 0);
    pathLayout->setSpacing(0);
    
    pathEdit = new QLineEdit();
    pathEdit->setPlaceholderText("Enter path to search...");
    pathEdit->setToolTip("Directory path to search in\n\nExamples:\n- C:\\MyProject\\src\n- /home/user/documents\n- . (current directory)\n\nPress Ctrl+Space to show history dropdown");
    pathEdit->setText(QDir::currentPath()); // Set default to current directory
    pathEdit->setStyleSheet(
        "QLineEdit {"
        "    border: 2px solid #e5e7eb;"
        "    border-radius: 8px 0px 0px 8px;"
        "    padding: 12px 16px;"
        "    font-size: 14px;"
        "    background-color: #ffffff;"
        "    color: #374151;"
        "    selection-background-color: #2563eb;"
        "    font-family: 'Segoe UI', 'Inter', -apple-system, sans-serif;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #2563eb;"
        "    box-shadow: 0 0 0 3px rgba(37, 99, 235, 0.1);"
        "    outline: none;"
        "}"
        "QLineEdit:hover {"
        "    border-color: #d1d5db;"
        "}"
    );
    
    // Path history dropdown button
    QPushButton *pathHistoryBtn = new QPushButton("▼");
    pathHistoryBtn->setToolTip("Show path history (Ctrl+Space)");
    pathHistoryBtn->setStyleSheet(
        "QPushButton {"
        "    border: 2px solid #e5e7eb;"
        "    border-left: none;"
        "    border-radius: 0px 8px 8px 0px;"
        "    padding: 12px 8px;"
        "    font-size: 10px;"
        "    background-color: #f9fafb;"
        "    color: #6b7280;"
        "    min-width: 20px;"
        "    max-width: 20px;"
        "    font-family: 'Segoe UI', 'Inter', -apple-system, sans-serif;"
        "}"
        "QPushButton:hover {"
        "    background-color: #f3f4f6;"
        "    color: #374151;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #e5e7eb;"
        "}"
    );
    
    // Connect path history button
    connect(pathHistoryBtn, &QPushButton::clicked, [this]() {
        if (pathEdit && pathEdit->completer()) {
            pathEdit->completer()->complete();
            LOG_INFO("MainWindow: Path history button clicked");
        }
    });
    
    pathLayout->addWidget(pathEdit);
    pathLayout->addWidget(pathHistoryBtn);
    
    browseButton = new QPushButton("Browse");
    browseButton->setIcon(QIcon(":/icons/open.png"));
    browseButton->setToolTip("Browse and select a directory to search");
    browseButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #795548, stop:1 #5D4037);"
        "    border: none;"
        "    color: white;"
        "    padding: 8px 16px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    border-radius: 6px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #8D6E63, stop:1 #795548);"
        "    transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4E342E, stop:1 #3E2723);"
        "    transform: translateY(1px);"
        "}"
    );
    
    fieldsLayout->addWidget(patternLabel);
    fieldsLayout->addWidget(patternContainer, 1); // Give pattern field more space
    fieldsLayout->addWidget(pathLabel);
    fieldsLayout->addWidget(pathContainer, 1); // Give path field more space
    fieldsLayout->addWidget(browseButton);
    
    searchControlsLayout->addLayout(fieldsLayout);
    
    // Button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    searchButton = new QPushButton("🔍 Search");
    searchButton->setToolTip("Start ripgrep search with current pattern and path");
    searchButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4CAF50, stop:1 #45a049);"
        "    border: none;"
        "    color: white;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "    border-radius: 8px;"
        "    min-width: 100px;"
        "    max-width: 100px;"
        "    box-shadow: 0 2px 4px rgba(0,0,0,0.2);"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5CBF60, stop:1 #4CAF50);"
        "    transform: translateY(-2px);"
        "    box-shadow: 0 4px 8px rgba(0,0,0,0.3);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3e8e41, stop:1 #357a38);"
        "    transform: translateY(0px);"
        "    box-shadow: 0 1px 2px rgba(0,0,0,0.2);"
        "}"
    );
    

    
    rgSearchButton = new QPushButton("🔧 Search Params"); // Renamed from "RG Search"
    rgSearchButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FF9800, stop:1 #F57C00);"
        "    border: none;"
        "    color: white;"
        "    padding: 8px 16px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    border-radius: 6px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFB74D, stop:1 #FF9800);"
        "    transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E65100, stop:1 #BF360C);"
        "    transform: translateY(1px);"
        "}"
    );
    
    highlightButton = new QPushButton("Highlight");
    highlightButton->setIcon(QIcon(":/icons/search_results.png"));
    highlightButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #9C27B0, stop:1 #7B1FA2);"
        "    border: none;"
        "    color: white;"
        "    padding: 8px 16px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    border-radius: 6px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #BA68C8, stop:1 #9C27B0);"
        "    transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6A1B9A, stop:1 #4A148C);"
        "    transform: translateY(1px);"
        "}"
    );
    
    engineeringButton = new QPushButton("Engineering");
    engineeringButton->setIcon(QIcon(":/icons/list.png"));
    engineeringButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #607D8B, stop:1 #455A64);"
        "    border: none;"
        "    color: white;"
        "    padding: 8px 16px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    border-radius: 6px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #78909C, stop:1 #607D8B);"
        "    transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #37474F, stop:1 #263238);"
        "    transform: translateY(1px);"
        "}"
    );
    
    // Add Search and Stop buttons to the left side
    buttonLayout->addWidget(searchButton);
    
    // Create Stop button
    stopButton = new QPushButton("⏹️ Stop");
    stopButton->setToolTip("Stop current search and cleanup processes");
    stopButton->setEnabled(false); // Initially disabled
    stopButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #dc3545, stop:1 #c82333);"
        "    border: none;"
        "    color: white;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "    border-radius: 8px;"
        "    min-width: 80px;"
        "    max-width: 80px;"
        "    box-shadow: 0 2px 4px rgba(0,0,0,0.2);"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e74c3c, stop:1 #dc3545);"
        "    transform: translateY(-2px);"
        "    box-shadow: 0 4px 8px rgba(0,0,0,0.3);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #bd2130, stop:1 #a71e2a);"
        "    transform: translateY(0px);"
        "    box-shadow: 0 1px 2px rgba(0,0,0,0.2);"
        "}"
        "QPushButton:disabled {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6c757d, stop:1 #495057);"
        "    color: #adb5bd;"
        "    opacity: 0.6;"
        "    box-shadow: 0 1px 2px rgba(0,0,0,0.1);"
        "}"
    );
    
    buttonLayout->addWidget(stopButton);
    buttonLayout->addStretch(); // Add stretch to push other buttons to the right
    
    // Create Config button
    configButton = new QPushButton("Config", this);
    configButton->setIcon(QIcon(":/icons/file.png")); // Using file icon for config
    configButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6c757d, stop:1 #495057);"
        "    border: none;"
        "    color: white;"
        "    padding: 8px 16px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    border-radius: 6px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #868e96, stop:1 #6c757d);"
        "    transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #495057, stop:1 #343a40);"
        "    transform: translateY(1px);"
        "}"
    );
    
    buttonLayout->addWidget(rgSearchButton);
    buttonLayout->addWidget(highlightButton);
    buttonLayout->addWidget(engineeringButton);
    buttonLayout->addWidget(configButton);
    
    searchControlsLayout->addLayout(buttonLayout);
    
    // Add beautiful modern progress bar under the buttons
    m_searchProgressBar = new QProgressBar();
    m_searchProgressBar->setVisible(true); // Always visible
    m_searchProgressBar->setTextVisible(false); // No text, just the bar
    m_searchProgressBar->setRange(0, 100);
    m_searchProgressBar->setValue(0);
    m_searchProgressBar->setFixedHeight(28); // Increased height for better visibility
    m_searchProgressBar->setMinimumWidth(300); // Ensure minimum width
    m_searchProgressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // Allow horizontal expansion
    m_searchProgressBar->setStyleSheet(
        "QProgressBar {"
        "    border: 3px solid #d1d5db;"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:0.5 #f8fafc, stop:1 #f1f5f9);"
        "    border-radius: 14px;"
        "    margin: 10px 0px;"
        "    padding: 4px;"
        "    min-height: 24px;"
        "    max-height: 28px;"
        "    box-shadow: inset 0 2px 4px rgba(0, 0, 0, 0.1);"
        "}"
        "QProgressBar::chunk {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3b82f6, stop:0.5 #2563eb, stop:1 #1d4ed8);"
        "    border-radius: 12px;"
        "    border: 1px solid #1e40af;"
        "    margin: 2px;"
        "    box-shadow: 0 2px 8px rgba(59, 130, 246, 0.4);"
        "}"
    );
    
    searchControlsLayout->addWidget(m_searchProgressBar);
    
    // Search Results (Below Search Controls)
    QVBoxLayout *searchResultsLayout = new QVBoxLayout();
    
    // Search Results label removed - header now in tree widget
    
    // Create both old and new search results widgets
    searchResults = new QListWidget();
    searchResults->setObjectName("searchResults");
    searchResults->setAlternatingRowColors(true);
    searchResults->hide(); // Hide the old widget
    
    // Create detachable search results pane
    searchResultsPane = new DetachablePane("Search Results");
    searchResultsPane->setDefaultSize(QSize(800, 400));
    
    collapsibleSearchResults = new CollapsibleSearchResults();
    collapsibleSearchResults->setObjectName("collapsibleSearchResults");
    searchResultsPane->setContentWidget(collapsibleSearchResults);
    searchResultsLayout->addWidget(searchResultsPane);
    
    // Set original parent information for proper attach/detach (will be set after widgets are created)
    
    // Connect the collapsible search results signal
    connect(collapsibleSearchResults, &CollapsibleSearchResults::resultSelected, 
            this, &MainWindow::onCollapsibleResultSelected);
    connect(collapsibleSearchResults, &CollapsibleSearchResults::parsingProgress, 
            this, &MainWindow::onParsingProgressUpdate);
    connect(collapsibleSearchResults, &CollapsibleSearchResults::parsingCompleted, 
            this, &MainWindow::onParsingCompleted);
    connect(collapsibleSearchResults, &CollapsibleSearchResults::parsingError, 
            this, &MainWindow::onParsingError);
    
    // Connect detachable pane signals
    connect(searchResultsPane, &DetachablePane::paneDetached, 
            this, &MainWindow::onSearchResultsPaneDetached);
    connect(searchResultsPane, &DetachablePane::paneAttached, 
            this, &MainWindow::onSearchResultsPaneAttached);
    connect(searchResultsPane, &DetachablePane::paneClosed, 
            this, &MainWindow::onSearchResultsPaneClosed);
    
    // File Content Viewer (Middle)
    QVBoxLayout *fileContentLayout = new QVBoxLayout();
    
    // File Content Viewer label removed
    
    // Create detachable file viewer pane
    fileViewerPane = new DetachablePane("File Viewer");
    fileViewerPane->setDefaultSize(QSize(1000, 600));
    
    // Create a container widget for file content
    QWidget* fileContentContainer = new QWidget();
    QVBoxLayout* fileContentContainerLayout = new QVBoxLayout(fileContentContainer);
    fileContentContainerLayout->setContentsMargins(0, 0, 0, 0);
    
    // Filename label removed - now shown in pane title for extra space
    
    fileContentView = new ScintillaEdit();
    fileContentContainerLayout->addWidget(fileContentView);
    
    fileViewerPane->setContentWidget(fileContentContainer);
    fileContentLayout->addWidget(fileViewerPane);
    
    // Set original parent information for proper attach/detach (will be set after widgets are created)
    
    // Connect background highlighting signals
    connect(fileContentView, &ScintillaEdit::backgroundHighlightProgress, this, &MainWindow::onBackgroundHighlightProgress);
    connect(fileContentView, &ScintillaEdit::backgroundHighlightCompleted, this, &MainWindow::onBackgroundHighlightCompleted);
    
    // Connect detachable pane signals
    connect(fileViewerPane, &DetachablePane::paneDetached, 
            this, &MainWindow::onFileViewerPaneDetached);
    connect(fileViewerPane, &DetachablePane::paneAttached, 
            this, &MainWindow::onFileViewerPaneAttached);
    connect(fileViewerPane, &DetachablePane::paneClosed, 
            this, &MainWindow::onFileViewerPaneClosed);
    
    // Log Window (Above Status Bar)
    QVBoxLayout *logLayout = new QVBoxLayout();
    logLayout->setContentsMargins(4, 4, 4, 4);  // Small margins
    logLayout->setSpacing(2);  // Minimal spacing
    
    // Log Window label removed as requested
    
    logWidget = new QTextEdit();
    logWidget->setReadOnly(true);
    logWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);  // Allow expansion
    logLayout->addWidget(logWidget, 1);  // Stretch factor of 1 to take remaining space
    
    // Create vertical splitter for resizable panes
    mainSplitter = new QSplitter(Qt::Vertical);
    mainSplitter->setChildrenCollapsible(false);  // Prevent panes from collapsing completely
    
    // Create widgets for each pane with borders and hover tooltips
    QWidget *searchControlsWidget = new QWidget();
    searchControlsWidget->setLayout(searchControlsLayout);
    searchControlsWidget->setStyleSheet("QWidget { border: 1px solid #e5e7eb; border-radius: 8px; background-color: #f9fafb; }");
    searchControlsWidget->setToolTip("🔍 Search Controls\n\nEnter your search pattern and path, then click Search to find files.\nUse Search Params to configure advanced options like case sensitivity, file filters, and highlight colors.");
    
    QWidget *searchResultsWidget = new QWidget();
    searchResultsWidget->setLayout(searchResultsLayout);
    searchResultsWidget->setStyleSheet("QWidget { border: 1px solid #e5e7eb; border-radius: 8px; background-color: #ffffff; }");
    searchResultsWidget->setToolTip("📋 Search Results\n\nDisplays all files and matches found by your search.\nClick on any result to open the file and jump to that line.\nExpand/collapse folders to organize your results.");
    
    QWidget *fileContentWidget = new QWidget();
    fileContentWidget->setLayout(fileContentLayout);
    fileContentWidget->setStyleSheet("QWidget { border: 1px solid #e5e7eb; border-radius: 8px; background-color: #ffffff; }");
    fileContentWidget->setToolTip("📄 File Content Viewer\n\nShows the content of the selected file with syntax highlighting.\nSearch matches are highlighted in color.\nUse the Highlight dialog to add custom highlighting rules.");
    
    QWidget *logWidgetContainer = new QWidget();
    logWidgetContainer->setLayout(logLayout);
    logWidgetContainer->setStyleSheet("QWidget { border: 1px solid #e5e7eb; border-radius: 8px; background-color: #f9fafb; }");
    logWidgetContainer->setToolTip("📝 Log Window\n\nShows detailed information about search operations, file loading, and system activities.\nUse this for debugging and monitoring application performance.\nResize this pane to see more or less log details.");
    
    // Add widgets to splitter in vertical order
    mainSplitter->addWidget(searchControlsWidget);
    mainSplitter->addWidget(searchResultsWidget);
    mainSplitter->addWidget(fileContentWidget);
    mainSplitter->addWidget(logWidgetContainer);
    
    // Set original parent information for proper attach/detach functionality
    searchResultsPane->setOriginalParent(searchResultsWidget, searchResultsLayout);
    fileViewerPane->setOriginalParent(fileContentWidget, fileContentLayout);
    
    // Load saved pane sizes or set defaults
    loadPaneSizes();
    
    mainLayout->addWidget(mainSplitter);
    
    // Connect splitter signal to save sizes when changed
    connect(mainSplitter, &QSplitter::splitterMoved, this, &MainWindow::savePaneSizes);
    
    // Add status bar at the bottom (fixed height)
    QStatusBar *statusBar = this->statusBar();
    statusBar->showMessage("Ready");
    
    // Add cache status indicator to the right side of status bar
    cacheStatusLabel = new QLabel();
    cacheStatusLabel->setStyleSheet("QLabel { color: #6b7280; font-size: 10px; padding: 4px 8px; border: 1px solid #e5e7eb; border-radius: 6px; background-color: #f9fafb; font-family: 'Segoe UI', 'Inter', -apple-system, sans-serif; }");
    cacheStatusLabel->setToolTip("Cache mode indicator");
    statusBar->addPermanentWidget(cacheStatusLabel);
    
    // Add process status lamps
    searchResultsLamp = new QLabel("Search Results");
    searchResultsLamp->setStyleSheet("QLabel { color: #6b7280; font-size: 10px; padding: 4px 8px; border: 1px solid #e5e7eb; border-radius: 6px; background-color: transparent; font-family: 'Segoe UI', 'Inter', -apple-system, sans-serif; }");
    searchResultsLamp->setToolTip("Search Results - RG Search in progress");
    statusBar->addPermanentWidget(searchResultsLamp);
    
    parsingLamp = new QLabel("Parsing");
    parsingLamp->setStyleSheet("QLabel { color: #6b7280; font-size: 10px; padding: 4px 8px; border: 1px solid #e5e7eb; border-radius: 6px; background-color: transparent; font-family: 'Segoe UI', 'Inter', -apple-system, sans-serif; }");
    parsingLamp->setToolTip("Parsing - Processing search results");
    statusBar->addPermanentWidget(parsingLamp);
    
    openFileLamp = new QLabel("Open File");
    openFileLamp->setStyleSheet("QLabel { color: #6b7280; font-size: 10px; padding: 4px 8px; border: 1px solid #e5e7eb; border-radius: 6px; background-color: transparent; font-family: 'Segoe UI', 'Inter', -apple-system, sans-serif; }");
    openFileLamp->setToolTip("Open File - Loading file into ScintillaEdit");
    statusBar->addPermanentWidget(openFileLamp);
    
    scrollHighlightLamp = new QLabel("Scroll & Highlight");
    scrollHighlightLamp->setStyleSheet("QLabel { color: #6b7280; font-size: 10px; padding: 4px 8px; border: 1px solid #e5e7eb; border-radius: 6px; background-color: transparent; font-family: 'Segoe UI', 'Inter', -apple-system, sans-serif; }");
    scrollHighlightLamp->setToolTip("Scroll & Highlight - FastScrollHighlight in progress");
    statusBar->addPermanentWidget(scrollHighlightLamp);
    
    highlightExtraLamp = new QLabel("Highlight Extra");
    highlightExtraLamp->setStyleSheet("QLabel { color: #6b7280; font-size: 10px; padding: 4px 8px; border: 1px solid #e5e7eb; border-radius: 6px; background-color: transparent; font-family: 'Segoe UI', 'Inter', -apple-system, sans-serif; }");
    highlightExtraLamp->setToolTip("Highlight Extra - applyExtraHighlights in progress");
    statusBar->addPermanentWidget(highlightExtraLamp);
    
    cleanLamp = new QLabel("Clean");
    cleanLamp->setStyleSheet("QLabel { color: #6b7280; font-size: 10px; padding: 4px 8px; border: 1px solid #e5e7eb; border-radius: 6px; background-color: transparent; font-family: 'Segoe UI', 'Inter', -apple-system, sans-serif; }");
    cleanLamp->setToolTip("Clean - KCompleteCleanUp in progress");
    statusBar->addPermanentWidget(cleanLamp);
    
    backgroundHighlightLamp = new QLabel("Background Highlighting");
    backgroundHighlightLamp->setStyleSheet("QLabel { color: #6b7280; font-size: 10px; padding: 4px 8px; border: 1px solid #e5e7eb; border-radius: 6px; background-color: transparent; font-family: 'Segoe UI', 'Inter', -apple-system, sans-serif; }");
    backgroundHighlightLamp->setToolTip("Background Highlighting - Systematic file highlighting in progress");
    statusBar->addPermanentWidget(backgroundHighlightLamp);
    
    // Update cache status display
    updateCacheStatusDisplay();
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    
    // File menu
    QMenu *fileMenu = menuBar->addMenu("&File");
    // Open File action removed - moved to Engineering dialog
    fileMenu->addAction("&Save", this, &MainWindow::saveFile, QKeySequence::Save);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", this, &QWidget::close, QKeySequence::Quit);
    
    // Search menu (Search and Clear actions removed - functionality moved to main interface)
    QMenu *searchMenu = menuBar->addMenu("&Search");
    
    // Tools menu
    QMenu *toolsMenu = menuBar->addMenu("&Tools");
    // Preferences action removed - functionality integrated into Search Params
    
    // Help menu
    QMenu *helpMenu = menuBar->addMenu("&Help");
    helpMenu->addAction("&About", [this]() {
        QMessageBox::about(this, "About TotalSearch", 
                          "TotalSearch - Advanced File Search and Viewer\n\n"
                          "A powerful tool for searching and viewing large files efficiently.");
    });
}

void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("Main Toolbar");
    
    // Removed Open File, Search, Clear, Preferences - functionality moved to main interface and dialogs
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "All Files (*.*)");
    if (!fileName.isEmpty()) {
        fileContentView->loadFile(fileName);
        m_currentFilePath = fileName;
        onFileLoaded(fileName);
    }
}

void MainWindow::saveFile()
{
    // Implementation for saving file
    QMessageBox::information(this, "Info", "Save functionality not implemented yet.");
}

void MainWindow::searchText()
{
    // Use the new search method with main window fields
    LOG_INFO("searchText: Using KSearch->KSsearchDo");
    m_kSearch->KSsearchDo();
}

void MainWindow::stopSearch()
{
    QThread::create([this]() {
        m_kSearch->stopSearch();
    })->start();
}

void MainWindow::clearSearch()
{
    searchResults->clear();
    logWidget->append("Search results cleared.");
}

void MainWindow::updateScrollPosition(int value)
{
    // Implementation for scroll position update
}

void MainWindow::showPreferences()
{
    PreferencesDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Save settings from dialog
        saveSettings(dialog.searchResultsPosition(), dialog.showLogWindow(),
                    dialog.searchEngine(), dialog.caseSensitive(), dialog.useRegex(),
                    dialog.wholeWord(), dialog.ignoreHidden(), dialog.followSymlinks(),
                    dialog.fileTypes(), dialog.maxDepth(), dialog.excludePatterns(),
                    dialog.smartCase(), dialog.multiline(), dialog.dotAll(), dialog.noIgnore(),
                    dialog.tabWidth(), dialog.useSpacesForTabs(), dialog.contextLines(),
                    dialog.highlightColor(), dialog.chunkSize(), dialog.firstChunkSize(), dialog.showLineNumbers());
        
        // Reload settings and apply them
        loadSettings();
        applyLayoutSettings();
        applyViewerSettings();
        
        // Apply font if changed
        if (dialog.selectedFont() != QApplication::font()) {
            applyFont(dialog.selectedFont());
        }
    }
}

void MainWindow::showSearchParameters()
{
    // Implementation for search parameters
}

void MainWindow::loadSettings()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = appDir + "/App.ini";
    QSettings settings(configFile, QSettings::IniFormat);
    settings.beginGroup("Configuration");
    
    // Load search parameters
    m_searchEngine = settings.value("searchEngine", "ripgrep").toString();
    m_caseSensitive = settings.value("caseSensitive", false).toBool();
    m_useRegex = settings.value("useRegex", false).toBool();
    m_wholeWord = settings.value("wholeWord", false).toBool();
    m_ignoreHidden = settings.value("ignoreHidden", true).toBool();
    m_followSymlinks = settings.value("followSymlinks", false).toBool();
    m_fileTypes = settings.value("fileTypes", "*.txt,*.log,*.cpp,*.h").toString();
    m_maxDepth = settings.value("maxDepth", 10).toInt();
    m_excludePatterns = settings.value("excludePatterns", "").toString();
    m_smartCase = settings.value("smartCase", false).toBool();
    m_multiline = settings.value("multiline", false).toBool();
    m_dotAll = settings.value("dotAll", false).toBool();
    m_noIgnore = settings.value("noIgnore", false).toBool();
    
    // Load layout settings
    m_searchResultsPosition = settings.value("searchResultsPosition", "left").toString();
    m_showLogWindow = settings.value("showLogWindow", true).toBool();
    m_isUpDownLayout = settings.value("UpDownLayout", true).toBool(); // Load layout preference
    LOG_INFO("MainWindow: Loaded UpDownLayout setting: " + QString(m_isUpDownLayout ? "true" : "false"));
    
    // Load viewer settings
    m_contextLines = settings.value("contextLines", 5).toInt();
    m_tabWidth = settings.value("tabWidth", 4).toInt();
    m_useSpacesForTabs = settings.value("useSpacesForTabs", true).toBool();
    m_highlightColor = settings.value("highlightColor", QColor(Qt::yellow)).value<QColor>();
    
    // Load KLOGG-style chunk sizes (convert from bytes to MB)
    int chunkSizeBytes = settings.value("chunkSize", 5 * 1024 * 1024).toInt(); // Default 5MB like KLOGG
    m_chunkSize = chunkSizeBytes / (1024 * 1024); // Convert to MB
    
    int firstChunkSizeBytes = settings.value("firstChunkSize", 16 * 1024 * 1024).toInt(); // Default 16MB like KLOGG
    m_firstChunkSize = firstChunkSizeBytes / (1024 * 1024); // Convert to MB
    
    // Load line numbers setting
    m_showLineNumbers = settings.value("showLineNumbers", true).toBool(); // Default show line numbers
    
    // Load Find in File settings
    m_findInFilePattern = settings.value("findInFilePattern", "").toString();
    m_findInFileEngine = settings.value("findInFileEngine", "Hyperscan").toString();
    m_findInFileType = settings.value("findInFileType", "ExtendedRegex").toString();
    m_findInFileCaseSensitive = settings.value("findInFileCaseSensitive", true).toBool();
    m_findInFileInverse = settings.value("findInFileInverse", false).toBool();
    m_findInFileBoolean = settings.value("findInFileBoolean", false).toBool();
    m_findInFilePlainText = settings.value("findInFilePlainText", false).toBool();
    m_findInFileAutoRefresh = settings.value("findInFileAutoRefresh", true).toBool();
    m_findInFileStartLine = settings.value("findInFileStartLine", 1).toInt();
    m_findInFileEndLine = settings.value("findInFileEndLine", 999999999).toInt();
    m_findInFileHighlightColor = settings.value("findInFileHighlightColor", QColor(Qt::yellow)).value<QColor>();
    
    settings.endGroup();
    
    // Load cache setting from App.ini (in RGSearch section to match dialog)
    QSettings appSettings(configFile, QSettings::IniFormat);
    appSettings.beginGroup("RGSearch");
    m_keepFilesInCache = appSettings.value("LastKeepFilesInCache", false).toBool();
    appSettings.endGroup();
    LOG_INFO("loadSettings: Cache setting loaded: " + QString(m_keepFilesInCache ? "ENABLED" : "DISABLED"));
    
    // Load search and folder history
    settings.beginGroup("History");
    QStringList searchHistory = settings.value("searchHistory").toStringList();
    QStringList folderHistory = settings.value("folderHistory").toStringList();
    settings.endGroup();
    
    // Update auto-completers with history
    if (!searchHistory.isEmpty()) {
        m_patternModel->setStringList(searchHistory);
        LOG_INFO("MainWindow: Loaded search history from file - " + QString::number(searchHistory.size()) + " patterns, " + QString::number(folderHistory.size()) + " paths");
    }
    
    updateCacheStatusDisplay();
}

void MainWindow::saveToSearchHistory(const QString &searchTerm)
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = appDir + "/App.ini";
    QSettings settings(configFile, QSettings::IniFormat);
    settings.beginGroup("History");
    QStringList history = settings.value("searchHistory").toStringList();
    history.removeAll(searchTerm);
    history.prepend(searchTerm);
    if (history.size() > 20) {
        history = history.mid(0, 20);
    }
    settings.setValue("searchHistory", history);
    settings.endGroup();
}

void MainWindow::saveToFolderHistory(const QString &folderPath)
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = appDir + "/App.ini";
    QSettings settings(configFile, QSettings::IniFormat);
    settings.beginGroup("History");
    QStringList history = settings.value("folderHistory").toStringList();
    history.removeAll(folderPath);
    history.prepend(folderPath);
    if (history.size() > 10) {
        history = history.mid(0, 10);
    }
    settings.setValue("folderHistory", history);
    settings.endGroup();
}

QStringList MainWindow::getSearchHistory()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = appDir + "/App.ini";
    QSettings settings(configFile, QSettings::IniFormat);
    settings.beginGroup("History");
    QStringList history = settings.value("searchHistory").toStringList();
    settings.endGroup();
    return history;
}

QStringList MainWindow::getFolderHistory()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = appDir + "/App.ini";
    QSettings settings(configFile, QSettings::IniFormat);
    settings.beginGroup("History");
    QStringList history = settings.value("folderHistory").toStringList();
    settings.endGroup();
    return history;
}

void MainWindow::saveSettings(const QString &searchResultsPosition, bool showLogWindow, 
                             const QString &searchEngine, bool caseSensitive, bool useRegex, 
                             bool wholeWord, bool ignoreHidden, bool followSymlinks, 
                             const QString &fileTypes, int maxDepth, const QString &excludePatterns, 
                             bool smartCase, bool multiline, bool dotAll, bool noIgnore, 
                             int tabWidth, bool useSpacesForTabs, int contextLines, 
                             QColor highlightColor, int chunkSize, int firstChunkSize, bool showLineNumbers)
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = appDir + "/App.ini";
    QSettings settings(configFile, QSettings::IniFormat);
    settings.beginGroup("Configuration");
    
    // Save search parameters
    settings.setValue("searchEngine", searchEngine);
    settings.setValue("caseSensitive", caseSensitive);
    settings.setValue("useRegex", useRegex);
    settings.setValue("wholeWord", wholeWord);
    settings.setValue("ignoreHidden", ignoreHidden);
    settings.setValue("followSymlinks", followSymlinks);
    settings.setValue("fileTypes", fileTypes);
    settings.setValue("maxDepth", maxDepth);
    settings.setValue("excludePatterns", excludePatterns);
    settings.setValue("smartCase", smartCase);
    settings.setValue("multiline", multiline);
    settings.setValue("dotAll", dotAll);
    settings.setValue("noIgnore", noIgnore);
    
    // Save layout settings
    settings.setValue("searchResultsPosition", searchResultsPosition);
    settings.setValue("showLogWindow", showLogWindow);
    
    // Save viewer settings
    settings.setValue("contextLines", contextLines);
    settings.setValue("tabWidth", tabWidth);
    settings.setValue("useSpacesForTabs", useSpacesForTabs);
    settings.setValue("highlightColor", highlightColor);
    
    // Convert MB to bytes for KLOGG-style chunk sizes
    int chunkSizeBytes = chunkSize * (1024 * 1024);
    int firstChunkSizeBytes = firstChunkSize * (1024 * 1024);
    settings.setValue("chunkSize", chunkSizeBytes);
    settings.setValue("firstChunkSize", firstChunkSizeBytes);
    
    // Save line numbers setting
    settings.setValue("showLineNumbers", showLineNumbers);
    settings.endGroup();
}

void MainWindow::saveFindInFileSettings(const QString &pattern, const QString &engine, const QString &type, bool caseSensitive,
                                       bool inverse, bool boolean, bool plainText, bool autoRefresh,
                                       int startLine, int endLine, QColor highlightColor)
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = appDir + "/App.ini";
    QSettings settings(configFile, QSettings::IniFormat);
    settings.beginGroup("FindInFile");
    
    settings.setValue("findInFilePattern", pattern);
    settings.setValue("findInFileEngine", engine);
    settings.setValue("findInFileType", type);
    settings.setValue("findInFileCaseSensitive", caseSensitive);
    settings.setValue("findInFileInverse", inverse);
    settings.setValue("findInFileBoolean", boolean);
    settings.setValue("findInFilePlainText", plainText);
    settings.setValue("findInFileAutoRefresh", autoRefresh);
    settings.setValue("findInFileStartLine", startLine);
    settings.setValue("findInFileEndLine", endLine);
    settings.setValue("findInFileHighlightColor", highlightColor);
    settings.endGroup();
}

void MainWindow::applyFont(const QFont &font)
{
    // Font setting handled by Scintilla internally
    searchResults->setFont(font);
    logWidget->setFont(font);
}

void MainWindow::applyLayoutSettings()
{
    // Apply layout-specific settings
    // Note: Layout is now fixed with search controls and results side by side,
    // file viewer in the middle, and log at the bottom
    
    // Show/hide log window based on setting
    logWidget->setVisible(m_showLogWindow);
}

void MainWindow::applyViewerSettings()
{
    // Apply viewer-specific settings
    fileContentView->setTabWidth(m_tabWidth);
    // fileContentView->setUseSpacesForTabs(m_useSpacesForTabs); // Not implemented
    fileContentView->setHighlightColor(m_highlightColor);
    
    // Convert MB to bytes for KLOGG-style chunk sizes
    int chunkSizeBytes = m_chunkSize * (1024 * 1024);
    int firstChunkSizeBytes = m_firstChunkSize * (1024 * 1024);
    
    fileContentView->setChunkSize(chunkSizeBytes);
    fileContentView->setFirstChunkSize(firstChunkSizeBytes);
}

void MainWindow::performRipgrepSearch(const QString &searchTerm, const QString &startTime)
{
    logFunctionStart("performRipgrepSearch");
    QProcess *process = new QProcess(this);
    
    QStringList arguments;
    arguments << "--json";
    arguments << "-a";                    // Search binary files
    arguments << "--threads" << "0";      // Use all available threads
    arguments << "--mmap";                // Use memory-mapped I/O
    arguments << "--no-ignore-dot";       // Don't ignore .gitignore files
    arguments << "--no-config";           // Don't use ripgrep config files
    arguments << "--line-number";
    arguments << "--column";
    arguments << "--with-filename";
    arguments << "--no-heading";
    arguments << "--no-messages";
    
    // Apply search parameters
    if (m_caseSensitive) {
        arguments << "--case-sensitive";
    }
    
    if (m_useRegex) {
        // ripgrep uses regex by default, but we can add --fixed-strings to disable it
        // Since we want regex when m_useRegex is true, we don't add --fixed-strings
    } else {
        arguments << "-F";                // Fixed strings mode
    }
    
    if (m_wholeWord) {
        arguments << "--word-regexp";
    }
    
    if (m_ignoreHidden) {
        arguments << "--hidden";
    }
    
    if (m_followSymlinks) {
        arguments << "--follow";
    }
    
    if (m_maxDepth > 0) {
        arguments << "--max-depth" << QString::number(m_maxDepth);
    }
    
    if (m_smartCase) {
        arguments << "--smart-case";
    }
    
    if (m_multiline) {
        arguments << "--multiline";
    }
    
    if (m_dotAll) {
        arguments << "--multiline-dotall";
    }
    
    if (m_noIgnore) {
        arguments << "--no-ignore";
    }
    
    // Add file type filters using -g (glob) instead of --type
    if (!m_fileTypes.isEmpty()) {
        QStringList fileTypes = m_fileTypes.split(',', Qt::SkipEmptyParts);
        for (const QString &type : fileTypes) {
            QString trimmedType = type.trimmed();
            if (!trimmedType.isEmpty()) {
                arguments << "-g" << trimmedType;
            }
        }
    }
    
    // Add exclude patterns using -g with ! prefix
    if (!m_excludePatterns.isEmpty()) {
        QStringList excludePatterns = m_excludePatterns.split(',', Qt::SkipEmptyParts);
        for (const QString &pattern : excludePatterns) {
            QString trimmedPattern = pattern.trimmed();
            if (!trimmedPattern.isEmpty()) {
                arguments << "-g" << QString("!%1").arg(trimmedPattern);
            }
        }
    }
    
    arguments << searchTerm;
    arguments << selectedFolderPath;
    
    // Log the complete ripgrep command
    QString fullCommand = "rg " + arguments.join(" ");
    logWidget->append(QString("Executing ripgrep command: %1").arg(fullCommand));
    
    connect(process, &QProcess::finished, [this, process, startTime](int exitCode, QProcess::ExitStatus exitStatus) {
        QString output = QString::fromUtf8(process->readAllStandardOutput());
        QString error = QString::fromUtf8(process->readAllStandardError());
        
        if (!error.isEmpty()) {
            logWidget->append(QString("Ripgrep error: %1").arg(error));
        }
        
        int resultCount = parseRipgrepResults(output);
        logWidget->append(QString("Search completed at %1. Found %2 results.").arg(startTime).arg(resultCount));
        
        process->deleteLater();
    });
    
    process->start("rg", arguments);
    logFunctionEnd("performRipgrepSearch");
}

void MainWindow::performBuiltinSearch(const QString &searchTerm, const QString &startTime)
{
    // Implementation for built-in search (if needed)
    logWidget->append("Built-in search not implemented yet.");
}

void MainWindow::logDebugMessage(const QString &message)
{
    logWidget->append(QString("[DEBUG] %1").arg(message));
}



// ==================================================================================
// <<<<<<<<<<<<<<<<<<<<   on return from threads    <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// ==================================================================================


void MainWindow::onSearchResultSelected(QListWidgetItem *item)
{
    logFunctionStart("onSearchResultSelected");
    if (!item) return;
    
    QString itemText = item->text();
    QString filePath = item->data(Qt::UserRole).toString();
    
    // Extract line number and text from "  Line 42: content"
    if (itemText.startsWith("  Line ")) {
        QString lineNumberStr = itemText.mid(7);
        int colonPos = lineNumberStr.indexOf(':');
        if (colonPos > 0) {
            bool ok;
            int lineNumber = lineNumberStr.left(colonPos).toInt(&ok);
            if (ok && lineNumber > 0) {
                QString lineText = lineNumberStr.mid(colonPos + 1).trimmed();
         //       KUpdateFileViewer(filePath, lineNumber, lineText);
                KUpdateFileViewer2(filePath, lineNumber);
            }
        }
    }
    logFunctionEnd("onSearchResultSelected");
}






void MainWindow::onFileLoaded(const QString &filePath)
{
    logWidget->append(QString("File loaded: %1").arg(filePath));
}

void MainWindow::onScintillaFileLoadError(const QString &error)
{
    logWidget->append(QString("File load error: %1").arg(error));
}

void MainWindow::onFileLineNumberChanged(int fileLine)
{
    // Update status bar or other UI elements
}

void MainWindow::onLoadingProgress(int chunksLoaded, int totalChunks)
{
   // logWidget->append(QString("Loading progress: %1/%2 chunks").arg(chunksLoaded).arg(totalChunks));
}




// ===== BACKGROUND HIGHLIGHTING SLOTS =====

void MainWindow::onBackgroundHighlightProgress(int percentage)
{
    // Update the progress lamp with current percentage
    setBackgroundHighlightLamp(true, percentage);
    
    LOG_INFO("MainWindow: Background highlighting progress: " + QString::number(percentage) + "%");
}

void MainWindow::onBackgroundHighlightCompleted()
{
    // Keep the progress lamp blue at 100% instead of turning off
    setBackgroundHighlightLamp(true, 100);
    
    LOG_INFO("MainWindow: Background highlighting completed - file is fully highlighted");
    
    // Optional: Show completion message in status bar
    statusBar()->showMessage("Background highlighting completed", 3000);
}

void MainWindow::onParsingProgressUpdate(int percentage, int files)
{
    // Update status file with parsing progress
    writeStatusFile("PARSING", percentage, QString("Processing %1% in %2 files").arg(percentage).arg(files));
    
    // Update progress bar with percentage
    updateParsingProgress(percentage);
    
    // Update parsing lamp with percentage
    setParsingLamp(true, percentage);
    
    LOG_INFO("MainWindow: Parsing progress update - " + QString::number(percentage) + "% in " + QString::number(files) + " files");
}

void MainWindow::onParsingCompleted(int totalMatches, int totalFiles)
{
    LOG_INFO("MainWindow: onParsingCompleted called - UI state handled centrally");
    
    // Check if search was stopped during parsing
    if (m_currentState == SearchState::STOP) {


 
    





        LOG_INFO("MainWindow: Search was stopped, not completing parsing state transition");
        m_kSearch->updateSearchState(SearchState::IDLE);
        //    hideSearchProgress();
        return;
    }
    
    // Set final state to IDLE and update UI
    LOG_INFO("MainWindow: Setting final state to IDLE");
    m_kSearch->updateSearchState(SearchState::IDLE);
    
    // Add total search time to header
    if (collapsibleSearchResults) {
        qint64 totalSearchTime = m_kSearch->getTotalSearchTime();
        // Update pane title with search time
        collapsibleSearchResults->updatePaneTitleWithSearchTime(totalSearchTime);
        LOG_INFO("MainWindow: Total search time added to pane title: " + QString::number(totalSearchTime) + " ms");
    }
    
   // Hide progress bar and set to 100%
  //  hideSearchProgress();
 //   updateParsingProgress(100);
    
    
    // Update status file
    writeStatusFile("IDLE", 0, QString("Search completed - %1 matches in %2 files").arg(totalMatches).arg(totalFiles));
    LOG_INFO("MainWindow: Status file updated");
    
    LOG_INFO("MainWindow: Parsing completed - " + QString::number(totalMatches) + " matches in " + QString::number(totalFiles) + " files");
}

void MainWindow::onParsingError(const QString &error)
{
    LOG_INFO("MainWindow: onParsingError called - UI state handled centrally");
    
    // Hide progress bar on error
    hideSearchProgress();
    
    // Parsing lamp and button state are now handled centrally in performSearchWithCurrentParams
    LOG_INFO("MainWindow: Parsing error - UI state handled centrally");
    
    // Skip timer operations since we disabled timer creation
    LOG_INFO("MainWindow: Skipping timer operations (timer creation was disabled)");
    
    // Update status file
    LOG_INFO("MainWindow: About to update status file");
    writeStatusFile("ERROR", 0, QString("Parsing error: %1").arg(error));
    LOG_INFO("MainWindow: Status file updated");
    
    LOG_ERROR("MainWindow: Parsing error - " + error);
}




// ==================================================================================
// >>>>>>>>>>>>>>>>> END  on return from threads  END >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// ==================================================================================











int MainWindow::parseRipgrepResults(const QString &jsonOutput)
{
    int resultCount = 0;
    QStringList lines = jsonOutput.split('\n', Qt::SkipEmptyParts);
    
    logWidget->append(QString("Parsing %1 JSON lines").arg(lines.size()));
    
    for (const QString &line : lines) {
        if (line.trimmed().isEmpty()) continue;
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError) {
            logWidget->append(QString("Error parsing JSON line: %1 - Line: %2").arg(error.errorString()).arg(line.left(100)));
            continue;
        }
        
        QJsonObject obj = doc.object();
        QString messageType = obj["type"].toString();
        
        // Log the message type for debugging
        if (resultCount < 5) { // Only log first few for debugging
            logWidget->append(QString("JSON message type: %1").arg(messageType));
        }
        
        if (messageType == "match") {
            QJsonObject data = obj["data"].toObject();
            
            // Handle different JSON structures
            QString filePath;
            int lineNumber = 0;
            QString lineText;
            
            // Try different path structures
            if (data.contains("path")) {
                QJsonValue pathValue = data["path"];
                if (pathValue.isObject()) {
                    QJsonObject path = pathValue.toObject();
                    filePath = path["text"].toString();
                } else if (pathValue.isString()) {
                    filePath = pathValue.toString();
                }
            }
            
            // Get line number
            if (data.contains("line_number")) {
                lineNumber = data["line_number"].toInt();
            }
            
            // Get line text
            if (data.contains("lines")) {
                QJsonValue linesValue = data["lines"];
                if (linesValue.isObject()) {
                    QJsonObject linesObj = linesValue.toObject();
                    lineText = linesObj["text"].toString();
                } else if (linesValue.isString()) {
                    lineText = linesValue.toString();
                }
            }
            
            // If we still don't have the text, try alternative structure
            if (lineText.isEmpty() && data.contains("text")) {
                lineText = data["text"].toString();
            }
            
            if (!filePath.isEmpty() && lineNumber > 0) {
                QString displayText = QString("%1:%2: %3").arg(filePath).arg(lineNumber).arg(lineText.trimmed());
                
                QListWidgetItem *item = new QListWidgetItem(displayText);
                item->setData(Qt::UserRole, line);
                searchResults->addItem(item);
                resultCount++;
                
                // Log first few results for debugging
                if (resultCount <= 3) {
                    logWidget->append(QString("Added result %1: %2").arg(resultCount).arg(displayText.left(80)));
                }
            } else {
                logWidget->append(QString("Skipping match - missing data: filePath='%1', lineNumber=%2, lineText='%3'")
                                .arg(filePath).arg(lineNumber).arg(lineText.left(50)));
            }
        } else if (messageType == "begin" || messageType == "end" || messageType == "summary") {
            // These are informational messages, not results
            if (resultCount < 5) { // Only log first few for debugging
                logWidget->append(QString("Info message: %1").arg(messageType));
            }
        } else {
            // Unknown message type
            if (resultCount < 5) { // Only log first few for debugging
                logWidget->append(QString("Unknown message type: %1").arg(messageType));
            }
        }
    }
    
    logWidget->append(QString("Parsing complete. Found %1 results.").arg(resultCount));
    return resultCount;
}

QString MainWindow::extractJsonValue(const QString &jsonLine, const QString &key, const QString &subKey)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonLine.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        return QString();
    }
    
    QJsonObject obj = doc.object();
    if (subKey.isEmpty()) {
        return obj[key].toString();
    } else {
        return obj[key].toObject()[subKey].toString();
    }
}

QString MainWindow::decodeJsonString(const QString &jsonString)
{
    // Simple JSON string decoding
    QString result = jsonString;
    result.replace("\\n", "\n");
    result.replace("\\t", "\t");
    result.replace("\\\"", "\"");
    result.replace("\\\\", "\\");
    return result;
} 

void MainWindow::openFile2()
{
    logFunctionStart("openFile2");
    LOG_DEBUG("openFile2: entered");
    if (!logWidget) {
        LOG_ERROR("openFile2: logWidget is null!");
        return;
    }
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    logWidget->append(QString("[%1] === OPEN FILE2 START ===").arg(timestamp));
    LOG_DEBUG("openFile2: started");

    // Clean up any previous state
    logWidget->append(QString("[%1] Cleaning up previous state...").arg(timestamp));
    KClean();

    setOpenFileLamp(true);
    QThread::msleep(2000);
    setOpenFileLamp(false);
    LOG_DEBUG("AMIRRRRRRRRRRRRRRRRRRRRRR TESTTTTTTTTTTTTTTT");

    // QFileDialog for file selection
    logWidget->append(QString("[%1] Opening file dialog...").arg(timestamp));
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "All Files (*.*)");
    
    if (fileName.isEmpty()) {
        logWidget->append(QString("[%1] No file selected").arg(timestamp));
        LOG_DEBUG("openFile2: no file selected");
        return;
    }
    
    logWidget->append(QString("[%1] File selected: %2").arg(timestamp, fileName));
    LOG_DEBUG("openFile2: file selected - " + fileName);
    
    // Use the new K-functions (non-blocking)
    logWidget->append(QString("[%1] Calling KOpenFile (non-blocking)...").arg(timestamp));
    bool openSuccess = KOpenFile(fileName);
    
    if (openSuccess) {
        logWidget->append(QString("[%1] File indexing started in background").arg(timestamp));
        logWidget->append(QString("[%1] UI remains responsive during indexing").arg(timestamp));
        logWidget->append(QString("[%1] KDisplayFile will be called automatically when indexing completes").arg(timestamp));
    } else {
        logWidget->append(QString("[%1] ERROR: Failed to start file indexing").arg(timestamp));
    }
    
    LOG_DEBUG("openFile2: completed");
    logFunctionEnd("openFile2");
}

// New K-functions implementation
bool MainWindow::KOpenFile(const QString& fileName)
{
    logFunctionStart("KOpenFile");
    LOG_DEBUG("KOpenFile: started - " + fileName);
    if (!logWidget) {
        LOG_ERROR("KOpenFile: logWidget is null!");
        logFunctionEnd("KOpenFile");
        return false;
    }
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    logWidget->append(QString("[%1] === KOPENFILE START ===").arg(timestamp));
    
    // Create new LogDataWorker (KClean ensures clean state)
    logWidget->append(QString("[%1] Creating LogDataWorker...").arg(timestamp));
    m_logDataWorker = new LogDataWorker(this);
    LOG_DEBUG("KOpenFile: LogDataWorker created");
    
    LOG_DEBUG("KOpenFile: LogDataWorker connected");
    
    // Connect progress messages to log widget
    connect(m_logDataWorker, &LogDataWorker::progressMessage, [this](const QString& message) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        logWidget->append(QString("[%1] %2").arg(timestamp, message));
    });
    
    // Connect indexing finished to show completion and trigger display
    connect(m_logDataWorker, &LogDataWorker::indexingFinished, [this](bool success) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        if (success) {
            logWidget->append(QString("[%1] File loading completed successfully!").arg(timestamp));
            logWidget->append(QString("[%1] Auto-triggering KDisplayFile...").arg(timestamp));
            KDisplayFile(); // Automatically display when indexing is complete
        } else {
            logWidget->append(QString("[%1] ERROR: File loading failed!").arg(timestamp));
        }
    });
    
    // Start indexing the file (NON-BLOCKING)
    logWidget->append(QString("[%1] Starting file indexing (non-blocking)...").arg(timestamp));
    m_logDataWorker->startIndexing(fileName);
    LOG_DEBUG("KOpenFile: indexing started (non-blocking)");
    
    // Update filename display
    updateFilenameDisplay(fileName);
    
    logWidget->append(QString("[%1] KOpenFile completed - indexing in background").arg(timestamp));
    LOG_DEBUG("KOpenFile: completed - indexing in background");
    
    logFunctionEnd("KOpenFile");
    return true; // Return immediately, don't wait for completion
}

void MainWindow::KDisplayFile()
{
    logFunctionStart("KDisplayFile");
    LOG_DEBUG("KDisplayFile: started");
    if (!logWidget) {
        LOG_ERROR("KDisplayFile: logWidget is null!");
        logFunctionEnd("KDisplayFile");
        return;
    }
        
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    logWidget->append(QString("[%1] === KDISPLAYFILE START ===").arg(timestamp));
    
    // Display file content in ScintillaEdit
    logWidget->append(QString("[%1] Displaying file in ScintillaEdit...").arg(timestamp));
    fileContentView->show();
    LOG_DEBUG("KDisplayFile: view switched to ScintillaEdit");
    
    // Update the viewport to display the file
    if (m_logDataWorker && m_logDataWorker->isFileLoaded()) {
        logWidget->append(QString("[%1] File loaded successfully").arg(timestamp));
        LOG_DEBUG("KDisplayFile: file loaded successfully");
        
        // Use the new optimized bulk loading method
        logWidget->append(QString("[%1] Loading file content using bulk method...").arg(timestamp));
        QElapsedTimer timer;
        timer.start();
        
        QString fileContent = m_logDataWorker->loadEntireFileContent();
        
        qint64 loadTime = timer.elapsed();
        
        if (!fileContent.isEmpty()) {
            logWidget->append(QString("[%1] Bulk load completed in %2ms").arg(timestamp, QString::number(loadTime)));
            
            // Clear loading message and set file content
            fileContentView->setStyleSheet(""); // Reset styling
        fileContentView->setText(fileContent);
            qint64 totalTime = timer.elapsed();
            
            logWidget->append(QString("[%1] Content set in ScintillaEdit in %2ms").arg(timestamp, QString::number(totalTime - loadTime)));
            logWidget->append(QString("[%1] Total display time: %2ms").arg(timestamp, QString::number(totalTime)));
        logWidget->append(QString("[%1] File content displayed successfully").arg(timestamp));
            
            LOG_INFO("KDisplayFile: Bulk load completed in " + QString::number(loadTime) + "ms");
            LOG_INFO("KDisplayFile: Content set in ScintillaEdit in " + QString::number(totalTime - loadTime) + "ms");
            LOG_INFO("KDisplayFile: Total display time: " + QString::number(totalTime) + "ms");
        LOG_DEBUG("KDisplayFile: file content displayed successfully");
        } else {
            logWidget->append(QString("[%1] ERROR: Failed to load file content").arg(timestamp));
            LOG_ERROR("KDisplayFile: Failed to load file content");
        }
        
        } else {
        logWidget->append(QString("[%1] ERROR: Cannot display file - worker not ready").arg(timestamp));
        LOG_ERROR("KDisplayFile: Cannot display file - worker not ready");
        logFunctionEnd("KDisplayFile");
    }
    
    logWidget->append(QString("[%1] KDisplayFile completed").arg(timestamp));
    LOG_DEBUG("KDisplayFile: completed");
    logFunctionEnd("KDisplayFile");
}

void MainWindow::KClean()
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    LOG_DEBUG("KClean: started");
    logWidget->append(QString("[%1] === KCLEAN START ===").arg(timestamp));
    
    // Clean up LogDataWorker (interrupt any ongoing indexing)
    if (m_logDataWorker) {
        logWidget->append(QString("[%1] Interrupting LogDataWorker...").arg(timestamp));
        m_logDataWorker->interrupt(); // Stop any ongoing indexing
        LOG_DEBUG("KClean: LogDataWorker interrupted");
    }
    

    
    // Clean up Scintilla view
    if (fileContentView) {
        logWidget->append(QString("[%1] Cleaning Scintilla view...").arg(timestamp));
        fileContentView->clearText(); // Use the correct method
        LOG_DEBUG("KClean: Scintilla view cleared");
    }
    
    // Reset view visibility
        if (fileContentView) {
            fileContentView->hide();
        logWidget->append(QString("[%1] Views hidden").arg(timestamp));
    }
    
    logWidget->append(QString("[%1] === KCLEAN COMPLETED ===").arg(timestamp));
    LOG_DEBUG("KClean: completed");
}

void MainWindow::KScrollToLine(int lineNumber)
{
    logFunctionStart("KScrollToLine");
    qDebug() << "KScrollToLine: function called with lineNumber =" << lineNumber;
    LOG_DEBUG("KScrollToLine: started");
    if (!logWidget) {
        LOG_ERROR("KScrollToLine: logWidget is null!");
        logFunctionEnd("KScrollToLine");
        return;
    }
        
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    logWidget->append(QString("[%1] === KSCROLLTOLINE START ===").arg(timestamp));
    logWidget->append(QString("[%1] Input lineNumber: %2").arg(timestamp, QString::number(lineNumber)));
    
    // SECTION 1: CHECK FILE LOADED
    logWidget->append(QString("[%1] === SECTION 1: CHECK FILE LOADED ===").arg(timestamp));
    LOG_DEBUG("KScrollToLine: Section 1 - Check file loaded");
        
    QString filePath;
    int totalLines = 0;
    
    // Check if we have a LogDataWorker (traditional approach) or direct file content (fast approach)
    if (m_logDataWorker && m_logDataWorker->isFileLoaded()) {
        // Traditional LogDataWorker approach
        logWidget->append(QString("[%1] Using LogDataWorker for file info").arg(timestamp));
        LOG_DEBUG("KScrollToLine: Using LogDataWorker for file info");
        
        filePath = m_logDataWorker->getFilePath();
        totalLines = m_logDataWorker->getTotalLines();
        
    } else if (!m_currentFilePath.isEmpty() && fileContentView) {
        // Fast memory-mapped approach - get line count from ScintillaEdit content
        logWidget->append(QString("[%1] Using fast memory-mapped file info").arg(timestamp));
        LOG_DEBUG("KScrollToLine: Using fast memory-mapped file info");
        
        filePath = m_currentFilePath;
        totalLines = fileContentView->lineCount();
        
        if (totalLines <= 0) {
            logWidget->append(QString("[%1] ERROR: Cannot determine line count from ScintillaEdit!").arg(timestamp));
            LOG_ERROR("KScrollToLine: Cannot determine line count from ScintillaEdit");
        logFunctionEnd("KScrollToLine");
        return;
    }
    
    } else {
        logWidget->append(QString("[%1] ERROR: No file loaded (neither LogDataWorker nor fast approach)!").arg(timestamp));
        LOG_ERROR("KScrollToLine: No file loaded (neither LogDataWorker nor fast approach)");
        logFunctionEnd("KScrollToLine");
        return;
    }
    
    logWidget->append(QString("[%1] File is loaded successfully").arg(timestamp));
    LOG_DEBUG("KScrollToLine: File is loaded successfully");
    
    // SECTION 2: GET FILE INFO
    logWidget->append(QString("[%1] === SECTION 2: GET FILE INFO ===").arg(timestamp));
    LOG_DEBUG("KScrollToLine: Section 2 - Get file info");
    
    logWidget->append(QString("[%1] File path: %2").arg(timestamp, filePath));
    logWidget->append(QString("[%1] Total lines in file: %2").arg(timestamp, QString::number(totalLines)));
    LOG_DEBUG("KScrollToLine: File path: " + filePath + ", Total lines: " + QString::number(totalLines));
    
    // SECTION 3: GET LINE NUMBER
    logWidget->append(QString("[%1] === SECTION 3: GET LINE NUMBER ===").arg(timestamp));
    LOG_DEBUG("KScrollToLine: Section 3 - Get line number");
    
    // If no line number provided, show dialog to get it
    if (lineNumber == -1) {
        qDebug() << "About to show QInputDialog for line number input";
        logWidget->append(QString("[%1] Showing line number input dialog...").arg(timestamp));
        LOG_DEBUG("KScrollToLine: Showing line number input dialog");
        
        bool ok;
        lineNumber = QInputDialog::getInt(this, "Scroll to Line", 
                                        QString("Enter line number (1-%1):").arg(totalLines),
                                        1, 1, totalLines, 1, &ok);
        
        qDebug() << "QInputDialog result: ok =" << ok << "lineNumber =" << lineNumber;
        logWidget->append(QString("[%1] QInputDialog result: ok=%2, lineNumber=%3")
                         .arg(timestamp, ok ? "true" : "false", QString::number(lineNumber)));
        
        if (!ok) {
            logWidget->append(QString("[%1] User cancelled line input").arg(timestamp));
            LOG_DEBUG("KScrollToLine: user cancelled");
            logFunctionEnd("KScrollToLine");
            return;
        }
        
        logWidget->append(QString("[%1] User entered line number: %2").arg(timestamp, QString::number(lineNumber)));
        LOG_DEBUG("KScrollToLine: User entered line number: " + QString::number(lineNumber));
    }
    
    // SECTION 4: VALIDATE LINE NUMBER
    logWidget->append(QString("[%1] === SECTION 4: VALIDATE LINE NUMBER ===").arg(timestamp));
    LOG_DEBUG("KScrollToLine: Section 4 - Validate line number");
    
    logWidget->append(QString("[%1] Validating line number: %2 (range: 1-%3)")
                     .arg(timestamp, QString::number(lineNumber), QString::number(totalLines)));
    
    if (lineNumber < 1 || lineNumber > totalLines) {
        logWidget->append(QString("[%1] ERROR: Invalid line number %2 (valid range: 1-%3)")
                         .arg(timestamp, QString::number(lineNumber), QString::number(totalLines)));
        LOG_ERROR("KScrollToLine: Invalid line number: " + QString::number(lineNumber));
        logFunctionEnd("KScrollToLine");
        return;
    }
    
    logWidget->append(QString("[%1] Line number validation passed").arg(timestamp));
    LOG_DEBUG("KScrollToLine: Line number validation passed");
    
    // SECTION 5: CHECK SCINTILLA VIEW
    logWidget->append(QString("[%1] === SECTION 5: CHECK SCINTILLA VIEW ===").arg(timestamp));
    LOG_DEBUG("KScrollToLine: Section 5 - Check Scintilla view");
    
    if (!fileContentView) {
        logWidget->append(QString("[%1] ERROR: fileContentView is null!").arg(timestamp));
        LOG_ERROR("KScrollToLine: fileContentView is null");
        logFunctionEnd("KScrollToLine");
        return;
    }
    
    logWidget->append(QString("[%1] fileContentView is valid").arg(timestamp));
    LOG_DEBUG("KScrollToLine: fileContentView is valid");
    
    // SECTION 6: PERFORM SCROLLING
    logWidget->append(QString("[%1] === SECTION 6: PERFORM SCROLLING ===").arg(timestamp));
    LOG_DEBUG("KScrollToLine: Section 6 - Perform scrolling");
    
    logWidget->append(QString("[%1] About to call fileContentView->scrollToLine(%2)").arg(timestamp, QString::number(lineNumber)));
    LOG_DEBUG("KScrollToLine: About to call fileContentView->scrollToLine(" + QString::number(lineNumber) + ")");
    
    // Get viewport information before scrolling
    int viewportHeight = fileContentView->height();
    logWidget->append(QString("[%1] Viewport height: %2 pixels").arg(timestamp, QString::number(viewportHeight)));
    
    // Check if the file content is actually loaded in the ScintillaEdit widget
    QString currentText = fileContentView->getText();
    int textLength = currentText.length();
    logWidget->append(QString("[%1] Current text length in ScintillaEdit: %2 characters").arg(timestamp, QString::number(textLength)));
    
    // Check cache size and current file info
    int cacheSize = m_fileCache.size();
    logWidget->append(QString("[%1] Cache size: %2 files, Current file: %3").arg(timestamp, QString::number(cacheSize), m_currentActiveFile));
    
    if (textLength < 100) {
        logWidget->append(QString("[%1] WARNING: ScintillaEdit content seems too small, file may not be properly loaded").arg(timestamp));
        LOG_WARNING("KScrollToLine: ScintillaEdit content seems too small, file may not be properly loaded");
    }
    
    // Cache size info (no longer warning since we removed the limit)
    if (cacheSize > 0) {
        logWidget->append(QString("[%1] INFO: Cache contains %2 files").arg(timestamp, QString::number(cacheSize)));
    }
    
    try {
        // Actually scroll to the line using ScintillaEdit's scrollToLine method
        fileContentView->scrollToLine(lineNumber);
        
        logWidget->append(QString("[%1] fileContentView->scrollToLine(%2) completed successfully").arg(timestamp, QString::number(lineNumber)));
        LOG_DEBUG("KScrollToLine: scrollToLine completed successfully");
        
        // Also highlight the line
        fileContentView->highlightLine(lineNumber);
        
        logWidget->append(QString("[%1] fileContentView->highlightLine(%2) completed successfully").arg(timestamp, QString::number(lineNumber)));
        LOG_DEBUG("KScrollToLine: highlightLine completed successfully");
        
        // Force a repaint to ensure the scroll is visible
        fileContentView->update();
        fileContentView->repaint();
        
        logWidget->append(QString("[%1] Forced viewport update and repaint").arg(timestamp));
        LOG_DEBUG("KScrollToLine: Forced viewport update and repaint");
        
            } catch (const std::exception& e) {
        logWidget->append(QString("[%1] ERROR: Exception during scrolling: %2").arg(timestamp, QString(e.what())));
        LOG_ERROR("KScrollToLine: Exception during scrolling: " + QString(e.what()));
        logFunctionEnd("KScrollToLine");
        return;
    } catch (...) {
        logWidget->append(QString("[%1] ERROR: Unknown exception during scrolling").arg(timestamp));
        LOG_ERROR("KScrollToLine: Unknown exception during scrolling");
        logFunctionEnd("KScrollToLine");
                return;
            }
    
    // SECTION 7: COMPLETION
    logWidget->append(QString("[%1] === SECTION 7: COMPLETION ===").arg(timestamp));
    LOG_DEBUG("KScrollToLine: Section 7 - Completion");
    
    logWidget->append(QString("[%1] Successfully scrolled to line %2").arg(timestamp, QString::number(lineNumber)));
    logWidget->append(QString("[%1] Line %2 highlighted with color: %3")
                     .arg(timestamp, QString::number(lineNumber), m_highlightColor.name()));
    
    logWidget->append(QString("[%1] === KSCROLLTOLINE COMPLETED ===").arg(timestamp));
    LOG_DEBUG("KScrollToLine: completed successfully - line " + QString::number(lineNumber));
    logFunctionEnd("KScrollToLine");
}

void MainWindow::KFindInFile()
{
                        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    LOG_DEBUG("KFindInFile: started");
    logWidget->append(QString("[%1] === KFINDINFILE START ===").arg(timestamp));
    
    // Check if we have a file loaded
    if (!m_logDataWorker || !m_logDataWorker->isFileLoaded()) {
        logWidget->append(QString("[%1] ERROR: No file loaded! Please open a file first.").arg(timestamp));
        LOG_ERROR("KFindInFile: No file loaded");
            return;
        }
        
    // Create and show the search dialog with saved settings
    SearchDialog dialog(this);
    
    // Set saved values in the dialog (from memory, not from file)
    dialog.setPattern(m_findInFilePattern);
    dialog.setSearchEngine(m_findInFileEngine);
    dialog.setSearchType(m_findInFileType);
    dialog.setCaseSensitive(m_findInFileCaseSensitive);
    dialog.setInverse(m_findInFileInverse);
    dialog.setBoolean(m_findInFileBoolean);
    dialog.setPlainText(m_findInFilePlainText);
    dialog.setAutoRefresh(m_findInFileAutoRefresh);
    dialog.setStartLine(m_findInFileStartLine);
    dialog.setEndLine(m_findInFileEndLine);
    dialog.setHighlightColor(m_findInFileHighlightColor);
    
    if (dialog.exec() != QDialog::Accepted) {
        logWidget->append(QString("[%1] Search cancelled by user").arg(timestamp));
        LOG_DEBUG("KFindInFile: Search cancelled");
            return;
        }
        
    // Get search parameters from dialog
    QString pattern = dialog.getPattern();
    QString searchEngine = dialog.getSearchEngine();
    QString searchType = dialog.getSearchType();
    bool caseSensitive = dialog.isCaseSensitive();
    bool inverse = dialog.isInverse();
    bool boolean = dialog.isBoolean();
    bool plainText = dialog.isPlainText();
    bool autoRefresh = dialog.isAutoRefresh();
    int startLine = dialog.getStartLine();
    int endLine = dialog.getEndLine();
    QColor highlightColor = dialog.getHighlightColor();
    
    // Update memory variables for next use
    m_findInFilePattern = pattern;
    m_findInFileEngine = searchEngine;
    m_findInFileType = searchType;
    m_findInFileCaseSensitive = caseSensitive;
    m_findInFileInverse = inverse;
    m_findInFileBoolean = boolean;
    m_findInFilePlainText = plainText;
    m_findInFileAutoRefresh = autoRefresh;
    m_findInFileStartLine = startLine;
    m_findInFileEndLine = endLine;
    m_findInFileHighlightColor = highlightColor;
    
    // Log search pattern and parameters in one line
    QString searchParams = QString("Pattern: '%1', Engine: %2, Type: %3, Case: %4, Inverse: %5, Boolean: %6, Plain: %7, Auto: %8, Range: %9-%10, Color: %11")
                          .arg(pattern)
                          .arg(searchEngine)
                          .arg(searchType)
                          .arg(caseSensitive ? "Yes" : "No")
                          .arg(inverse ? "Yes" : "No")
                          .arg(boolean ? "Yes" : "No")
                          .arg(plainText ? "Yes" : "No")
                          .arg(autoRefresh ? "Yes" : "No")
                          .arg(startLine)
                          .arg(endLine)
                          .arg(highlightColor.name());
    
    LOG_INFO("Search parameters: " + searchParams);
    logWidget->append(QString("[%1] Search parameters: %2").arg(timestamp, searchParams));
    
    // Save the search settings for next time
    saveFindInFileSettings(pattern, searchEngine, searchType, caseSensitive, inverse, boolean, 
                          plainText, autoRefresh, startLine, endLine, highlightColor);
    
    // Start timing the search
    QElapsedTimer searchTimer;
    searchTimer.start();
    
    if (m_logDataWorker) {
        // Perform synchronous search to get immediate results with timing
        m_logDataWorker->searchInFileSync(pattern, caseSensitive, inverse, boolean, plainText, startLine, endLine);
        
        QList<int> searchResults = m_logDataWorker->getSearchResults();
        
        // Calculate search time
        qint64 searchTime = searchTimer.elapsed();
        QString searchTimeMsg = QString("Search for '%1' took %2 msec").arg(pattern, QString::number(searchTime));
        
        if (searchResults.isEmpty()) {
            logWidget->append(QString("[%1] No matches found").arg(timestamp));
            logWidget->append(QString("[%1] %2").arg(timestamp, searchTimeMsg));
            LOG_INFO("KFindInFile: No matches found");
            LOG_INFO("KFindInFile: " + searchTimeMsg);
            
            // Clear any previous search results

        } else {
            logWidget->append(QString("[%1] Found %2 matches").arg(timestamp, QString::number(searchResults.size())));
            logWidget->append(QString("[%1] %2").arg(timestamp, searchTimeMsg));
            LOG_INFO("KFindInFile: Found " + QString::number(searchResults.size()) + " matches");
            LOG_INFO("KFindInFile: " + searchTimeMsg);
            
        }
    } else {
        logWidget->append(QString("[%1] ERROR: No LogDataWorker available").arg(timestamp));
        LOG_ERROR("KFindInFile: No LogDataWorker available");
    }
    

}


/*
void MainWindow::KRGSearch()
{
    logFunctionStart("KRGSearch");
    
                        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    logWidget->append(QString("[%1] === KRGSEARCH START ===").arg(timestamp));
    
    
    // Create and show the RG Search dialog
    RGSearchDialog dialog(this);
    dialog.setLogWidget(logWidget);
 
    
    
    // Set the main window reference in the KSearchBun instance
    dialog.kSearchBun->setMainWindow(this);
    
    if (dialog.exec() != QDialog::Accepted) {
        logWidget->append(QString("[%1] RG Search cancelled by user").arg(timestamp));
        LOG_DEBUG("KRGSearch: Search cancelled");
        logFunctionEnd("KRGSearch");
            return;
        }
        
    // Activate search results status lamp
        setSearchResultsLamp(true);
        
    // Get search parameters from dialog
    RGSearchParams params = dialog.getSearchParams();
    
    // Store the RG Search pattern for Pattern 0
    m_rgSearchPattern = params.pattern;
    LOG_INFO("KRGSearch: Stored RG Search pattern for Pattern 0: '" + m_rgSearchPattern + "'");
    
    // Store the cache setting for file handling
    m_keepFilesInCache = params.keep_files_in_cache;
    LOG_INFO("KRGSearch: File cache setting: " + QString(m_keepFilesInCache ? "ENABLED" : "DISABLED"));
    logWidget->append(QString("[%1] File cache setting: %2").arg(timestamp, m_keepFilesInCache ? "ENABLED" : "DISABLED"));
    updateCacheStatusDisplay();
    
    // Store the highlight color from RG search dialog
    m_rgSearchHighlightColor = params.highlight_color;
    LOG_INFO("KRGSearch: Highlight color: " + m_rgSearchHighlightColor.name());
    logWidget->append(QString("[%1] Highlight color: %2").arg(timestamp, m_rgSearchHighlightColor.name()));
    
    // Log search parameters
    QString searchParams = QString("Path: '%1', Pattern: '%2', Fixed String: %3, Add Pattern: '%4', "
                                  "Case Sensitive: %5, Ignore Case: %6, Smart Case: %7, Include/Exclude: '%8'")
                          .arg(params.path)
                          .arg(params.pattern)
                          .arg(params.fixed_string ? "Yes" : "No")
                          .arg(params.add_pattern)
                          .arg(params.case_sensitive ? "Yes" : "No")
                          .arg(params.ignore_case ? "Yes" : "No")
                          .arg(params.smart_case ? "Yes" : "No")
                          .arg(params.incl_exclude);
    
    LOG_INFO("RG Search parameters: " + searchParams);
    logWidget->append(QString("[%1] RG Search parameters: %2").arg(timestamp, searchParams));
 
    // Add to search history
    if (m_logDataWorker) {
        m_logDataWorker->addToSearchHistory(params.pattern, params.path);
        LOG_INFO("Search added to history: " + params.pattern + " in " + params.path);
    }
    
    // Disable search button to prevent multiple searches
            m_kSearch->setSearchButton(SearchButtonState::SEARCHING);

    // ===== COMPLETE CLEANUP - FRESH START =====
    LOG_INFO("KRGSearch: Calling KCompleteCleanUp for fresh start");
            m_kSearch->KCompleteCleanUp();
    
    // ===== CREATE PARSE THREAD AFTER CLEANUP =====
    LOG_INFO("KRGSearch: Creating parse thread after cleanup");
    LOG_INFO("KRGSearch: collapsibleSearchResults pointer: " + QString(collapsibleSearchResults ? "EXISTS" : "NULL"));
    if (collapsibleSearchResults) {
        LOG_INFO("KRGSearch: About to call createParseThread");
        collapsibleSearchResults->createParseThread();
        LOG_INFO("KRGSearch: createParseThread called successfully");
    } else {
        LOG_ERROR("KRGSearch: collapsibleSearchResults is null - cannot create parse thread!");
    }
    
    // Apply existing highlight rules before starting RG search
    LOG_INFO("KRGSearch: Applying existing highlight rules before search");
    applyExtraHighlightParamsRules();



     
   // display message - starting to search
    if (collapsibleSearchResults) {
        LOG_INFO("KRGSearch: Showing 'Searching...' loading message in results window");
        collapsibleSearchResults->showSearchingMessage();
    }



    // Execute the search using KSearchBun
    QElapsedTimer searchTimer;
    searchTimer.start();
    

    // ===== TRUE ASYNCHRONOUS MODE =====
    // Reset ERROR state to IDLE when starting new search
    if (m_currentState == SearchState::ERROR) {
        LOG_INFO("KRGSearch: Resetting ERROR state to IDLE for new search");
        logWidget->append(QString("[%1] 🔄 RESETTING ERROR STATE - Starting new search").arg(timestamp));
        m_kSearch->updateSearchState(SearchState::IDLE);
    }
    
        m_kSearch->updateSearchState(SearchState::SEARCHING);

        logWidget->append(QString("[%1] === THREAD 1: SearchThread STARTED ===").arg(timestamp));
        LOG_INFO("KRGSearch: Thread 1 (SearchThread) - Starting ripgrep execution");
        
        
        // Disconnect any existing connections to prevent multiple signal handlers
        disconnect(m_searchBun, &KSearchBun::asyncSearchCompleted, nullptr, nullptr);
        
        // Connect to the async search completion signal (Stage 1: ripgrep completes)
        connect(m_searchBun, &KSearchBun::asyncSearchCompleted, 
                [this, params, searchTimer, timestamp](const QString &rawOutput) {
            
            qint64 searchTime = searchTimer.elapsed();
            LOG_INFO("KRGSearch: Stage 1 - Async ripgrep completed in " + QString::number(searchTime) + " ms");
            
            // Stage 2: Display JSON results directly (no parsing needed)
            m_kSearch->updateSearchState(SearchState::PARSING_MAIN_SEARCH);

            LOG_INFO("KRGSearch: Stage 2 - Starting JSON display...");
            
            // Processing state and parsing lamp are now handled in performSearchWithCurrentParams
            
            // Add timing right before the call
            QElapsedTimer displayTimer;
            displayTimer.start();
            LOG_INFO("KRGSearch: ⚠️ TIMING - About to enter KDisplayResultWin (JSON) at " + QString::number(displayTimer.elapsed()) + " ms");
            
            KDisplayResultWin(params.pattern, rawOutput, params.path);
            LOG_INFO("KRGSearch: Thread 2 (DisplayThread) - JSON display completed, persisting until new RG_Search");
            
            // UI updates (removed try-catch blocks as requested)
 //           update();
            
 //           repaint();
            
            LOG_INFO("KRGSearch: ⚠️ TIMING - Exited KDisplayResultWin (JSON) at " + QString::number(displayTimer.elapsed()) + " ms");
            LOG_INFO("KRGSearch: KDisplayResultWin (JSON) completed");
            logWidget->append(QString("[%1] KRGSearch: KDisplayResultWin (JSON) completed").arg(timestamp));
            
            // Processing state and parsing lamp are now handled in performSearchWithCurrentParams
            
            // For JSON mode, we don't do file mapping since we don't have parsed results
            LOG_INFO("KRGSearch: JSON mode - skipping file mapping phase");
            
            // File mapping option (currently disabled with flag)
            bool enableFileMapping = false;  // Flag to control file mapping
            
            if (enableFileMapping) {
                LOG_INFO("KRGSearch: About to start file mapping phase...");
                
                // Extract file paths from JSON data for mapping
                QStringList filesToMap;
                QStringList lines = rawOutput.split('\n', Qt::SkipEmptyParts);
                
                for (const QString &line : lines) {
                    if (line.trimmed().isEmpty()) continue;
                    
                    QJsonParseError error;
                    QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);
                    
                    if (error.error == QJsonParseError::NoError) {
                        QJsonObject obj = doc.object();
                        QString type = obj["type"].toString();
                        
                        if (type == "match") {
                            QJsonObject dataObj = obj["data"].toObject();
                            QString filePath = dataObj["path"].toObject()["text"].toString();
                            
                            if (!filePath.isEmpty() && !filesToMap.contains(filePath)) {
                                filesToMap.append(filePath);
                            }
                        }
                    }
                }
                
                // Send all files in results to KMap function for mapping
                m_kSearch->updateSearchState(SearchState::MAPPING_IN_BACKGROUND);
                LOG_INFO("KRGSearch: Starting KMap for all found files...");
                QElapsedTimer kMapTimer;
                kMapTimer.start();
                
                int totalFilesMapped = 0;
                LOG_INFO("KRGSearch: About to iterate through " + QString::number(filesToMap.size()) + " files");
                
                for (const QString &filePath : filesToMap) {
                    LOG_INFO("KRGSearch: About to map file: " + filePath);
                    
                    // Create KSearchBun instance and call KMap for each file
                    KSearchBun kSearchBun;
                    LOG_INFO("KRGSearch: KSearchBun created for file: " + filePath);
                    
                    QVector<long> lineOffsets = kSearchBun.KMap(filePath);
                    LOG_INFO("KRGSearch: KMap completed for file: " + filePath + " with " + QString::number(lineOffsets.size()) + " line offsets");
                    
                    totalFilesMapped++;
                    LOG_INFO("KRGSearch: Successfully mapped file " + QString::number(totalFilesMapped) + " of " + QString::number(filesToMap.size()));
                }
                
                qint64 kMapTime = kMapTimer.elapsed();
                LOG_INFO("KRGSearch: KMap completed for " + QString::number(totalFilesMapped) + " files in " + QString::number(kMapTime) + " ms");
                logWidget->append(QString("[%1] KRGSearch: KMap completed for " + QString::number(totalFilesMapped) + " files in " + QString::number(kMapTime) + " ms").arg(timestamp));
            } else {
                LOG_INFO("KRGSearch: File mapping disabled (enableFileMapping = false)");
            }
            
        // Deactivate search results status lamp
        setSearchResultsLamp(false);
        
        // Re-enable search button now that search is complete
        m_kSearch->setSearchButton(SearchButtonState::IDLE);
            
            m_kSearch->updateSearchState(SearchState::IDLE);
            LOG_DEBUG("KRGSearch: completed");
            logWidget->append(QString("[%1] KRGSearch: completed").arg(timestamp));
        });
        
        // Start the asynchronous search using search object
        m_searchBun->K_RGresults_method3_async(params);
        
        LOG_INFO("KRGSearch: Asynchronous search started, continuing execution...");
        LOG_INFO("KRGSearch: Thread 1 (SearchThread) - Search started, will persist until new RG_Search");

    logFunctionEnd("KRGSearch");
}
*/


void MainWindow::KKMap()
{

    
    LOG_INFO("MainWindow: ===THREAD=== KKMap <<<<<STARTed<<<<<");
    
    // Open file dialog to select a file
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select File for KMap",
        QDir::currentPath(),
        "All Files (*.*)"
    );
    
    
    if (filePath.isEmpty()) {
        LOG_INFO("MainWindow: KKMap - No file selected");
        LOG_INFO("MainWindow: ===THREAD=== KKMap >>>>>ENDed>>>>>");
            return;
        }
        
    LOG_INFO("MainWindow: KKMap - Selected file: " + filePath);

    QElapsedTimer timer;
    timer.start();   
     
    // Create KSearchBun instance and call KMap
    KSearchBun kSearchBun;
    QVector<long> lineOffsets = kSearchBun.KMap(filePath);
    

    LOG_INFO("MainWindow: ===THREAD=== KKMap >>>>>ENDed>>>>>");
    LOG_INFO("MainWindow: TIMING: KKMap took " + QString::number(timer.elapsed() / 1000.0, 'f', 3) + " sec");
    logFunctionEnd("KKMap");
}

void MainWindow::KClockTest()
{
    LOG_INFO("MainWindow: ===THREAD=== KClockTest <<<<<STARTed<<<<<");
    
    // Create and show the Clock Test dialog
    ClockTestDialog dialog(this);
    dialog.exec();
    
    LOG_INFO("MainWindow: ===THREAD=== KClockTest >>>>>ENDed>>>>>");
    logFunctionEnd("KClockTest");
}

void MainWindow::KTestPane()
{
    LOG_INFO("MainWindow: ===THREAD=== KTestPane <<<<<STARTed<<<<<");
    
    // Create a custom dialog for test pane options
    QDialog dialog(this);
    dialog.setWindowTitle("Test Pane Options");
    dialog.setModal(true);
    dialog.resize(400, 300);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    
    // Section 1: Clear options
    QGroupBox *clearGroup = new QGroupBox("Clear Panes");
    QVBoxLayout *clearLayout = new QVBoxLayout(clearGroup);
    
    QPushButton *clearSearchButton = new QPushButton("Clear Search Result Pane");
    clearSearchButton->setIcon(QIcon(":/icons/clear.png"));
    QPushButton *clearViewerButton = new QPushButton("Clear File Content Viewer Pane");
    clearViewerButton->setIcon(QIcon(":/icons/clear.png"));
    QPushButton *clearLogButton = new QPushButton("Clear Log Pane");
    clearLogButton->setIcon(QIcon(":/icons/clear.png"));
    
    clearLayout->addWidget(clearSearchButton);
    clearLayout->addWidget(clearViewerButton);
    clearLayout->addWidget(clearLogButton);
    
    // Section 2: Test options
    QGroupBox *testGroup = new QGroupBox("Test Panes");
    QVBoxLayout *testLayout = new QVBoxLayout(testGroup);
    
    QCheckBox *testSearchCheck = new QCheckBox("Search Result Pane");
    QCheckBox *testViewerCheck = new QCheckBox("File Content Viewer Pane");
    QCheckBox *testLogCheck = new QCheckBox("Log Pane");
    
    testLayout->addWidget(testSearchCheck);
    testLayout->addWidget(testViewerCheck);
    testLayout->addWidget(testLogCheck);
    
    // Section 3: Text input
    QGroupBox *textGroup = new QGroupBox("Test Text");
    QVBoxLayout *textLayout = new QVBoxLayout(textGroup);
    
    QLabel *textLabel = new QLabel("Enter text to display in selected panes:");
    QTextEdit *textInput = new QTextEdit();
    textInput->setMaximumHeight(100);
    textInput->setPlaceholderText("Enter test text here...");
    
    textLayout->addWidget(textLabel);
    textLayout->addWidget(textInput);
    
    // Add sections to main layout
    mainLayout->addWidget(clearGroup);
    mainLayout->addWidget(testGroup);
    mainLayout->addWidget(textGroup);
    
    // Connect clear buttons
    connect(clearSearchButton, &QPushButton::clicked, [this]() {
        searchResults->clear();
        logWidget->append("Search Result Pane cleared");
        LOG_INFO("KTestPane: Search Result Pane cleared");
    });
    
    connect(clearViewerButton, &QPushButton::clicked, [this]() {
        fileContentView->clearText();
        logWidget->append("File Content Viewer Pane cleared");
        LOG_INFO("KTestPane: File Content Viewer Pane cleared");
    });
    
    connect(clearLogButton, &QPushButton::clicked, [this]() {
        logWidget->clear();
        LOG_INFO("KTestPane: Log Pane cleared");
    });
    
    // Add a button to apply test text
    QPushButton *applyButton = new QPushButton("Apply Test Text");
    applyButton->setIcon(QIcon(":/icons/save.png"));
    mainLayout->addWidget(applyButton);
    
    // Connect apply button
    connect(applyButton, &QPushButton::clicked, [this, textInput, testSearchCheck, testViewerCheck, testLogCheck]() {
        QString testText = textInput->toPlainText();
        if (testText.isEmpty()) {
            QMessageBox::warning(this, "Warning", "Please enter some text to test.");
            return;
        }
        
        // Apply text to selected panes
        if (testSearchCheck->isChecked()) {
            searchResults->addItem("Test Item: " + testText);
            logWidget->append("Added test text to Search Result Pane");
        }
        
        if (testViewerCheck->isChecked()) {
            fileContentView->setText(testText);
            logWidget->append("Added test text to File Content Viewer Pane");
        }
        
        if (testLogCheck->isChecked()) {
            logWidget->append("Test Log Entry: " + testText);
        }
        
        LOG_INFO("KTestPane: Applied test text to selected panes");
    });
    
    // Show the dialog
    dialog.exec();
    
    LOG_INFO("MainWindow: ===THREAD=== KTestPane >>>>>ENDed>>>>>");
    logFunctionEnd("KTestPane");
}

void MainWindow::KUpdateFileViewer(const QString &filePath, int lineNumber, const QString &lineText)
{
    LOG_INFO("KUpdateFileViewer: === START ===");
    LOG_INFO("KUpdateFileViewer: File: " + filePath + ", Line: " + QString::number(lineNumber) + ", Text: " + lineText);
    
                        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    logWidget->append(QString("[%1] === KUPDATEFILEVIEWER START ===").arg(timestamp));
    logWidget->append(QString("[%1] File: %2, Line: %3, Text: %4").arg(timestamp, filePath, QString::number(lineNumber), lineText));
    
    // SECTION 1: INPUT VALIDATION
    logWidget->append(QString("[%1] === SECTION 1: INPUT VALIDATION ===").arg(timestamp));
    LOG_INFO("KUpdateFileViewer: Section 1 - Input validation");
    
    if (filePath.isEmpty()) {
        logWidget->append(QString("[%1] ERROR: File path is empty").arg(timestamp));
        LOG_ERROR("KUpdateFileViewer: File path is empty");
        return;
    }
    
    if (lineNumber <= 0) {
        logWidget->append(QString("[%1] ERROR: Invalid line number: %2").arg(timestamp, QString::number(lineNumber)));
        LOG_ERROR("KUpdateFileViewer: Invalid line number: " + QString::number(lineNumber));
        return;
    }
    
    if (lineText.isEmpty()) {
        logWidget->append(QString("[%1] WARNING: Line text is empty").arg(timestamp));
        LOG_WARNING("KUpdateFileViewer: Line text is empty");
    }
    
    logWidget->append(QString("[%1] Input validation passed").arg(timestamp));
    LOG_INFO("KUpdateFileViewer: Input validation passed");
    
    // SECTION 2: FILE OPENING AND MAPPING
    logWidget->append(QString("[%1] === SECTION 2: FILE OPENING AND MAPPING ===").arg(timestamp));
    LOG_INFO("KUpdateFileViewer: Section 2 - File opening and mapping");
    
    // Check if file is already open
    bool fileAlreadyOpen = false;
    if (m_logDataWorker && m_logDataWorker->isFileLoaded()) {
        // Check if the current loaded file matches our target
        QString currentFilePath = m_logDataWorker->getFilePath();
        if (currentFilePath == filePath) {
            fileAlreadyOpen = true;
            logWidget->append(QString("[%1] File is already open: %2").arg(timestamp, filePath));
            LOG_INFO("KUpdateFileViewer: File is already open: " + filePath);
        } else {
            logWidget->append(QString("[%1] Different file is open, opening target: %2").arg(timestamp, filePath));
            LOG_INFO("KUpdateFileViewer: Different file is open, opening target: " + filePath);
        }
    } else {
        logWidget->append(QString("[%1] No file is open, opening: %2").arg(timestamp, filePath));
        LOG_INFO("KUpdateFileViewer: No file is open, opening: " + filePath);
    }
    
    // Open the file if not already open
    if (!fileAlreadyOpen) {
        // Create LogDataWorker if it doesn't exist
        if (!m_logDataWorker) {
            logWidget->append(QString("[%1] Creating LogDataWorker...").arg(timestamp));
            LOG_INFO("KUpdateFileViewer: Creating LogDataWorker");
                m_logDataWorker = new LogDataWorker(this);
            
            // Connect progress messages to log widget
            connect(m_logDataWorker, &LogDataWorker::progressMessage, [this](const QString& message) {
                QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
                logWidget->append(QString("[%1] %2").arg(timestamp, message));
            });
            
            // Connect indexing finished to show completion
            connect(m_logDataWorker, &LogDataWorker::indexingFinished, [this](bool success) {
                        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
                if (success) {
                    logWidget->append(QString("[%1] File loading completed successfully!").arg(timestamp));
                } else {
                    logWidget->append(QString("[%1] ERROR: File loading failed!").arg(timestamp));
                }
            });
        } else {
            // Reconnect signals
            connect(m_logDataWorker, &LogDataWorker::progressMessage, [this](const QString& message) {
                QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
                logWidget->append(QString("[%1] %2").arg(timestamp, message));
            });
            
            connect(m_logDataWorker, &LogDataWorker::indexingFinished, [this](bool success) {
                        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
                if (success) {
                    logWidget->append(QString("[%1] File loading completed successfully!").arg(timestamp));
                } else {
                    logWidget->append(QString("[%1] ERROR: File loading failed!").arg(timestamp));
                }
            });
        }
        
        m_logDataWorker->startIndexing(filePath);
        logWidget->append(QString("[%1] File indexing started").arg(timestamp));
        LOG_INFO("KUpdateFileViewer: File indexing started");
    }
    
    // Check if file was already mapped in current search session
    bool fileAlreadyMappedInSession = m_mappedFiles.contains(filePath);
    if (fileAlreadyMappedInSession) {
        logWidget->append(QString("[%1] File already mapped in current session: %2").arg(timestamp, filePath));
        LOG_INFO("KUpdateFileViewer: File already mapped in current session: " + filePath);
    } else {
        logWidget->append(QString("[%1] File not mapped in current session: %2").arg(timestamp, filePath));
        LOG_INFO("KUpdateFileViewer: File not mapped in current session: " + filePath);
    }
    
    // Get line offsets (either use existing mapping or create new mapping)
    QVector<long> lineOffsets;
    if (fileAlreadyMappedInSession) {
        // File was already mapped in this session - use stored line offsets
        lineOffsets = m_mappedFiles[filePath];
        logWidget->append(QString("[%1] File already mapped in current session, using stored mapping. Line offsets count: %2").arg(timestamp, QString::number(lineOffsets.size())));
        LOG_INFO("KUpdateFileViewer: File already mapped in current session, using stored mapping. Line offsets count: " + QString::number(lineOffsets.size()));
    } else {
        // First time mapping this file in current session
        logWidget->append(QString("[%1] Creating new mapping for file (first time in session)").arg(timestamp));
        LOG_INFO("KUpdateFileViewer: Creating new mapping for file (first time in session)");
        
        KSearchBun kSearchBun;
        lineOffsets = kSearchBun.KMap(filePath);
        
        // Store this file's line offsets in current session
        m_mappedFiles[filePath] = lineOffsets;
        logWidget->append(QString("[%1] File mapping stored in current session: %2").arg(timestamp, filePath));
        LOG_INFO("KUpdateFileViewer: File mapping stored in current session: " + filePath);
        
        logWidget->append(QString("[%1] New mapping completed. Line offsets count: %2").arg(timestamp, QString::number(lineOffsets.size())));
        LOG_INFO("KUpdateFileViewer: New mapping completed. Line offsets count: " + QString::number(lineOffsets.size()));
    }
    
    if (lineOffsets.isEmpty()) {
        logWidget->append(QString("[%1] ERROR: No line offsets found for file").arg(timestamp));
        LOG_ERROR("KUpdateFileViewer: No line offsets found for file");
                return;
            }
        
    // SECTION 3: VIEWER SIZE CALCULATION
    logWidget->append(QString("[%1] === SECTION 3: VIEWER SIZE CALCULATION ===").arg(timestamp));
    LOG_INFO("KUpdateFileViewer: Section 3 - Viewer size calculation");
    
    // Get the file viewer pane vertical size in lines
    int viewerHeight = fileContentView->height();
    int lineHeight = 20; // Default line height in pixels
    int visibleLines = viewerHeight / lineHeight;
    
    logWidget->append(QString("[%1] Viewer height: %2 pixels, Line height: %3 pixels, Visible lines: %4")
                     .arg(timestamp, QString::number(viewerHeight), QString::number(lineHeight), QString::number(visibleLines)));
    LOG_INFO("KUpdateFileViewer: Viewer height: " + QString::number(viewerHeight) + " pixels, Visible lines: " + QString::number(visibleLines));
    
    if (visibleLines <= 0) {
        logWidget->append(QString("[%1] ERROR: Invalid visible lines: %2").arg(timestamp, QString::number(visibleLines)));
        LOG_ERROR("KUpdateFileViewer: Invalid visible lines: " + QString::number(visibleLines));
            return;
        }
        
    // SECTION 4: LINE RANGE CALCULATION
    logWidget->append(QString("[%1] === SECTION 4: LINE RANGE CALCULATION ===").arg(timestamp));
    LOG_INFO("KUpdateFileViewer: Section 4 - Line range calculation");
    
    // Calculate the range of lines to display
    // If line aaa is requested and the size of the vertical pane is 10, then xx is aaa-5 and yyy is aaa+5
    int startLine = lineNumber - (visibleLines / 2);
    int endLine = lineNumber + (visibleLines / 2);
    
    logWidget->append(QString("[%1] Initial calculation: startLine=%2, endLine=%3 (target line=%4, visibleLines=%5)")
                     .arg(timestamp, QString::number(startLine), QString::number(endLine), QString::number(lineNumber), QString::number(visibleLines)));
    LOG_INFO("KUpdateFileViewer: Initial calculation: startLine=" + QString::number(startLine) + ", endLine=" + QString::number(endLine));
    
    // SECTION 5: BOUNDARY ADJUSTMENT
    logWidget->append(QString("[%1] === SECTION 5: BOUNDARY ADJUSTMENT ===").arg(timestamp));
    LOG_INFO("KUpdateFileViewer: Section 5 - Boundary adjustment");
    
    // Ensure we don't go out of bounds
    if (startLine < 1) {
        logWidget->append(QString("[%1] Adjusting startLine from %2 to 1 (below minimum)").arg(timestamp, QString::number(startLine)));
        LOG_INFO("KUpdateFileViewer: Adjusting startLine from " + QString::number(startLine) + " to 1");
        startLine = 1;
        endLine = qMin(visibleLines, lineOffsets.size());
    }
    
    if (endLine > lineOffsets.size()) {
        logWidget->append(QString("[%1] Adjusting endLine from %2 to %3 (above maximum)").arg(timestamp, QString::number(endLine), QString::number(lineOffsets.size())));
        LOG_INFO("KUpdateFileViewer: Adjusting endLine from " + QString::number(endLine) + " to " + QString::number(lineOffsets.size()));
        endLine = lineOffsets.size();
        startLine = qMax(1, lineOffsets.size() - visibleLines + 1);
    }
    
    logWidget->append(QString("[%1] Final range: startLine=%2, endLine=%3").arg(timestamp, QString::number(startLine), QString::number(endLine)));
    LOG_INFO("KUpdateFileViewer: Final range: startLine=" + QString::number(startLine) + ", endLine=" + QString::number(endLine));
    
    // SECTION 6: DISPLAY RANGE LOGGING
    logWidget->append(QString("[%1] === SECTION 6: DISPLAY RANGE LOGGING ===").arg(timestamp));
    LOG_INFO("KUpdateFileViewer: Section 6 - Display range logging");
    
    // Log the calculated range
    logWidget->append(QString("[%1] Lines %2 to %3 from file %4").arg(timestamp, QString::number(startLine), QString::number(endLine), filePath));
    LOG_INFO("KUpdateFileViewer: Lines " + QString::number(startLine) + " to " + QString::number(endLine) + " from file " + filePath);
    
    // Log the line offsets for the calculated range
    logWidget->append(QString("[%1] Line offsets for range %2-%3:").arg(timestamp, QString::number(startLine), QString::number(endLine)));
    for (int i = startLine - 1; i < endLine && i < lineOffsets.size(); ++i) {
        logWidget->append(QString("[%1] Line %2: offset %3").arg(timestamp, QString::number(i + 1), QString::number(lineOffsets[i])));
    }
    
    // SECTION 7: COMPLETION
    logWidget->append(QString("[%1] === SECTION 7: COMPLETION ===").arg(timestamp));
    LOG_INFO("KUpdateFileViewer: Section 7 - Completion");
    
    logWidget->append(QString("[%1] KUpdateFileViewer: printing to screen completed").arg(timestamp));
    LOG_INFO("KUpdateFileViewer: printing to screen completed");
    
    logWidget->append(QString("[%1] === KUPDATEFILEVIEWER COMPLETED ===").arg(timestamp));
    LOG_INFO("KUpdateFileViewer: === COMPLETED ===");
    logFunctionEnd("KUpdateFileViewer");
}

void MainWindow::logFunctionStart(const QString& functionName)
{
                        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString message = QString("[%1] KSearch.cpp-%2: START").arg(timestamp, functionName);
    logWidget->append(message);
    LOG_INFO("KSearch.cpp-" + functionName + ": START");
    m_kSearch->updateSearchState(m_currentState); // Keep current state but log the function
}

void MainWindow::logFunctionEnd(const QString& functionName)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString message = QString("[%1] KSearch.cpp-%2: END").arg(timestamp, functionName);
    logWidget->append(message);
    LOG_INFO("KSearch.cpp-" + functionName + ": END");
    m_kSearch->updateSearchState(m_currentState); // Keep current state but log the function
}

// New bulk read methods with NUL handling
QString MainWindow::sanitizeForDisplay(const QByteArray& data)
{
    logFunctionStart("sanitizeForDisplay");
    
    QElapsedTimer timer;
    timer.start();
    
    LOG_INFO("sanitizeForDisplay: Starting NUL character sanitization");
    LOG_DEBUG("sanitizeForDisplay: Input data size: " + QString::number(data.size()) + " bytes");
    
    // FAST METHOD: Use QByteArray operations instead of character-by-character
    QByteArray sanitized = data;
    
    // Count NUL characters first
    int nulCount = sanitized.count('\0');
    LOG_DEBUG("sanitizeForDisplay: Found " + QString::number(nulCount) + " NUL characters");
    
    if (nulCount > 0) {
        // Replace all NUL characters with spaces in one operation
        sanitized.replace('\0', ' ');
        LOG_DEBUG("sanitizeForDisplay: Replaced " + QString::number(nulCount) + " NUL characters with spaces");
    }
    
    // Convert to QString using fromUtf8 (much faster than character-by-character)
    QString result = QString::fromUtf8(sanitized);
    
    qint64 sanitizeTime = timer.elapsed();
    LOG_INFO("sanitizeForDisplay: Completed sanitization in " + QString::number(sanitizeTime) + "ms");
    LOG_DEBUG("sanitizeForDisplay: Output size: " + QString::number(result.size()) + " characters");
    
    logFunctionEnd("sanitizeForDisplay");
    return result;
}

void MainWindow::KDisplayFile_BulkRead()
{
    QElapsedTimer timer;
    timer.start();
    
                        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    logWidget->append(QString("[%1] === KDisplayFile_BulkRead STARTED ===").arg(timestamp));
    LOG_INFO("KDisplayFile_BulkRead: Starting bulk read method");
    
    if (m_logDataWorker && m_logDataWorker->isFileLoaded()) {
        QString filePath = m_logDataWorker->getFilePath();
        logWidget->append(QString("[%1] File is loaded: %2").arg(timestamp, filePath));
        LOG_INFO("KDisplayFile_BulkRead: File is loaded: " + filePath);
        
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            logWidget->append(QString("[%1] File opened successfully for bulk read").arg(timestamp));
            LOG_INFO("KDisplayFile_BulkRead: File opened successfully for bulk read");
            
            // Try memory-mapping for zero-copy I/O
            qint64 fsize = file.size();
            const uchar* mapped = fsize > 0 ? file.map(0, fsize) : nullptr;
            if (mapped) {
                qint64 mapTime = timer.elapsed();
                logWidget->append(QString("[%1] Memory map succeeded: %2 bytes in %3ms").arg(timestamp, QString::number(fsize), QString::number(mapTime)));
                LOG_INFO("KDisplayFile_BulkRead: Memory map succeeded: " + QString::number(fsize) + " bytes in " + QString::number(mapTime) + "ms");

                // Detect NULs; if present, copy and sanitize, otherwise send directly
                bool hasNul = false;
                for (qint64 i = 0; i < fsize; ++i) {
                    if (mapped[i] == '\0') { hasNul = true; break; }
                }
                if (hasNul) {
                    QByteArray sanitized(reinterpret_cast<const char*>(mapped), static_cast<int>(fsize));
                    for (int i = 0; i < sanitized.size(); ++i) {
                        if (sanitized[i] == '\0') sanitized[i] = ' ';
                    }
                    fileContentView->setUtf8Bytes(sanitized.constData(), sanitized.size());
                } else {
                    fileContentView->setUtf8Bytes(reinterpret_cast<const char*>(mapped), static_cast<int>(fsize));
                }

                // Unmap after Scintilla copies data
                file.unmap(const_cast<uchar*>(mapped));

                qint64 totalTime = timer.elapsed();
                if (fileContentView) {
                    LOG_INFO("KDisplayFile_BulkRead: Clearing loading message - file loaded successfully (mmap)");
                    fileContentView->setStyleSheet("");
                }
                logWidget->append(QString("[%1] Content set via mmap. Total time: %2ms").arg(timestamp, QString::number(totalTime)));
                LOG_INFO("KDisplayFile_BulkRead: Content set via mmap. Total time: " + QString::number(totalTime) + "ms");

                file.close();
            } else {
                // Fallback to single read
            QByteArray fileData = file.readAll();
            qint64 readTime = timer.elapsed();
            
            logWidget->append(QString("[%1] Bulk read completed: %2 bytes in %3ms").arg(timestamp, QString::number(fileData.size()), QString::number(readTime)));
            LOG_INFO("KDisplayFile_BulkRead: Bulk read completed: " + QString::number(fileData.size()) + " bytes in " + QString::number(readTime) + "ms");
            
                int nulIndex = fileData.indexOf('\0');
                if (nulIndex != -1) {
                    for (int i = 0; i < fileData.size(); ++i) {
                        if (fileData[i] == '\0') fileData[i] = ' ';
                    }
                }
                fileContentView->setUtf8Bytes(fileData.constData(), fileData.size());
            qint64 totalTime = timer.elapsed();

                if (fileContentView) {
                    LOG_INFO("KDisplayFile_BulkRead: Clearing loading message - file loaded successfully (readAll)");
                    fileContentView->setStyleSheet("");
                }
            
            logWidget->append(QString("[%1] Content set in ScintillaEdit. Total time: %2ms").arg(timestamp, QString::number(totalTime)));
            LOG_INFO("KDisplayFile_BulkRead: Content set in ScintillaEdit. Total time: " + QString::number(totalTime) + "ms");
            
            file.close();
            }
        } else {
            logWidget->append(QString("[%1] ERROR: Failed to open file for bulk read").arg(timestamp));
            LOG_ERROR("KDisplayFile_BulkRead: Failed to open file for bulk read");
        }
    } else {
        logWidget->append(QString("[%1] ERROR: No file loaded in LogDataWorker").arg(timestamp));
        LOG_ERROR("KDisplayFile_BulkRead: No file loaded in LogDataWorker");
    }
    
    logWidget->append(QString("[%1] === KDisplayFile_BulkRead COMPLETED ===").arg(timestamp));
    LOG_INFO("KDisplayFile_BulkRead: Completed");
}

void MainWindow::KDisplayFile_SequentialRead()
{
    QElapsedTimer timer;
    timer.start();
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    logWidget->append(QString("[%1] === KDisplayFile_SequentialRead STARTED ===").arg(timestamp));
    LOG_INFO("KDisplayFile_SequentialRead: Starting sequential read method");
    
    if (m_logDataWorker && m_logDataWorker->isFileLoaded()) {
        QString filePath = m_logDataWorker->getFilePath();
        logWidget->append(QString("[%1] File is loaded: %2").arg(timestamp, filePath));
        LOG_INFO("KDisplayFile_SequentialRead: File is loaded: " + filePath);
        
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            logWidget->append(QString("[%1] File opened successfully for sequential read").arg(timestamp));
            LOG_INFO("KDisplayFile_SequentialRead: File opened successfully for sequential read");
            
            // Get file size for progress tracking
            qint64 fileSize = file.size();
            logWidget->append(QString("[%1] File size: %2 bytes").arg(timestamp, QString::number(fileSize)));
            LOG_INFO("KDisplayFile_SequentialRead: File size: " + QString::number(fileSize) + " bytes");
            
            // Read file in chunks to avoid memory issues with large files
            QByteArray fileData;
            fileData.reserve(fileSize);  // Pre-allocate memory
            
            const qint64 chunkSize = 50 * 1024 * 1024;  // 50MB chunks
            qint64 totalRead = 0;
            int chunkCount = 0;
            
            logWidget->append(QString("[%1] Starting chunked read with %2MB chunks").arg(timestamp, QString::number(chunkSize / (1024 * 1024))));
            LOG_INFO("KDisplayFile_SequentialRead: Starting chunked read");
            
            while (!file.atEnd()) {
                QByteArray chunk = file.read(chunkSize);
                if (chunk.isEmpty()) {
                    break;
                }
                
                fileData.append(chunk);
                totalRead += chunk.size();
                chunkCount++;
                
                // Log progress every chunk (with 50MB chunks, this is reasonable)
                logWidget->append(QString("[%1] Read chunk %2: %3MB total").arg(timestamp, QString::number(chunkCount), QString::number(totalRead / (1024 * 1024))));
                LOG_DEBUG("KDisplayFile_SequentialRead: Read chunk " + QString::number(chunkCount) + ": " + QString::number(totalRead / (1024 * 1024)) + "MB total");
            }
            
            qint64 readTime = timer.elapsed();
            logWidget->append(QString("[%1] Chunked read completed: %2 bytes in %3 chunks, %4ms").arg(timestamp, QString::number(totalRead), QString::number(chunkCount), QString::number(readTime)));
            LOG_INFO("KDisplayFile_SequentialRead: Chunked read completed: " + QString::number(totalRead) + " bytes in " + QString::number(chunkCount) + " chunks, " + QString::number(readTime) + "ms");
            
            // Handle NUL characters efficiently
            QString fileContent = sanitizeForDisplay(fileData);
            qint64 sanitizeTime = timer.elapsed() - readTime;
            
            logWidget->append(QString("[%1] NUL sanitization completed in %2ms").arg(timestamp, QString::number(sanitizeTime)));
            LOG_INFO("KDisplayFile_SequentialRead: NUL sanitization completed in " + QString::number(sanitizeTime) + "ms");
            
            // Set content in ScintillaEdit
            fileContentView->setText(fileContent);
            qint64 totalTime = timer.elapsed();
            
            // Clear loading message since file is now loaded
            if (fileContentView) {
                LOG_INFO("KDisplayFile_SequentialRead: Clearing loading message - file loaded successfully");
                fileContentView->setStyleSheet(""); // Clear styling
            }
            
            logWidget->append(QString("[%1] Content set in ScintillaEdit. Total time: %2ms").arg(timestamp, QString::number(totalTime)));
            LOG_INFO("KDisplayFile_SequentialRead: Content set in ScintillaEdit. Total time: " + QString::number(totalTime) + "ms");
            
            file.close();
        } else {
            logWidget->append(QString("[%1] ERROR: Failed to open file for sequential read").arg(timestamp));
            LOG_ERROR("KDisplayFile_SequentialRead: Failed to open file for sequential read");
        }
    } else {
        logWidget->append(QString("[%1] ERROR: No file loaded in LogDataWorker").arg(timestamp));
        LOG_ERROR("KDisplayFile_SequentialRead: No file loaded in LogDataWorker");
    }
    
    logWidget->append(QString("[%1] === KDisplayFile_SequentialRead COMPLETED ===").arg(timestamp));
    LOG_INFO("KDisplayFile_SequentialRead: Completed");
}

void MainWindow::KDisplayFile_SequentialRead_Background()
{
    QElapsedTimer timer;
    timer.start();
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    logWidget->append(QString("[%1] === KDisplayFile_SequentialRead_Background STARTED ===").arg(timestamp));
    LOG_INFO("KDisplayFile_SequentialRead_Background: Starting background sequential read");
    
    // STEP 1: COMPLETE CLEANUP - FRESH START
    logWidget->append(QString("[%1] Performing complete cleanup before processing").arg(timestamp));
    LOG_INFO("KDisplayFile_SequentialRead_Background: Performing complete cleanup");
    
    // Clear current content
    if (fileContentView) {
        fileContentView->clearText();
        fileContentView->update();
    }
    
    // Clear any existing LogDataWorker
    if (m_logDataWorker) {
        delete m_logDataWorker;
        m_logDataWorker = nullptr;
        logWidget->append(QString("[%1] Cleared existing LogDataWorker").arg(timestamp));
        LOG_INFO("KDisplayFile_SequentialRead_Background: Cleared existing LogDataWorker");
    }
    
    // STEP 2: BROWSE FOR FILE
    QString filePath = QFileDialog::getOpenFileName(this, "Select File for Sequential Read", "", "All Files (*.*)");
    if (filePath.isEmpty()) {
        logWidget->append(QString("[%1] No file selected, aborting").arg(timestamp));
        LOG_INFO("KDisplayFile_SequentialRead_Background: No file selected, aborting");
        this->statusBar()->showMessage("No file selected", 2000);
        return;
    }
    
    logWidget->append(QString("[%1] File selected: %2").arg(timestamp, filePath));
    LOG_INFO("KDisplayFile_SequentialRead_Background: File selected: " + filePath);
    
    // STEP 3: CREATE NEW LogDataWorker
    m_logDataWorker = new LogDataWorker(this);
    
    logWidget->append(QString("[%1] Created new LogDataWorker").arg(timestamp));
    LOG_INFO("KDisplayFile_SequentialRead_Background: Created new LogDataWorker");
    
    // STEP 4: BACKGROUND PROCESSING
    QThread* workerThread = QThread::create([this, filePath, timer]() {
        QElapsedTimer threadTimer;
        threadTimer.start();
        
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        LOG_INFO("KDisplayFile_SequentialRead_Background: Worker thread started");
        
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            LOG_INFO("KDisplayFile_SequentialRead_Background: File opened successfully");
            
            qint64 fileSize = file.size();
            LOG_INFO("KDisplayFile_SequentialRead_Background: File size: " + QString::number(fileSize) + " bytes");
            
            // Read file in chunks
            QByteArray fileData;
            fileData.reserve(fileSize);
            
            const qint64 chunkSize = 50 * 1024 * 1024;  // 50MB chunks
            qint64 totalRead = 0;
            int chunkCount = 0;
            
            LOG_INFO("KDisplayFile_SequentialRead_Background: Starting chunked read with 50MB chunks");
            
            while (!file.atEnd()) {
                QByteArray chunk = file.read(chunkSize);
                if (chunk.isEmpty()) {
                    break;
                }
                
                fileData.append(chunk);
                totalRead += chunk.size();
                chunkCount++;
                
                LOG_DEBUG("KDisplayFile_SequentialRead_Background: Read chunk " + QString::number(chunkCount) + ": " + QString::number(totalRead / (1024 * 1024)) + "MB total");
            }
            
            qint64 readTime = threadTimer.elapsed();
            LOG_INFO("KDisplayFile_SequentialRead_Background: Chunked read completed: " + QString::number(totalRead) + " bytes in " + QString::number(chunkCount) + " chunks, " + QString::number(readTime) + "ms");
            
            // Handle NUL characters efficiently
            QString fileContent = sanitizeForDisplay(fileData);
            qint64 sanitizeTime = threadTimer.elapsed() - readTime;
            
            LOG_INFO("KDisplayFile_SequentialRead_Background: NUL sanitization completed in " + QString::number(sanitizeTime) + "ms");
            LOG_INFO("KDisplayFile_SequentialRead_Background: Total processing time: " + QString::number(threadTimer.elapsed()) + "ms");
            
            file.close();
            
            // Emit completion signal
            emit sequentialReadCompleted(fileContent);
            
        } else {
            LOG_ERROR("KDisplayFile_SequentialRead_Background: Failed to open file");
            emit sequentialReadError("Failed to open file for reading");
        }
    });
    
    // Note: Removed signal disconnections to prevent crashes
    
    // Connect signals for completion handling
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);
    
    connect(this, &MainWindow::sequentialReadCompleted, [this, timer](const QString& fileContent) {
                        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        logWidget->append(QString("[%1] Background processing completed, setting content in ScintillaEdit").arg(timestamp));
        LOG_INFO("KDisplayFile_SequentialRead_Background: Background processing completed");
        
        // Disable updates temporarily to prevent flashing
        fileContentView->setUpdatesEnabled(false);
        
        // Set content in ScintillaEdit (this must be done in main thread)
        fileContentView->setText(fileContent);
        
        // Re-enable updates and force a single repaint
        fileContentView->setUpdatesEnabled(true);
        fileContentView->repaint();
        
        qint64 totalTime = timer.elapsed();
        
        logWidget->append(QString("[%1] Content set in ScintillaEdit. Total time: %2ms").arg(timestamp, QString::number(totalTime)));
        LOG_INFO("KDisplayFile_SequentialRead_Background: Content set in ScintillaEdit. Total time: " + QString::number(totalTime) + "ms");
        
        logWidget->append(QString("[%1] === KDisplayFile_SequentialRead_Background COMPLETED ===").arg(timestamp));
        LOG_INFO("KDisplayFile_SequentialRead_Background: Completed");
        
        // Update status bar to show completion
        this->statusBar()->showMessage("File processing completed", 3000);
        
        // Cleanup: Clear LogDataWorker to free memory
        if (m_logDataWorker) {
            delete m_logDataWorker;
            m_logDataWorker = nullptr;
            logWidget->append(QString("[%1] Cleanup: Cleared LogDataWorker after processing").arg(timestamp));
            LOG_INFO("KDisplayFile_SequentialRead_Background: Cleanup: Cleared LogDataWorker after processing");
        }
    });
    
    connect(this, &MainWindow::sequentialReadError, [this, timer](const QString& error) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        logWidget->append(QString("[%1] ERROR: %2").arg(timestamp, error));
        LOG_ERROR("KDisplayFile_SequentialRead_Background: " + error);
        
        logWidget->append(QString("[%1] === KDisplayFile_SequentialRead_Background FAILED ===").arg(timestamp));
        LOG_INFO("KDisplayFile_SequentialRead_Background: Failed");
        
        // Update status bar to show error
        this->statusBar()->showMessage("File processing failed", 3000);
        
        // Cleanup: Clear LogDataWorker to free memory
        if (m_logDataWorker) {
            delete m_logDataWorker;
            m_logDataWorker = nullptr;
            logWidget->append(QString("[%1] Cleanup: Cleared LogDataWorker after error").arg(timestamp));
            LOG_INFO("KDisplayFile_SequentialRead_Background: Cleanup: Cleared LogDataWorker after error");
        }
    });
    
    // Start the worker thread
    workerThread->start();
    
    logWidget->append(QString("[%1] Background processing started").arg(timestamp));
    LOG_INFO("KDisplayFile_SequentialRead_Background: Background processing started");
    
    // Show processing indicator in status bar
    this->statusBar()->showMessage("Processing file in background...");
    logFunctionEnd("KDisplayFile_SequentialRead_Background");
}

void MainWindow::KDisplayFile_SequentialRead_Optimized()
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    logWidget->append(QString("[%1] === ULTRA SIMPLE ASYNC VERSION ===").arg(timestamp));
    LOG_INFO("KDisplayFile_SequentialRead_Optimized: ULTRA SIMPLE ASYNC VERSION");
    
    // STEP 1: NO SIGNAL DISCONNECTION (PREVENTS CRASHES)
    logWidget->append(QString("[%1] Skipping signal disconnection to prevent crashes").arg(timestamp));
    
    // STEP 2: FILE DIALOG
    logWidget->append(QString("[%1] About to open file dialog").arg(timestamp));
    QString filePath = QFileDialog::getOpenFileName(this, "Select File for Ultra Simple Async Read", "", "All Files (*.*)");
    
    if (filePath.isEmpty()) {
        logWidget->append(QString("[%1] No file selected").arg(timestamp));
            return;
        }
        
    logWidget->append(QString("[%1] File selected: %2").arg(timestamp, filePath));
    
    // STEP 3: MINIMAL CLEANUP
    logWidget->append(QString("[%1] Performing minimal cleanup").arg(timestamp));
    
    if (m_workerThread) {
        if (m_workerThread->isRunning()) {
            m_workerThread->quit();
            m_workerThread->wait(1000);
        }
        delete m_workerThread;
        m_workerThread = nullptr;
    }
    
    logWidget->append(QString("[%1] Cleanup completed").arg(timestamp));
    
    // STEP 4: CREATE SIMPLE WORKER THREAD
    logWidget->append(QString("[%1] Creating simple worker thread").arg(timestamp));
    
    m_workerThread = QThread::create([this, filePath, timestamp]() {
        // Simple file reading in background thread
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray fileData = file.readAll();
            file.close();
            
            // Simple sanitization
            fileData.replace('\0', ' ');
            QString content = QString::fromUtf8(fileData);
            
            // Emit the result using the existing signal
            emit this->sequentialReadCompleted(content);
        } else {
            emit this->sequentialReadError("Failed to open file for reading");
        }
    });
    
    if (!m_workerThread) {
        logWidget->append(QString("[%1] ERROR: Failed to create worker thread").arg(timestamp));
            return;
        }
        
    // STEP 5: CONNECT TO EXISTING SIGNALS (SIMPLE)
    // Note: Removed signal disconnections to prevent crashes
    
    connect(this, &MainWindow::sequentialReadCompleted, [this](const QString& fileContent) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        logWidget->append(QString("[%1] Background processing completed").arg(timestamp));
        
        if (fileContentView) {
            fileContentView->setText(fileContent);
        }
        
        logWidget->append(QString("[%1] === ULTRA SIMPLE ASYNC READ COMPLETED ===").arg(timestamp));
        this->statusBar()->showMessage("File processing completed", 3000);
    });
    
    connect(this, &MainWindow::sequentialReadError, [this](const QString& error) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        logWidget->append(QString("[%1] ERROR: %2").arg(timestamp, error));
        this->statusBar()->showMessage("File processing failed", 3000);
    });
    
    // STEP 6: START THE THREAD
    logWidget->append(QString("[%1] Starting worker thread").arg(timestamp));
    m_workerThread->start();
    
    logWidget->append(QString("[%1] Background processing started").arg(timestamp));
    this->statusBar()->showMessage("Processing file in background...");
    logFunctionEnd("KDisplayFile_SequentialRead_Optimized");
}



void MainWindow::cleanupThreadAndMemory()
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    logWidget->append(QString("[%1] Starting explicit memory cleanup").arg(timestamp));
    LOG_INFO("KDisplayFile_SequentialRead_Optimized: Starting explicit memory cleanup");
    
    // Cleanup worker thread (non-blocking)
    if (m_workerThread) {
        if (m_workerThread->isRunning()) {
            m_workerThread->quit();
            if (!m_workerThread->wait(100)) { // Short wait
                m_workerThread->terminate();
                m_workerThread->wait(100);
            }
        }
        delete m_workerThread;
        m_workerThread = nullptr;
        logWidget->append(QString("[%1] Cleared worker thread").arg(timestamp));
        LOG_INFO("KDisplayFile_SequentialRead_Optimized: Cleared worker thread");
    }
    
    // Cleanup timers
    if (m_cleanupTimer) {
        m_cleanupTimer->stop();
        m_cleanupTimer->deleteLater();
        m_cleanupTimer = nullptr;
    }
    
    if (m_progressTimer) {
        m_progressTimer->stop();
        m_progressTimer->deleteLater();
        m_progressTimer = nullptr;
    }
    
    // Clear reusable buffer
    m_reusableBuffer.clear();
    m_reusableBuffer.squeeze(); // Release memory
    
    // Clear content chunks
    m_contentChunks.clear();
    m_currentChunkIndex = 0;
    m_isProcessing = false;
    
    // Cleanup LogDataWorker
    if (m_logDataWorker) {
        delete m_logDataWorker;
        m_logDataWorker = nullptr;
        logWidget->append(QString("[%1] Cleared LogDataWorker").arg(timestamp));
        LOG_INFO("KDisplayFile_SequentialRead_Optimized: Cleared LogDataWorker");
    }
    
    // Force garbage collection
    QApplication::processEvents();
    
    logWidget->append(QString("[%1] Explicit memory cleanup completed").arg(timestamp));
    LOG_INFO("KDisplayFile_SequentialRead_Optimized: Explicit memory cleanup completed");
    logFunctionEnd("cleanupThreadAndMemory");
}

void MainWindow::KDisplayFile_SequentialRead_Sync()
{
    QElapsedTimer timer;
    timer.start();
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    logWidget->append(QString("[%1] === KDisplayFile_SequentialRead_Sync STARTED ===").arg(timestamp));
    LOG_INFO("KDisplayFile_SequentialRead_Sync: Starting synchronous sequential read");
    
    // STEP 1: BASIC CLEANUP - FRESH START
    logWidget->append(QString("[%1] Performing basic cleanup - fresh start").arg(timestamp));
    LOG_INFO("KDisplayFile_SequentialRead_Sync: Performing basic cleanup - fresh start");
    
    // Clear current content
    if (fileContentView) {
        fileContentView->clearText();
        fileContentView->update();
    }
    
    // Clear any existing LogDataWorker
    if (m_logDataWorker) {
        delete m_logDataWorker;
        m_logDataWorker = nullptr;
        logWidget->append(QString("[%1] Cleared existing LogDataWorker").arg(timestamp));
        LOG_INFO("KDisplayFile_SequentialRead_Sync: Cleared existing LogDataWorker");
    }
    
    // Reset processing state
    m_isProcessing = false;
    
    logWidget->append(QString("[%1] Basic cleanup finished").arg(timestamp));
    LOG_INFO("KDisplayFile_SequentialRead_Sync: Basic cleanup finished");
    
    // STEP 2: BROWSE FOR FILE
    QString filePath = QFileDialog::getOpenFileName(this, "Select File for Synchronous Sequential Read", "", "All Files (*.*)");
    if (filePath.isEmpty()) {
        logWidget->append(QString("[%1] No file selected, aborting").arg(timestamp));
        LOG_INFO("KDisplayFile_SequentialRead_Sync: No file selected, aborting");
        this->statusBar()->showMessage("No file selected", 2000);
            return;
        }
        
    logWidget->append(QString("[%1] File selected: %2").arg(timestamp, filePath));
    LOG_INFO("KDisplayFile_SequentialRead_Sync: File selected: " + filePath);
    
    // STEP 3: SYNCHRONOUS FILE PROCESSING (IN MAIN THREAD)
    logWidget->append(QString("[%1] Starting synchronous processing in main thread").arg(timestamp));
    LOG_INFO("KDisplayFile_SequentialRead_Sync: Starting synchronous processing in main thread");
    
    // Show processing indicator in status bar
    this->statusBar()->showMessage("Processing file synchronously...");
    
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        LOG_INFO("KDisplayFile_SequentialRead_Sync: File opened successfully");
        
        qint64 fileSize = file.size();
        LOG_INFO("KDisplayFile_SequentialRead_Sync: File size: " + QString::number(fileSize) + " bytes");
        
        // Use 50MB chunks as requested
        const qint64 chunkSize = 50 * 1024 * 1024; // 50MB chunks
        qint64 totalRead = 0;
        int chunkCount = 0;
        QString accumulatedContent;
        
        LOG_INFO("KDisplayFile_SequentialRead_Sync: Starting chunked read with 50MB chunks");
        
        while (!file.atEnd()) {
            // Read chunk directly
            QByteArray chunk = file.read(chunkSize);
            if (chunk.isEmpty()) {
                break;
            }
            
            totalRead += chunk.size();
            chunkCount++;
            
            // Process chunk immediately (sanitize and convert)
            QString sanitizedChunk = sanitizeForDisplay(chunk);
            accumulatedContent += sanitizedChunk;
            
            LOG_DEBUG("KDisplayFile_SequentialRead_Sync: Read chunk " + QString::number(chunkCount) + ": " + QString::number(totalRead / (1024 * 1024)) + "MB total");
            
            // Update status bar to show progress
            this->statusBar()->showMessage(QString("Processing: %1MB read...").arg(totalRead / (1024 * 1024)));
            
            // Process events to keep UI responsive
            QApplication::processEvents();
        }
        
        qint64 readTime = timer.elapsed();
        LOG_INFO("KDisplayFile_SequentialRead_Sync: Chunked read completed: " + QString::number(totalRead) + " bytes in " + QString::number(chunkCount) + " chunks, " + QString::number(readTime) + "ms");
        
        file.close();
        
        // STEP 4: SET CONTENT IN SCINTILLAEDIT (SYNCHRONOUS)
        logWidget->append(QString("[%1] Setting content in ScintillaEdit synchronously").arg(timestamp));
        LOG_INFO("KDisplayFile_SequentialRead_Sync: Setting content in ScintillaEdit synchronously");
        
        // Disable updates temporarily to prevent flashing
        fileContentView->setUpdatesEnabled(false);
        
        // Set content in ScintillaEdit (single bulk update)
        fileContentView->setText(accumulatedContent);
        
        // Re-enable updates and force a single repaint
        fileContentView->setUpdatesEnabled(true);
        fileContentView->repaint();
        
        qint64 totalTime = timer.elapsed();
        logWidget->append(QString("[%1] Content set in ScintillaEdit. Total time: %2ms").arg(timestamp, QString::number(totalTime)));
        LOG_INFO("KDisplayFile_SequentialRead_Sync: Content set in ScintillaEdit. Total time: " + QString::number(totalTime) + "ms");
        
        logWidget->append(QString("[%1] === KDisplayFile_SequentialRead_Sync COMPLETED ===").arg(timestamp));
        LOG_INFO("KDisplayFile_SequentialRead_Sync: Completed");
        
        // Update status bar to show completion
        this->statusBar()->showMessage("File processing completed", 3000);
        
    } else {
        LOG_ERROR("KDisplayFile_SequentialRead_Sync: Failed to open file");
        logWidget->append(QString("[%1] ERROR: Failed to open file for reading").arg(timestamp));
        LOG_ERROR("KDisplayFile_SequentialRead_Sync: Failed to open file for reading");
        
        logWidget->append(QString("[%1] === KDisplayFile_SequentialRead_Sync FAILED ===").arg(timestamp));
        LOG_INFO("KDisplayFile_SequentialRead_Sync: Failed");
        
        // Update status bar to show error
        this->statusBar()->showMessage("File processing failed", 3000);
    }
    logFunctionEnd("KDisplayFile_SequentialRead_Sync");
}


void MainWindow::updateCacheStatusDisplay()
{
    if (cacheStatusLabel) {
        if (m_keepFilesInCache) {
            cacheStatusLabel->setText("CACHED");
            cacheStatusLabel->setStyleSheet("QLabel { color: #28a745; font-size: 10px; padding: 2px 6px; border: 1px solid #28a745; border-radius: 3px; background-color: #d4edda; }");
            cacheStatusLabel->setToolTip("Cache mode: ENABLED - Files are kept in memory for faster navigation");
        } else {
            cacheStatusLabel->setText("UNCACHED");
            cacheStatusLabel->setStyleSheet("QLabel { color: #6c757d; font-size: 10px; padding: 2px 6px; border: 1px solid #dee2e6; border-radius: 3px; background-color: #f8f9fa; }");
            cacheStatusLabel->setToolTip("Cache mode: DISABLED - Files are closed after viewing");
        }
    }
}

void MainWindow::verifyCacheIntegrity()
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    int cacheSize = m_fileCache.size();
    
    LOG_INFO("verifyCacheIntegrity: Checking " + QString::number(cacheSize) + " cached files");
    logWidget->append(QString("[%1] CACHE INTEGRITY CHECK - Verifying %2 cached files").arg(timestamp, QString::number(cacheSize)));
    
    for (auto it = m_fileCache.begin(); it != m_fileCache.end(); ++it) {
        QString filePath = it.key();
        LogDataWorker* worker = it.value();
        
        if (worker && worker->isFileLoaded()) {
            int expectedLines = worker->getTotalLines();
            QString fileContent = worker->loadEntireFileContent();
            int actualLines = fileContent.count('\n') + 1;
            
            LOG_INFO("verifyCacheIntegrity: File: " + filePath + " - Expected: " + QString::number(expectedLines) + " lines, Actual: " + QString::number(actualLines) + " lines");
            logWidget->append(QString("[%1] CACHE INTEGRITY - File: %2 - Expected: %3 lines, Actual: %4 lines").arg(timestamp, filePath, QString::number(expectedLines), QString::number(actualLines)));
            
            if (abs(expectedLines - actualLines) > 10) {
                LOG_WARNING("verifyCacheIntegrity: File content mismatch detected: " + filePath);
                logWidget->append(QString("[%1] CACHE INTEGRITY WARNING - Content mismatch in file: %2").arg(timestamp, filePath));
            }
        } else {
            LOG_ERROR("verifyCacheIntegrity: Invalid worker for file: " + filePath);
            logWidget->append(QString("[%1] CACHE INTEGRITY ERROR - Invalid worker for file: %2").arg(timestamp, filePath));
        }
    }
    LOG_INFO("verifyCacheIntegrity: Cache integrity check completed");
    logFunctionEnd("verifyCacheIntegrity");
}

void MainWindow::performFastInFileSearch(const QString &pattern, bool caseSensitive, bool wholeWord, bool useRegex)
{
    logFunctionStart("performFastInFileSearch");
    QElapsedTimer timer;
    timer.start();
    
    LOG_INFO("performFastInFileSearch: Starting fast in-file search");
    LOG_INFO("performFastInFileSearch: Pattern: " + pattern);
    LOG_INFO("performFastInFileSearch: Case sensitive: " + QString(caseSensitive ? "Yes" : "No"));
    LOG_INFO("performFastInFileSearch: Whole word: " + QString(wholeWord ? "Yes" : "No"));
    LOG_INFO("performFastInFileSearch: Use regex: " + QString(useRegex ? "Yes" : "No"));
    
    if (!fileContentView) {
        LOG_WARNING("performFastInFileSearch: No file content view available");
        return;
    }
    
    // Perform the fast search using Scintilla's built-in mechanism
    fileContentView->fastSearchAndHighlight(pattern, caseSensitive, wholeWord, useRegex);
    
    qint64 searchTime = timer.elapsed();
    LOG_INFO("performFastInFileSearch: Fast in-file search completed in " + QString::number(searchTime) + "ms");
    
    // Update status bar
    statusBar()->showMessage(QString("Fast search completed in %1ms").arg(searchTime), 2000);
    
    logFunctionEnd("performFastInFileSearch");
}

void MainWindow::showHighlightDialog()
{
    logFunctionStart("showHighlightDialog");
    LOG_INFO("showHighlightDialog: Opening highlight configuration dialog");
    
    HighlightDialog dialog(this);
    
    // Pass the configuration dialog reference if it exists
    if (m_configDialog) {
        dialog.setConfigurationDialog(m_configDialog);
        LOG_INFO("showHighlightDialog: Configuration dialog reference passed to HighlightDialog");
    } else {
        LOG_WARNING("showHighlightDialog: No configuration dialog available, using default engine settings");
    }
    
    // Set Pattern 0 with current search pattern from main search field
    QString currentPattern = patternEdit->text().trimmed();
    if (!currentPattern.isEmpty()) {
        dialog.setRGSearchPattern(currentPattern);
        LOG_INFO("showHighlightDialog: Set Pattern 0 with current search pattern: '" + currentPattern + "'");
    }
    
    // Set current rules if any exist
    if (!m_extraHighlightRules.isEmpty()) {
        dialog.setHighlightRules(m_extraHighlightRules);
    }
    
    // Show the dialog
    if (dialog.exec() == QDialog::Accepted) {
        // Get the highlight rules and global options from the dialog
        QList<HighlightRule> newRules = dialog.getHighlightRules();
        bool newCaseSensitive = dialog.isCaseSensitive();
        bool newHighlightSentence = dialog.isHighlightSentence();
        bool newUseRGHighlight = dialog.isUseRGHighlight();
        bool newUseQtHighlight = dialog.isUseQtHighlight();
        
        // Check if anything changed
        bool hasChanges = false;
        
        if (newRules.size() != m_extraHighlightRules.size()) {
            hasChanges = true;
            LOG_INFO("showHighlightDialog: Rule count changed from " + QString::number(m_extraHighlightRules.size()) + 
                     " to " + QString::number(newRules.size()));
        } else {
            for (int i = 0; i < newRules.size(); ++i) {
                if (i < m_extraHighlightRules.size()) {
                    const HighlightRule &oldRule = m_extraHighlightRules[i];
                    const HighlightRule &newRule = newRules[i];
                    if (oldRule.pattern != newRule.pattern || 
                        oldRule.color != newRule.color || 
                        oldRule.enabled != newRule.enabled) {
                        hasChanges = true;
                        LOG_INFO("showHighlightDialog: Rule " + QString::number(i + 1) + " changed");
                        break;
                    }
                } else {
                    hasChanges = true;
                    LOG_INFO("showHighlightDialog: New rule added at position " + QString::number(i + 1));
                    break;
                }
            }
        }
        
        bool newUseViewportHighlight = dialog.isUseViewportHighlight();
        
        if (newCaseSensitive != m_highlightCaseSensitive || 
            newHighlightSentence != m_highlightSentence ||
            newUseRGHighlight != m_useRGHighlight ||
            newUseQtHighlight != m_useQtHighlight ||
            newUseViewportHighlight != m_useViewportHighlight) {
            hasChanges = true;
            LOG_INFO("showHighlightDialog: Global options changed");
        }
        
        // Update the rules
        m_extraHighlightRules = newRules;
        m_highlightCaseSensitive = newCaseSensitive;
        m_highlightSentence = newHighlightSentence;
        m_useRGHighlight = newUseRGHighlight;
        m_useQtHighlight = newUseQtHighlight;
        m_useViewportHighlight = newUseViewportHighlight;
        
        LOG_INFO("showHighlightDialog: Dialog accepted with " + QString::number(m_extraHighlightRules.size()) + " rules");
        LOG_INFO("showHighlightDialog: Global options - Case Sensitive: " + QString(m_highlightCaseSensitive ? "Yes" : "No") + 
                 ", Highlight Sentence: " + QString(m_highlightSentence ? "Yes" : "No") + 
                 ", Use RG Highlight: " + QString(m_useRGHighlight ? "Yes" : "No") + 
                 ", Use Qt Highlight: " + QString(m_useQtHighlight ? "Yes" : "No") +
                 ", Use Viewport Highlight: " + QString(m_useViewportHighlight ? "Yes" : "No"));
        
        // Apply the highlights to the current file if there were changes and a file is loaded
        if (hasChanges && fileContentView && !fileContentView->getText().isEmpty()) {
            LOG_INFO("showHighlightDialog: Changes detected, applying highlights to current file");
 /*           if (m_useRGHighlight) {
                // Get the current file path
                QString currentFilePath = m_currentFilePath4NonCached;
                if (currentFilePath.isEmpty()) {
                    // Try to get from cache mode
                    for (const QString &cachedPath : m_fileAlreadyOpen4CacheMode) {
                        currentFilePath = cachedPath;
                        break; // Use the first cached file
                    }
                }
                if (!currentFilePath.isEmpty()) {
                    applyExtraHighlightsWithRG(currentFilePath);
                } else {
                    LOG_WARNING("showHighlightDialog: No file path available for RG highlighting");
                    m_kSearch->applyExtraHighlights(); // Fallback to Qt highlighting
                }
            } else {
                m_kSearch->applyExtraHighlights();
            }
            */



            m_kSearch->applyExtraHighlights();



            

            statusBar()->showMessage(QString("Applied %1 highlight rules").arg(m_extraHighlightRules.size()), 2000);
        } else if (hasChanges) {
            LOG_INFO("showHighlightDialog: Changes detected but no file is currently loaded");
            statusBar()->showMessage("Highlight rules saved (no file to apply to)", 2000);
        } else {
            LOG_INFO("showHighlightDialog: No changes detected");
            statusBar()->showMessage("No changes to highlight rules", 2000);
        }
    } else {
        LOG_INFO("showHighlightDialog: Dialog cancelled");
    }
    
    logFunctionEnd("showHighlightDialog");
}

void MainWindow::showEngineeringDialog()
{
    logFunctionStart("showEngineeringDialog");
    LOG_INFO("showEngineeringDialog: Opening engineering tools dialog");
    
    EngineeringDialog dialog(this, this);
    dialog.exec();
    
    LOG_INFO("showEngineeringDialog: Engineering dialog closed");
    logFunctionEnd("showEngineeringDialog");
}

void MainWindow::showSearchParamsDialog()
{
    logFunctionStart("showSearchParamsDialog");
    LOG_INFO("showSearchParamsDialog: Opening search parameters dialog");
    
    // Create and show the Search Params dialog (formerly RG Search)
    RGSearchDialog dialog(this);
    dialog.setWindowTitle("Search Parameters"); // Renamed title
    dialog.setLogWidget(logWidget);
    
    // Set current pattern and path from main window fields
    dialog.setCurrentPattern(patternEdit->text().trimmed());
    dialog.setCurrentPath(pathEdit->text().trimmed());
    
    // Set the main window reference in the KSearchBun instance
    dialog.kSearchBun->setMainWindow(this);
    
    if (dialog.exec() == QDialog::Accepted) {
        // Get search parameters from dialog and update main window fields
        RGSearchParams params = dialog.getSearchParams();
        
        // Update main window fields with values from dialog
        patternEdit->setText(params.pattern);
        pathEdit->setText(params.path);
        
        // Store other parameters for search execution
        m_rgSearchHighlightColor = params.highlight_color;
        m_searchResultHighlightColor = params.highlight_color;  // Use search params color for result line highlighting
        m_keepFilesInCache = params.keep_files_in_cache;
        
        // Update persistent search parameters in KSearchBun with dialog values
        if (m_kSearch && m_kSearch->getSearchBun()) {
            m_kSearch->getSearchBun()->updateSearchParams(params);
            LOG_INFO("showSearchParamsDialog: Updated persistent search parameters in KSearchBun with dialog values");
        }
        
        LOG_INFO("showSearchParamsDialog: Updated main window fields with dialog values");
        LOG_INFO("  Pattern: '" + params.pattern + "'");
        LOG_INFO("  Path: '" + params.path + "'");
        
        statusBar()->showMessage("Search parameters updated", 2000);
    } else {
        LOG_INFO("showSearchParamsDialog: Dialog cancelled");
    }
    
    logFunctionEnd("showSearchParamsDialog");
}

void MainWindow::browseForPath()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory", pathEdit->text());
    if (!dir.isEmpty()) {
        pathEdit->setText(dir);
        // Add the selected path to history for auto-complete
        addToPathHistory(dir);
        LOG_INFO("browseForPath: Selected directory: " + dir);
    }
}


void MainWindow::loadSearchHistory()
{
    loadHistoryFromFile();
}

void MainWindow::saveSearchHistory()
{
    saveHistoryToFile();
}

void MainWindow::updateSearchFieldsFromHistory()
{
    // Always update pattern field with first item from history if available
    if (!m_searchPatternHistory.isEmpty()) {
        patternEdit->setText(m_searchPatternHistory.first());
        LOG_INFO("updateSearchFieldsFromHistory: Set pattern field to: " + m_searchPatternHistory.first());
    }
    
    // Always update path field with first item from history if available
    if (!m_searchPathHistory.isEmpty()) {
        pathEdit->setText(m_searchPathHistory.first());
        LOG_INFO("updateSearchFieldsFromHistory: Set path field to: " + m_searchPathHistory.first());
    }
    
    // Update auto-complete suggestions
    updatePatternCompleter();
    updatePathCompleter();
}

void MainWindow::showConfigurationDialog()
{
    // Create and store the configuration dialog instance
    m_configDialog = new ConfigurationDialog(this);
    
    // Set the MainWindow reference so the dialog can apply settings immediately
    m_configDialog->setMainWindow(this);
    
    if (m_configDialog->exec() == QDialog::Accepted) {
        LOG_INFO("Configuration dialog accepted - settings saved and applied");
    } else {
        LOG_INFO("Configuration dialog cancelled");
    }
    // Keep the dialog instance for HighlightDialog to use
    // It will be cleaned up when MainWindow is destroyed
}


void MainWindow::applyExtraHighlightsWithRG(const QString& filePath)
{
    logFunctionStart("applyExtraHighlightsWithRG");
    
    // Activate highlight extra indicator
    
    QElapsedTimer timer;
    timer.start();
    
    LOG_INFO("applyExtraHighlightsWithRG: Starting RG highlighting process");
    LOG_INFO("applyExtraHighlightsWithRG: File path: " + filePath);
    
    if (!fileContentView) {
        LOG_WARNING("applyExtraHighlightsWithRG: No file content view available");
        return;
    }
    
    // 1. Check if new file - use static variables to track
    static QString lastFilePath;
    static QList<HighlightRule> lastRules;
    
    bool isNewFile = (lastFilePath != filePath);
    bool rulesChanged = (lastRules.size() != m_extraHighlightRules.size());
    
    // Check if rules content changed
    if (!rulesChanged && lastRules.size() == m_extraHighlightRules.size()) {
        for (int i = 0; i < lastRules.size(); ++i) {
            if (lastRules[i].pattern != m_extraHighlightRules[i].pattern ||
                lastRules[i].color != m_extraHighlightRules[i].color ||
                lastRules[i].enabled != m_extraHighlightRules[i].enabled) {
                rulesChanged = true;
                break;
            }
        }
    }
    
    LOG_INFO("applyExtraHighlightsWithRG: File check - New file: " + QString(isNewFile ? "Yes" : "No"));
    LOG_INFO("applyExtraHighlightsWithRG: Rules check - Rules changed: " + QString(rulesChanged ? "Yes" : "No"));
    
    if (!isNewFile && !rulesChanged) {
        LOG_INFO("applyExtraHighlightsWithRG: Same file and rules, skipping highlighting");
        return;
    }
    
    // 2. Clear previous highlights if rules changed
    if (rulesChanged) {
        LOG_INFO("applyExtraHighlightsWithRG: Rules changed, clearing previous highlights");
        fileContentView->clearExtraHighlights();
    }
    
    // 3. Collect all valid rules and create big regex expression
    QList<HighlightRule> validRules;
    QStringList patterns;
    
    for (int i = 0; i < m_extraHighlightRules.size(); ++i) {
        const HighlightRule &rule = m_extraHighlightRules[i];
        if (rule.enabled && !rule.pattern.isEmpty()) {
            validRules.append(rule);
            patterns.append(rule.pattern);
            LOG_INFO("applyExtraHighlightsWithRG: Valid rule " + QString::number(i + 1) + 
                     " - Pattern: '" + rule.pattern + "', Color: " + rule.color.name());
        }
    }
    
    if (validRules.isEmpty()) {
        LOG_INFO("applyExtraHighlightsWithRG: No valid rules found");
        return;
    }
    
    // Create big regex expression with OR logic
    QString bigPattern = patterns.join("|");
    LOG_INFO("applyExtraHighlightsWithRG: Big regex pattern: '" + bigPattern + "'");
    
    // 4. Perform RG command with hardcoded parameters
    QString rgPath = "lib\\rg.exe";
    QStringList arguments;
    arguments << "-a" << "-n" << "--threads" << "0" << "--mmap" << "--column" << "--byte-offset" 
             << "--stats" << "--only-matching" << "--heading" << "-S";
    
    // Add case sensitivity based on dialog setting
    if (!m_highlightCaseSensitive) {
        arguments << "-i";
    }
    
    arguments << "-e" << bigPattern << filePath;
    
    QString command = rgPath + " " + arguments.join(" ");
    LOG_INFO("applyExtraHighlightsWithRG: Executing RG command: " + command);
    
    // Execute ripgrep
    QProcess process;
    process.start(rgPath, arguments);
    process.waitForFinished();
    
    if (process.exitCode() != 0) {
        LOG_ERROR("applyExtraHighlightsWithRG: Ripgrep failed with exit code " + QString::number(process.exitCode()));
        LOG_ERROR("applyExtraHighlightsWithRG: Error output: " + QString::fromUtf8(process.readAllStandardError()));
        return;
    }
    
    QString output = QString::fromUtf8(process.readAllStandardOutput());
    LOG_INFO("applyExtraHighlightsWithRG: RG output length: " + QString::number(output.length()) + " characters");
    
    // 5. Decode output format: line_number:offset_from_start_of_line:offset_from_start_of_file:word_found
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    LOG_INFO("applyExtraHighlightsWithRG: Parsing " + QString::number(lines.size()) + " result lines");
    
    // Configure indicators for each rule
    for (int i = 0; i < validRules.size(); ++i) {
        int indicatorIndex = 2 + i;
        fileContentView->send(SCI_SETINDICATORCURRENT, indicatorIndex);
        fileContentView->send(SCI_INDICSETSTYLE, indicatorIndex, INDIC_FULLBOX);
        
        // Convert QColor to Scintilla BGR format
        const QColor &color = validRules[i].color;
        int r = color.red();
        int g = color.green();
        int b = color.blue();
        int scintillaColor = (b << 16) | (g << 8) | r;
        
        fileContentView->send(SCI_INDICSETFORE, indicatorIndex, scintillaColor);
        fileContentView->send(SCI_INDICSETALPHA, indicatorIndex, 100);
        fileContentView->send(SCI_INDICSETUNDER, indicatorIndex, true);
        
        LOG_INFO("applyExtraHighlightsWithRG: Configured indicator " + QString::number(indicatorIndex) + 
                 " for rule " + QString::number(i + 1) + " with color: " + color.name() + 
                 " (RGB: " + QString::number(r) + "," + QString::number(g) + "," + QString::number(b) + 
                 ") -> Scintilla BGR: " + QString::number(scintillaColor));
    }
    
    int totalHighlights = 0;
    
    // 6. Manual highlight the results using Qt regex to find matches
    for (const QString &line : lines) {
        // Skip header lines (they start with "File:")
        if (line.startsWith("File:")) {
            continue;
        }
        
        // Parse format: line_number:offset_from_start_of_line:offset_from_start_of_file:word_found
        QStringList parts = line.split(':');
        if (parts.size() >= 4) {
            bool ok1, ok2, ok3;
            int lineNumber = parts[0].toInt(&ok1);
            int offsetFromLine = parts[1].toInt(&ok2);
            int offsetFromFile = parts[2].toInt(&ok3);
            QString matchedWord = parts[3];

            if (ok1 && ok2 && ok3) {
                LOG_INFO("applyExtraHighlightsWithRG: Parsed result - Line: " + QString::number(lineNumber) + 
                         ", Offset from line: " + QString::number(offsetFromLine) + 
                         ", Offset from file: " + QString::number(offsetFromFile) + 
                         ", Word: '" + matchedWord + "'");
                
                // Get the size of the matchedWord
                int wordSize = matchedWord.length();
                LOG_INFO("applyExtraHighlightsWithRG: Word size: " + QString::number(wordSize));
                
                // Get the color of the matchedWord from the validRules
                QColor finalHighlightColor = Qt::yellow; // Default color
                int matchedRuleIndex = -1;
                
                for (int i = 0; i < validRules.size(); ++i) {
                    QRegularExpression ruleRegex(validRules[i].pattern);
                    QRegularExpressionMatch match = ruleRegex.match(matchedWord);
                    if (match.hasMatch()) {
                        matchedRuleIndex = i;
                        finalHighlightColor = validRules[i].color;
                        LOG_INFO("applyExtraHighlightsWithRG: Word '" + matchedWord + "' matched rule " + QString::number(i + 1) + 
                                 " (pattern: '" + validRules[i].pattern + "', color: " + finalHighlightColor.name() + ")");
                        break;
                    }
                }
                
                if (matchedRuleIndex >= 0) {
                
                // Use Scintilla indicators directly to avoid clearing previous highlights
                int indicatorIndex = 2 + matchedRuleIndex; // Use different indicator for each rule
                fileContentView->send(SCI_SETINDICATORCURRENT, indicatorIndex);
                fileContentView->send(SCI_INDICSETSTYLE, indicatorIndex, INDIC_FULLBOX);
                
                // Convert QColor to Scintilla BGR format
                int r = finalHighlightColor.red();
                int g = finalHighlightColor.green();
                int b = finalHighlightColor.blue();
                int scintillaColor = (b << 16) | (g << 8) | r;
                
                fileContentView->send(SCI_INDICSETFORE, indicatorIndex, scintillaColor);
                fileContentView->send(SCI_INDICSETALPHA, indicatorIndex, 100);
                fileContentView->send(SCI_INDICSETUNDER, indicatorIndex, true);
                
                if (m_highlightSentence) {
                    // Highlight the entire line
                    int lineStart = fileContentView->send(SCI_POSITIONFROMLINE, lineNumber - 1);
                    int lineEnd = fileContentView->send(SCI_GETLINEENDPOSITION, lineNumber - 1);
                    fileContentView->send(SCI_INDICATORFILLRANGE, lineStart, lineEnd - lineStart);
                    
                    LOG_INFO("applyExtraHighlightsWithRG: Highlighted entire line " + QString::number(lineNumber) + 
                             " with color: " + finalHighlightColor.name() + " (word: '" + matchedWord + "', size: " + QString::number(wordSize) + ")");
                } else {
                    // Highlight just the word
                    int wordStart = offsetFromFile;
                    int wordEnd = offsetFromFile + wordSize;
                    fileContentView->send(SCI_INDICATORFILLRANGE, wordStart, wordEnd - wordStart);
                    
                                         LOG_INFO("applyExtraHighlightsWithRG: Highlighted word at position " + QString::number(wordStart) + 
                              " to " + QString::number(wordEnd) + " with color: " + finalHighlightColor.name() + 
                              " (word: '" + matchedWord + "', size: " + QString::number(wordSize) + ")");
                 }
                 
                 totalHighlights++;
                } else {
                    LOG_WARNING("applyExtraHighlightsWithRG: Could not determine which rule matched word: '" + matchedWord + "'");
                }
            } else {
                LOG_WARNING("applyExtraHighlightsWithRG: Failed to parse line: '" + line + "'");
            }
        } else {
            LOG_WARNING("applyExtraHighlightsWithRG: Invalid line format: '" + line + "'");
        }
    }
    
    // Update static variables for next check
    lastFilePath = filePath;
    lastRules = m_extraHighlightRules;
    
    qint64 highlightTime = timer.elapsed();
    LOG_INFO("applyExtraHighlightsWithRG: RG highlighting completed in " + QString::number(highlightTime) + "ms");
    LOG_INFO("applyExtraHighlightsWithRG: Total highlights applied: " + QString::number(totalHighlights));
    
    // ===== BACKGROUND HIGHLIGHTING SETUP =====
    // Check if background highlighting should be started
    if (fileContentView && !fileContentView->isFullyHighlighted()) {
        // Get background highlighting settings from Configuration group
        QSettings settings("app.ini", QSettings::IniFormat);
        settings.beginGroup("Configuration");
        
        bool backgroundEnabled = settings.value("BackgroundHighlighting", false).toBool();
        QString chunkMode = settings.value("ChunkMode", "Lines").toString();
        QString chunkSizeText = settings.value("ChunkSize", "1000 lines").toString();
        QString durationText = settings.value("Duration", "50ms").toString();
        QString idleDelayText = settings.value("IdleDelay", "1 second").toString();
        QString performanceMode = settings.value("PerformanceMode", "Balanced").toString();
        
        settings.endGroup();
        
        // Debug logging
        LOG_INFO("applyExtraHighlightsWithRG: Background highlighting setting read: " + QString(backgroundEnabled ? "ENABLED" : "DISABLED"));
        LOG_INFO("applyExtraHighlightsWithRG: Background highlighting will " + QString(backgroundEnabled ? "START" : "NOT START"));
        
        // Parse chunk size from text
        int chunkSize = 1000; // default
        if (chunkSizeText.contains("100 lines")) chunkSize = 100;
        else if (chunkSizeText.contains("500 lines")) chunkSize = 500;
        else if (chunkSizeText.contains("1000 lines")) chunkSize = 1000;
        else if (chunkSizeText.contains("2000 lines")) chunkSize = 2000;
        else if (chunkSizeText.contains("5000 lines")) chunkSize = 5000;
        else if (chunkSizeText.contains("10000 lines")) chunkSize = 10000;
        else if (chunkSizeText.contains("20000 lines")) chunkSize = 20000;
        else if (chunkSizeText.contains("50000 lines")) chunkSize = 50000;
        else if (chunkSizeText.contains("100000 lines")) chunkSize = 100000;
        else if (chunkSizeText.contains("Custom lines")) {
            settings.beginGroup("Configuration");
            chunkSize = settings.value("CustomLines", "1000").toInt();
            settings.endGroup();
        }
        
        // Parse duration from text
        int duration = 50; // default
        if (durationText.contains("10ms")) duration = 10;
        else if (durationText.contains("50ms")) duration = 50;
        else if (durationText.contains("100ms")) duration = 100;
        else if (durationText.contains("500ms")) duration = 500;
        else if (durationText.contains("Custom ms")) {
            settings.beginGroup("Configuration");
            duration = settings.value("CustomDuration", "50").toInt();
            settings.endGroup();
        }
        
        // Parse idle delay from text
        int idleDelay = 1000; // default
        if (idleDelayText.contains("0.05 seconds")) idleDelay = 50;
        else if (idleDelayText.contains("0.1 seconds")) idleDelay = 100;
        else if (idleDelayText.contains("0.5 seconds")) idleDelay = 500;
        else if (idleDelayText.contains("1 second")) idleDelay = 1000;
        else if (idleDelayText.contains("2 seconds")) idleDelay = 2000;
        else if (idleDelayText.contains("5 seconds")) idleDelay = 5000;
        
        if (backgroundEnabled) {
            LOG_INFO("applyExtraHighlightsWithRG: Starting background highlighting - Mode: " + chunkMode + 
                     ", Chunk Size: " + QString::number(chunkSize) + ", Duration: " + QString::number(duration) + "ms" +
                     ", Idle Delay: " + QString::number(idleDelay) + "ms, Performance: " + performanceMode);
            
            fileContentView->startBackgroundHighlighting(m_extraHighlightRules, 
                                                        m_highlightCaseSensitive, 
                                                        m_highlightSentence, 
                                                        chunkSize, 
                                                        idleDelay,
                                                        performanceMode,
                                                        chunkMode,
                                                        duration);
        }
    }
    
    // Deactivate highlight extra indicator
    
    logFunctionEnd("applyExtraHighlightsWithRG");
}

void MainWindow::applyBackgroundHighlightingSettingsFromConfig(bool backgroundEnabled, bool progressEnabled)
{
    logFunctionStart("applyBackgroundHighlightingSettingsFromConfig");
    
    LOG_INFO("applyBackgroundHighlightingSettingsFromConfig: Background enabled: " + QString(backgroundEnabled ? "YES" : "NO") + 
             ", Progress enabled: " + QString(progressEnabled ? "YES" : "NO"));
    
    if (!fileContentView) {
        LOG_WARNING("applyBackgroundHighlightingSettingsFromConfig: No file content view available");
        return;
    }
    
    // Stop current background highlighting if it's running
    fileContentView->stopBackgroundHighlighting();
    LOG_INFO("applyBackgroundHighlightingSettingsFromConfig: Stopped current background highlighting");
    
    // Update the lamp immediately based on new settings
    if (backgroundEnabled) {
        // If background highlighting is enabled, show the lamp with current progress
        int currentProgress = fileContentView->getBackgroundProgress();
        setBackgroundHighlightLamp(true, currentProgress);
    } else {
        // If background highlighting is disabled, turn off the lamp
        setBackgroundHighlightLamp(false, 0);
    }
    
    // If background highlighting is enabled and we have a file loaded, restart it with new settings
    if (backgroundEnabled && !fileContentView->getText().isEmpty() && !m_extraHighlightRules.isEmpty()) {
        // Get current settings from Configuration group
        QSettings settings("app.ini", QSettings::IniFormat);
        settings.beginGroup("Configuration");
        
        QString chunkMode = settings.value("ChunkMode", "Lines").toString();
        QString chunkSizeText = settings.value("ChunkSize", "1000 lines").toString();
        QString durationText = settings.value("Duration", "50ms").toString();
        QString idleDelayText = settings.value("IdleDelay", "1 second").toString();
        QString performanceMode = settings.value("PerformanceMode", "Balanced").toString();
        
        settings.endGroup();
        
        // Parse chunk size from text
        int chunkSize = 1000; // default
        if (chunkSizeText.contains("100 lines")) chunkSize = 100;
        else if (chunkSizeText.contains("500 lines")) chunkSize = 500;
        else if (chunkSizeText.contains("1000 lines")) chunkSize = 1000;
        else if (chunkSizeText.contains("2000 lines")) chunkSize = 2000;
        else if (chunkSizeText.contains("5000 lines")) chunkSize = 5000;
        else if (chunkSizeText.contains("10000 lines")) chunkSize = 10000;
        else if (chunkSizeText.contains("20000 lines")) chunkSize = 20000;
        else if (chunkSizeText.contains("50000 lines")) chunkSize = 50000;
        else if (chunkSizeText.contains("100000 lines")) chunkSize = 100000;
        else if (chunkSizeText.contains("Custom lines")) {
            settings.beginGroup("Configuration");
            chunkSize = settings.value("CustomLines", "1000").toInt();
            settings.endGroup();
        }
        
        // Parse duration from text
        int duration = 50; // default
        if (durationText.contains("10ms")) duration = 10;
        else if (durationText.contains("50ms")) duration = 50;
        else if (durationText.contains("100ms")) duration = 100;
        else if (durationText.contains("500ms")) duration = 500;
        else if (durationText.contains("Custom ms")) {
            settings.beginGroup("Configuration");
            duration = settings.value("CustomDuration", "50").toInt();
            settings.endGroup();
        }
        
        // Parse idle delay from text
        int idleDelay = 1000; // default
        if (idleDelayText.contains("0.05 seconds")) idleDelay = 50;
        else if (idleDelayText.contains("0.1 seconds")) idleDelay = 100;
        else if (idleDelayText.contains("0.5 seconds")) idleDelay = 500;
        else if (idleDelayText.contains("1 second")) idleDelay = 1000;
        else if (idleDelayText.contains("2 seconds")) idleDelay = 2000;
        else if (idleDelayText.contains("5 seconds")) idleDelay = 5000;
        
        LOG_INFO("applyBackgroundHighlightingSettingsFromConfig: Starting background highlighting with new settings");
        
        fileContentView->startBackgroundHighlighting(m_extraHighlightRules, 
                                                    m_highlightCaseSensitive, 
                                                    m_highlightSentence, 
                                                    chunkSize, 
                                                    idleDelay,
                                                    performanceMode,
                                                    chunkMode,
                                                    duration);
    }
    
    logFunctionEnd("applyBackgroundHighlightingSettingsFromConfig");
}


////    ==========================================
///     <<<<    Lamps <<<<<<<<<<<<<<<<<<<<<<<<<<<<
////    ==========================================



void MainWindow::setOpenFileLamp(bool active)
{
    if (openFileLamp) {
        if (active) {
            openFileLamp->setStyleSheet("QLabel { color: white; font-size: 10px; padding: 2px 6px; border: 1px solid #007bff; border-radius: 3px; background-color: #007bff; }");
        } else {
            openFileLamp->setStyleSheet("QLabel { color: #6c757d; font-size: 10px; padding: 2px 6px; border: 1px solid #dee2e6; border-radius: 3px; background-color: transparent; }");
        }
  //   QApplication::processEvents();
  
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents); 

    } 
}

void MainWindow::setScrollHighlightLamp(bool active)
{
    
    if (scrollHighlightLamp) {
        if (active) {
            scrollHighlightLamp->setStyleSheet("QLabel { color: white; font-size: 10px; padding: 2px 6px; border: 1px solid #007bff; border-radius: 3px; background-color: #007bff; }");
        } else {

            QElapsedTimer timer;
            timer.start();
            while (timer.elapsed() < 100) {
                QApplication::processEvents();  // Allow UI updates
            }
            scrollHighlightLamp->setStyleSheet("QLabel { color: #6c757d; font-size: 10px; padding: 2px 6px; border: 1px solid #dee2e6; border-radius: 3px; background-color: transparent; }");


        }
    
    // Force immediate UI update
//    scrollHighlightLamp->update();
 //   statusBar()->update();    
    // Debug: Check if the lamp is actually visible
  //  QString currentStyle = scrollHighlightLamp->styleSheet();
    
  //   Force Qt to process events immediately
 //   QApplication::processEvents();
  //   QApplication::processEvents();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents); 
    }
}     

void MainWindow::setHighlightExtraLamp(bool active)
{
    if (highlightExtraLamp) {
        if (active) {
            highlightExtraLamp->setStyleSheet("QLabel { color: white; font-size: 10px; padding: 2px 6px; border: 1px solid #007bff; border-radius: 3px; background-color: #007bff; }");
        } else {
            highlightExtraLamp->setStyleSheet("QLabel { color: #6c757d; font-size: 10px; padding: 2px 6px; border: 1px solid #dee2e6; border-radius: 3px; background-color: transparent; }");
        }
     // Force immediate UI update
  //   highlightExtraLamp->update();
   //  statusBar()->update();    
     // Debug: Check if the lamp is actually visible
 // QString currentStyle = highlightExtraLamp->styleSheet();
     
     // Force Qt to process events immediately
  //   QApplication::processEvents();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents); 
        
    }
}

void MainWindow::setCleanLamp(bool active)
{
    if (cleanLamp) {
        if (active) {
            cleanLamp->setStyleSheet("QLabel { color: white; font-size: 10px; padding: 2px 6px; border: 1px solid #007bff; border-radius: 3px; background-color: #007bff; }");
        } else {            
            QTimer::singleShot(100, [this]() {
                cleanLamp->setStyleSheet("QLabel { color: #6c757d; font-size: 10px; padding: 2px 6px; border: 1px solid #dee2e6; border-radius: 3px; background-color: transparent; }");
            });
        }
  //   QApplication::processEvents();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents); 

    }
}

void MainWindow::setBackgroundHighlightLamp(bool active, int progress)
{
    if (backgroundHighlightLamp) {
        if (active) {
            // Check if progress indicator is enabled in settings
            QSettings settings("app.ini", QSettings::IniFormat);
            settings.beginGroup("Configuration");
            bool progressEnabled = settings.value("ProgressIndicator", true).toBool();
            settings.endGroup();
            
            QString text;
            if (progressEnabled) {
                text = QString("Background Highlighting (%1%)").arg(progress);
            } else {
                text = "Background Highlighting";
            }
            
            backgroundHighlightLamp->setText(text);
            backgroundHighlightLamp->setStyleSheet("QLabel { color: white; font-size: 10px; padding: 2px 6px; border: 1px solid #007bff; border-radius: 3px; background-color: #007bff; }");
            LOG_INFO("Background highlighting lamp activated with " + QString::number(progress) + "% progress, progress indicator: " + QString(progressEnabled ? "enabled" : "disabled"));
        } else {            
            QTimer::singleShot(1000, [this]() {
                backgroundHighlightLamp->setText("Background Highlighting");
                backgroundHighlightLamp->setStyleSheet("QLabel { color: #6c757d; font-size: 10px; padding: 2px 6px; border: 1px solid #dee2e6; border-radius: 3px; background-color: transparent; }");
            });
            LOG_INFO("Background highlighting lamp deactivated");
        }
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents); 
    }
}


void MainWindow::setSearchResultsLamp(bool active)
{
    if (searchResultsLamp) {
        if (active) {
            searchResultsLamp->setStyleSheet("QLabel { color: white; font-size: 10px; padding: 2px 6px; border: 1px solid #28a745; border-radius: 3px; background-color: #28a745; }");
        } else {            
            QTimer::singleShot(100, [this]() {
                searchResultsLamp->setStyleSheet("QLabel { color: #6c757d; font-size: 10px; padding: 2px 6px; border: 1px solid #dee2e6; border-radius: 3px; background-color: transparent; }");
            });
        }
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents); 
    }
}

void MainWindow::setParsingLamp(bool active, int percentage)
{
    if (parsingLamp) {
        if (active) {
            QString text = percentage > 0 ? QString("Parsing %1%").arg(percentage) : "Parsing";
            parsingLamp->setText(text);
            parsingLamp->setStyleSheet("QLabel { color: white; font-size: 10px; padding: 2px 6px; border: 1px solid #ffc107; border-radius: 3px; background-color: #ffc107; }");
        } else {            
            QTimer::singleShot(100, [this]() {
                parsingLamp->setText("Parsing");
                parsingLamp->setStyleSheet("QLabel { color: #6c757d; font-size: 10px; padding: 2px 6px; border: 1px solid #dee2e6; border-radius: 3px; background-color: transparent; }");
            });
        }
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents); 
    }
}

////    ==========================================
///     >>>>    Lamps End >>>>>>>>>>>>>>>>>>>>>>>>
////    ==========================================

void MainWindow::writeStatusFile(const QString &state, int progress, const QString &description)
{
    QJsonObject statusObj;
    statusObj["state"] = state;
    statusObj["progress"] = progress;
    statusObj["description"] = description;
    statusObj["timestamp"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    
    QJsonDocument doc(statusObj);
    QString statusJson = doc.toJson(QJsonDocument::Compact);
    
    QString statusFilePath = QDir::currentPath() + "/build/Release/status.json";
    QFile statusFile(statusFilePath);
    
    if (statusFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&statusFile);
        out << statusJson;
        statusFile.close();
    }
}

void MainWindow::heartbeatTimer()
{
    // Write current state to status file every second for watchdog monitoring
    switch (m_currentState) {
        case SearchState::IDLE:
            writeStatusFile("IDLE", 0, "Application is idle");
            break;
        case SearchState::SEARCHING:
            writeStatusFile("SEARCHING", 0, "Executing ripgrep search");
            break;
        case SearchState::PARSING_MAIN_SEARCH:
            writeStatusFile("PARSING", 0, "Parsing JSON search results");
            break;
        case SearchState::MAPPING_IN_BACKGROUND:
            writeStatusFile("MAPPING", 0, "Mapping files in background");
            break;
        case SearchState::NAVIGATING_FILES:
            writeStatusFile("NAVIGATING", 0, "Navigating between files");
            break;
        case SearchState::ERROR:
            writeStatusFile("ERROR", 0, "Application error occurred");
            break;
    }
}

void MainWindow::checkParsingTimeout()
{
    LOG_WARNING("MainWindow: Parsing timeout detected - UI state handled centrally");
    
    // Parsing lamp and button state are now handled centrally in performSearchWithCurrentParams
    
    // Update status file
    writeStatusFile("ERROR", 0, "Parsing timeout - operation stuck");
    
    // Show error message to user
    QMessageBox::warning(this, "Parsing Timeout", 
                        "The parsing operation appears to be stuck and has been reset.\n"
                        "The search button has been re-enabled. You can try the search again.");
    
    LOG_ERROR("MainWindow: Parsing timeout - forced reset completed");
}

void MainWindow::loadPaneSizes()
{
    if (!mainSplitter) return;
    
    QSettings settings("app.ini", QSettings::IniFormat);
    settings.beginGroup("PaneSizes");
    
    // Load saved sizes
    QByteArray savedSizes = settings.value("MainSplitterSizes").toByteArray();
    
    if (!savedSizes.isEmpty()) {
        mainSplitter->restoreState(savedSizes);
        LOG_INFO("MainWindow: Loaded saved pane sizes");
    } else {
        // Set default sizes if no saved sizes exist
        QList<int> defaultSizes;
        defaultSizes << 100 << 200 << 400 << 150;  // Search Controls, Results, File Content, Log
        mainSplitter->setSizes(defaultSizes);
        LOG_INFO("MainWindow: Applied default pane sizes");
    }
    
    settings.endGroup();
}

void MainWindow::savePaneSizes()
{
    if (!mainSplitter) return;
    
    QSettings settings("app.ini", QSettings::IniFormat);
    settings.beginGroup("PaneSizes");
    
    // Save current splitter state
    settings.setValue("MainSplitterSizes", mainSplitter->saveState());
    
    settings.endGroup();
    
    LOG_INFO("MainWindow: Saved pane sizes to settings");
}



// ===== AUTO-COMPLETE IMPLEMENTATION =====

void MainWindow::setupAutoComplete()
{
    LOG_INFO("MainWindow: Setting up auto-complete for pattern and path fields");
    
    // Create models for suggestions
    m_patternModel = new QStringListModel(this);
    m_pathModel = new QStringListModel(this);
    
    // Create completers
    m_patternCompleter = new QCompleter(this);
    m_pathCompleter = new QCompleter(this);
    
    // Configure completers
    m_patternCompleter->setModel(m_patternModel);
    m_pathCompleter->setModel(m_pathModel);
    
    // Set completion mode and case sensitivity
    m_patternCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_pathCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    
    // Set completion mode to show popup
    m_patternCompleter->setCompletionMode(QCompleter::PopupCompletion);
    m_pathCompleter->setCompletionMode(QCompleter::PopupCompletion);
    
    // Note: setMinimalPrefixLength is not available in Qt 6, using default behavior
    
    // Set wrap around for cycling through suggestions
    m_patternCompleter->setWrapAround(true);
    m_pathCompleter->setWrapAround(true);
    
    // Apply completers to the fields
    if (patternEdit) {
        patternEdit->setCompleter(m_patternCompleter);
        LOG_INFO("MainWindow: Applied pattern completer to pattern field");
        
        // Add mouse hover auto-complete
        patternEdit->installEventFilter(this);
        
        // Add keyboard shortcuts for auto-complete
        patternEdit->setToolTip("Search pattern (regex or literal text)\n\nExamples:\n- function.*main (regex)\n- TODO: (literal text)\n- \\b(error|warning)\\b (word boundaries)\n\nPress Ctrl+Space to show history dropdown");
    }
    
    if (pathEdit) {
        pathEdit->setCompleter(m_pathCompleter);
        LOG_INFO("MainWindow: Applied path completer to path field");
        
        // Add keyboard shortcuts for auto-complete
        pathEdit->setToolTip("Directory path to search in\n\nExamples:\n- C:\\MyProject\\src\n- /home/user/documents\n- . (current directory)\n\nPress Ctrl+Space to show history dropdown");
        
        // Add event filter for keyboard shortcuts
        pathEdit->installEventFilter(this);
    }
    
    // Update the suggestion lists
    updatePatternCompleter();
    updatePathCompleter();
    
    LOG_INFO("MainWindow: Auto-complete setup completed");
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Handle mouse hover for pattern field auto-complete
    if (obj == patternEdit) {
        if (event->type() == QEvent::Enter) {
            // Mouse entered the pattern field - show auto-complete if field is empty
            if (patternEdit->text().isEmpty() && !m_searchPatternHistory.isEmpty()) {
                // Trigger auto-complete popup
                patternEdit->completer()->complete();
                LOG_INFO("MainWindow: Mouse hover triggered auto-complete for pattern field");
            }
        }
        else if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            // Ctrl+Space to show auto-complete dropdown
            if (keyEvent->key() == Qt::Key_Space && keyEvent->modifiers() == Qt::ControlModifier) {
                patternEdit->completer()->complete();
                LOG_INFO("MainWindow: Ctrl+Space triggered auto-complete for pattern field");
                return true; // Event handled
            }
        }
    }
    else if (obj == pathEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            // Ctrl+Space to show auto-complete dropdown
            if (keyEvent->key() == Qt::Key_Space && keyEvent->modifiers() == Qt::ControlModifier) {
                pathEdit->completer()->complete();
                LOG_INFO("MainWindow: Ctrl+Space triggered auto-complete for path field");
                return true; // Event handled
            }
        }
    }
    
    // Call parent event filter
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::addToPatternHistory(const QString &pattern)
{
    if (pattern.trimmed().isEmpty()) {
        return;
    }
    
    LOG_INFO("MainWindow: Adding pattern to history: " + pattern);
    
    // Remove if exists and add to front
    m_searchPatternHistory.removeAll(pattern);
    m_searchPatternHistory.prepend(pattern);
    
    // Keep within size limit
    while (m_searchPatternHistory.size() > m_maxHistorySize) {
        m_searchPatternHistory.removeLast();
    }
    
    // Update auto-complete suggestions
    updatePatternCompleter();
    
    // Save to file
    saveHistoryToFile();
    
    LOG_INFO("MainWindow: Pattern history updated, total items: " + QString::number(m_searchPatternHistory.size()));
}

void MainWindow::addToPathHistory(const QString &path)
{
    if (path.trimmed().isEmpty()) {
        return;
    }
    
    LOG_INFO("MainWindow: Adding path to history: " + path);
    
    // Remove if exists and add to front
    m_searchPathHistory.removeAll(path);
    m_searchPathHistory.prepend(path);
    
    // Keep within size limit
    while (m_searchPathHistory.size() > m_maxHistorySize) {
        m_searchPathHistory.removeLast();
    }
    
    // Update auto-complete suggestions
    updatePathCompleter();
    
    // Save to file
    saveHistoryToFile();
    
    LOG_INFO("MainWindow: Path history updated, total items: " + QString::number(m_searchPathHistory.size()));
}

void MainWindow::clearAllSearchHistory()
{
    LOG_INFO("MainWindow: Clearing all search history");
    
    // Clear pattern history
    m_searchPatternHistory.clear();
    
    // Clear path history
    m_searchPathHistory.clear();
    
    // Update auto-complete suggestions
    updatePatternCompleter();
    updatePathCompleter();
    
    // Save empty history to file
    saveHistoryToFile();
    
    LOG_INFO("MainWindow: All search history cleared successfully");
}

void MainWindow::updatePatternCompleter()
{
    if (m_patternModel) {
        m_patternModel->setStringList(m_searchPatternHistory);
        LOG_INFO("MainWindow: Updated pattern completer with " + QString::number(m_searchPatternHistory.size()) + " suggestions");
    }
}

void MainWindow::updatePathCompleter()
{
    if (m_pathModel) {
        m_pathModel->setStringList(m_searchPathHistory);
        LOG_INFO("MainWindow: Updated path completer with " + QString::number(m_searchPathHistory.size()) + " suggestions");
    }
}

void MainWindow::saveHistoryToFile()
{
    QSettings settings("app.ini", QSettings::IniFormat);
    settings.beginGroup("SearchHistory");
    
    // Save pattern history
    settings.setValue("Patterns", m_searchPatternHistory);
    
    // Save path history
    settings.setValue("Paths", m_searchPathHistory);
    
    settings.endGroup();
    
    LOG_INFO("MainWindow: Saved search history to file - " + 
             QString::number(m_searchPatternHistory.size()) + " patterns, " + 
             QString::number(m_searchPathHistory.size()) + " paths");
}

void MainWindow::loadHistoryFromFile()
{
    QSettings settings("app.ini", QSettings::IniFormat);
    settings.beginGroup("SearchHistory");
    
    // Load pattern history
    m_searchPatternHistory = settings.value("Patterns", QStringList()).toStringList();
    
    // Load path history
    m_searchPathHistory = settings.value("Paths", QStringList()).toStringList();
    
    settings.endGroup();
    
    // Ensure history doesn't exceed size limit
    while (m_searchPatternHistory.size() > m_maxHistorySize) {
        m_searchPatternHistory.removeLast();
    }
    
    while (m_searchPathHistory.size() > m_maxHistorySize) {
        m_searchPathHistory.removeLast();
    }
    
    LOG_INFO("MainWindow: Loaded search history from file - " + 
             QString::number(m_searchPatternHistory.size()) + " patterns, " + 
             QString::number(m_searchPathHistory.size()) + " paths");
}

// Progress bar control methods
void MainWindow::showSearchProgress()
{
    if (m_searchProgressBar) {
        m_searchProgressBar->setValue(0);
    }
}

void MainWindow::hideSearchProgress()
{
    if (m_searchProgressBar) {
        m_searchProgressBar->setValue(0);
    }
}

void MainWindow::updateSearchProgress(int value)
{
    if (m_searchProgressBar) {
        m_searchProgressBar->setValue(value);
    }
}

void MainWindow::updateParsingProgress(int percentage)
{
    if (m_searchProgressBar) {
        m_searchProgressBar->setValue(percentage);
    }
}

void MainWindow::resetParsingProgress()
{
    // Reset parsing progress tracking
    m_totalExpectedMatches = 0;
    m_isFirstParsingCall = true;
    
    // Reset progress bar
    if (m_searchProgressBar) {
        m_searchProgressBar->setValue(0);
    }
    
    // Reset parsing lamp
    setParsingLamp(false, 0);
}






// ==================================================================================
// <<<<<<<<<<<<<<<<<<<<   on collapsible from search result selected    <<<<<<<<<<<<<
// ==================================================================================
void MainWindow::onCollapsibleResultSelected(const QString &filePath, int lineNumber)
{
    logFunctionStart("onCollapsibleResultSelected");
    
    // Check if we're in ERROR state - only allow navigation if not in ERROR
    if (m_currentState == SearchState::ERROR) {
        LOG_WARNING("onCollapsibleResultSelected: Cannot navigate files while in ERROR state - start new RG search to reset");
        logWidget->append(QString("[%1] ⚠️ ERROR STATE BLOCKING - Cannot navigate files. Start new RG search to reset.").arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz")));
        logFunctionEnd("onCollapsibleResultSelected");
        return;
    }
    
    if (filePath.isEmpty() || lineNumber <= 0) {
        LOG_WARNING("onCollapsibleResultSelected: Invalid file path or line number");
        logFunctionEnd("onCollapsibleResultSelected");
        return;
    }
    
    LOG_INFO("onCollapsibleResultSelected: File: " + filePath + ", Line: " + QString::number(lineNumber));
    
    // Set state to NAVIGATING_FILES for file navigation activity
    m_kSearch->updateSearchState(SearchState::NAVIGATING_FILES);
        
    // Check if we need to open a different file first
    bool needToOpenFile = false;
    
    if (m_keepFilesInCache) {
        // Cache mode: check if file is already in cache
        needToOpenFile = !m_fileCache.contains(filePath);
        LOG_INFO("onCollapsibleResultSelected1: Cache mode - need to open file: " + filePath);
        LOG_INFO("onCollapsibleResultSelected1: needToOpenFile : " + QString::number(needToOpenFile));
    } else {
        // Non-cache mode: check if different file is currently open
        needToOpenFile = (m_currentFilePath4NonCached != filePath);
        LOG_INFO("onCollapsibleResultSelected1: Non-cache mode - need to open file: " + filePath);
        LOG_INFO("onCollapsibleResultSelected1: needToOpenFile : " + QString::number(needToOpenFile));
    }

    
    if (needToOpenFile) {
        LOG_INFO("onCollapsibleResultSelected: Need to open file: " + filePath);
        // Open the file first, then highlight the search result line
        KUpdateFileViewer2(filePath, lineNumber);
    } else {
        LOG_INFO("onCollapsibleResultSelected: File already open, highlighting search result line directly");
        // File is already open, just highlight the search result line with search params color
        highlightSearchResultLine(filePath, lineNumber);
    }
    
    // Reset state to IDLE after successful file navigation (unless we're in ERROR state)
    if (m_currentState != SearchState::ERROR) {
        m_kSearch->updateSearchState(SearchState::IDLE);
    }
    
    LOG_DEBUG("onCollapsibleResultSelected: completed");
    logFunctionEnd("onCollapsibleResultSelected");
}

// <<<<<<< handling with highlight

void MainWindow::KUpdateFileViewer2(const QString &filePath, int lineNumber)
{
    logFunctionStart("KUpdateFileViewer2");
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    LOG_INFO("KUpdateFileViewer2: File: " + filePath + ", Line: " + QString::number(lineNumber));
    LOG_INFO("KUpdateFileViewer2: Cache setting: " + QString(m_keepFilesInCache ? "ENABLED" : "DISABLED"));
    
    // SECTION 1: INPUT VALIDATION
    if (filePath.isEmpty()) {
        LOG_ERROR("KUpdateFileViewer2: File path is empty");
        logFunctionEnd("KUpdateFileViewer2");
        return;
    }
        
    if (lineNumber <= 0) {
        LOG_ERROR("KUpdateFileViewer2: Invalid line number: " + QString::number(lineNumber));
        logFunctionEnd("KUpdateFileViewer2");
        return;
    }
    
    // SECTION 2: FILE CACHE HANDLING
    {
        QMutexLocker cacheLocker(&m_cacheMutex);
        int cacheSize = m_fileCache.size();
        LOG_INFO("KUpdateFileViewer2: Cache state check - cache size: " + QString::number(cacheSize) + ", checking for file: " + filePath);
        if (cacheSize == 0) {
            LOG_INFO("KUpdateFileViewer2: ✅ Cache is empty - no files in cache");
            logWidget->append(QString("[%1] ✅ CACHE EMPTY - No files in cache").arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz")));
        }
    }
    
    bool fileAlreadyInCache = m_fileCache.contains(filePath);
    bool fileAlreadyOpen = false;
    
    if (m_keepFilesInCache) {
        // NEW CACHE MODE: Simple file path list with sanitized content
        LOG_INFO("KUpdateFileViewer2: New cache mode - checking file list");
        
        // Check if file is already in our cache list
        if (m_fileAlreadyOpen4CacheMode.contains(filePath)) {
            // CACHE HIT: File is already in cache
            LOG_INFO("KUpdateFileViewer2: CACHE HIT - File found in cache: " + filePath);
            
            // Get the cached sanitized content
            QString cachedContent = m_cachedFileContent.value(filePath, QString());
            if (!cachedContent.isEmpty()) {
                // Display the cached content
                if (fileContentView) {
                    fileContentView->show();
                    fileContentView->setText(cachedContent);
                    fileContentView->update();
                    QApplication::processEvents();
                }
                
                // Update filename display
                updateFilenameDisplay(filePath);
                
                LOG_INFO("KUpdateFileViewer2: Cached content displayed successfully");
            } else {
                LOG_ERROR("KUpdateFileViewer2: Cached content is empty for file: " + filePath);
            }
        } else {
            // CACHE MISS: File is not in cache
            LOG_INFO("KUpdateFileViewer2: CACHE MISS - File not in cache, loading: " + filePath);
            
            // Show loading message
            if (fileContentView) {
                fileContentView->show();
                fileContentView->setText("Loading file...\n" + filePath + "\nPlease wait...");
                fileContentView->setStyleSheet("QWidget { background-color: #f0f0f0; color: #666; font-size: 14px; }");
                fileContentView->update();
                QApplication::processEvents();
            }
            
                    // Use kOpenFileTransfetToContentFast to load the file
        bool openSuccess = kOpenFileTransfetToContentFast(filePath);
            
            if (openSuccess) {
                // Get the sanitized content from ScintillaEdit
                QString sanitizedContent = fileContentView->getText();
                
                // Add to cache
                m_fileAlreadyOpen4CacheMode.append(filePath);
                m_cachedFileContent[filePath] = sanitizedContent;
                
                LOG_INFO("KUpdateFileViewer2: File loaded and added to cache: " + filePath);
                LOG_INFO("KUpdateFileViewer2: Cache size: " + QString::number(m_fileAlreadyOpen4CacheMode.size()) + " files");
                
                // Update filename display
                updateFilenameDisplay(filePath);
                
                LOG_INFO("KUpdateFileViewer2: File loaded successfully with fast memory mapping");
            } else {
                LOG_ERROR("KUpdateFileViewer2: Failed to load file with fast memory mapping: " + filePath);
                
                // Clear loading message on failure
                if (fileContentView) {
                    fileContentView->setStyleSheet("");
                    fileContentView->setText("Failed to load file: " + filePath);
                }
                return;
            }
        }
        
        // Highlight the search result line with search params color
        setScrollHighlightLamp(true);
        highlightSearchResultLine(filePath, lineNumber);
        setScrollHighlightLamp(false);

        LOG_INFO("KUpdateFileViewer2: highlightSearchResultLine completed for cache mode");
        
    } else {


        // NON-CACHE MODE: Replace files (original behavior)
        LOG_INFO("KUpdateFileViewer2: Non-cache mode - replacing files");
        
        // OPTIMIZATION: Check if the same file is already open in non-cache mode
        if (m_currentFilePath4NonCached == filePath) {
            fileAlreadyOpen = true;
            LOG_INFO("KUpdateFileViewer2: File is already open in non-cache mode: " + filePath);
        } else if (m_logDataWorker && m_logDataWorker->isFileLoaded()) {
        QString currentFilePath = m_logDataWorker->getFilePath();
        if (currentFilePath == filePath) {
            fileAlreadyOpen = true;
            LOG_INFO("KUpdateFileViewer2: File is already open: " + filePath);
        } else {
                LOG_INFO("KUpdateFileViewer2: Different file is open, cleaning up previous file: " + filePath);
            
                // Clean up previous file from memory
            if (m_logDataWorker) {
                    m_logDataWorker->interrupt();
                    // Note: We don't delete m_logDataWorker here to maintain thread persistence
                }
            }
        }
        
        // Open the file if not already open
    if (!fileAlreadyOpen) {
            
            // Show loading message
            if (fileContentView) {
                fileContentView->show();
                fileContentView->setText("Loading file...\n" + filePath + "\nPlease wait while the file is being indexed and loaded.");
                fileContentView->setStyleSheet("QWidget { background-color: #f0f0f0; color: #666; font-size: 14px; }");
                fileContentView->update();
                QApplication::processEvents();
            }
        
                // ===== HIGH-PERFORMANCE FILE OPENING =====
        // Replaced KOpenFile (LogDataWorker-based) with kOpenFileTransfetToContentFast for 5-10x speed improvement
        // KOpenFile creates LogDataWorker, starts background indexing, waits for completion
        // kOpenFileTransfetToContentFast uses direct memory mapping for immediate zero-copy file access
        bool openSuccess = kOpenFileTransfetToContentFast(filePath);
        



        
        if (openSuccess) {
            
            // Update non-cache file path tracking
            m_currentFilePath4NonCached = filePath;     

            // Update filename display
            updateFilenameDisplay(filePath);
            
            // ===== HIGHLIGHT SEARCH RESULT LINE =====
            // Use highlightSearchResultLine for proper search result highlighting with search params color
            LOG_INFO("KUpdateFileViewer2: Calling highlightSearchResultLine with line number: " + QString::number(lineNumber));
            
            setScrollHighlightLamp(true);
            highlightSearchResultLine(filePath, lineNumber);
            setScrollHighlightLamp(false);
            
            LOG_INFO("KUpdateFileViewer2: highlightSearchResultLine completed");
        } else {
            LOG_ERROR("KUpdateFileViewer2: Failed to open file with fast approach");
            logFunctionEnd("KUpdateFileViewer2");
            return;
        }
    } else {
        // File already open in non-cache mode, just highlight the search result line
        highlightSearchResultLine(filePath, lineNumber);
    }
    } // End of non-cache mode section
    
    logFunctionEnd("KUpdateFileViewer2");
}


void MainWindow::highlightSearchResultLine(const QString &filePath, int lineNumber)
{
    logFunctionStart("highlightSearchResultLine");
    LOG_INFO("highlightSearchResultLine: File: " + filePath + ", Line: " + QString::number(lineNumber));
    
    // Update tracking variables
    m_currentSearchResultLine = lineNumber;
    m_currentSearchResultFile = filePath;
    
    // Apply the new search result line highlighting with search params color
    if (fileContentView) {
        LOG_INFO("highlightSearchResultLine: Highlighting search result line: " + QString::number(lineNumber) + 
                 " with color: " + m_searchResultHighlightColor.name());
        
        // Set the search result highlight color and highlight the line
        fileContentView->setHighlightColor(m_searchResultHighlightColor);
        fileContentView->highlightLine(lineNumber);
        
        // Scroll to the line
        fileContentView->scrollToLine(lineNumber);
        
        // Force UI update
        fileContentView->update();
        QApplication::processEvents();
    }
    
    // Apply extra highlights to add any Highlight dialog rules on top
    if (!m_extraHighlightRules.isEmpty()) {
        LOG_INFO("highlightSearchResultLine: Applying extra highlights for Highlight dialog rules");
        m_kSearch->applyExtraHighlights();
    }
    
    LOG_INFO("highlightSearchResultLine: Completed - search result line highlighted with search params color");
    logFunctionEnd("highlightSearchResultLine");
}



bool MainWindow::kOpenFileTransfetToContentFast(const QString& filePath)
{
    logFunctionStart("kOpenFileTransfetToContentFast");
    
    // Activate open file indicator
    setOpenFileLamp(true);
    
    QElapsedTimer timer;
    timer.start();
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    LOG_INFO("kOpenFileTransfetToContentFast: Starting ultra-fast file open - " + filePath);
    logWidget->append(QString("[%1] === ULTRA-FAST FILE OPEN START ===").arg(timestamp));
    
    // ===== STEP 1: FILE VALIDATION =====
    // Check if the file exists before attempting to open it
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        LOG_ERROR("kOpenFileTransfetToContentFast: File does not exist - " + filePath);
        logWidget->append(QString("[%1] ERROR: File does not exist - %2").arg(timestamp, filePath));
        logFunctionEnd("kOpenFileTransfetToContentFast");
        setOpenFileLamp(false);
        return false;
    }
    
    // Get file size for performance monitoring and memory allocation decisions
    qint64 fileSize = fileInfo.size();
    LOG_INFO("kOpenFileTransfetToContentFast: File size: " + QString::number(fileSize) + " bytes");
    
    // ===== STEP 2: FILE OPENING =====
    // PURPOSE: Open file handle in read-only mode for memory mapping
    // WHY: Memory mapping requires an open file handle to create the mapping
    
    // Create QFile object and open in read-only mode
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR("kOpenFileTransfetToContentFast: Failed to open file - " + file.errorString());
        logWidget->append(QString("[%1] ERROR: Failed to open file - %2").arg(timestamp, file.errorString()));
        logFunctionEnd("kOpenFileTransfetToContentFast");
        setOpenFileLamp(false);
        return false;
    }
    
    // Record time taken to open file for performance monitoring
    qint64 openTime = timer.elapsed();
    LOG_INFO("kOpenFileTransfetToContentFast: File opened in " + QString::number(openTime) + "ms");
    
    // ===== STEP 3: MEMORY MAPPING (ZERO-COPY I/O) =====
    // PURPOSE: Create memory mapping for zero-copy file access
    // WHY: Memory mapping allows direct access to file data without copying to RAM
    // PERFORMANCE: This is the key optimization - eliminates data copying overhead
    
    // Attempt to memory-map the entire file for zero-copy access
    // This creates a direct pointer to the file data in virtual memory
    const uchar* mapped = fileSize > 0 ? file.map(0, fileSize) : nullptr;
    if (mapped) {
        // Record time taken for memory mapping
        qint64 mapTime = timer.elapsed();
        LOG_INFO("kOpenFileTransfetToContentFast: Memory mapping succeeded in " + QString::number(mapTime - openTime) + "ms");
        
        // ===== STEP 4: NUL BYTE DETECTION AND SANITIZATION =====
        // PURPOSE: Detect and handle NUL bytes that can cause issues with text editors
        // WHY: NUL bytes (\0) can corrupt text display and cause crashes in Scintilla
        // OPTIMIZATION: Only check first 1MB for performance - NULs are rare in text files
        
        // Check for NUL bytes in the first 1MB (performance optimization)
        // NUL bytes can cause issues with text editors and need to be replaced
        bool hasNul = false;
        for (qint64 i = 0; i < qMin(fileSize, qint64(1000000)); ++i) { // Check first 1MB for NULs
            if (mapped[i] == '\0') { 
                hasNul = true; 
                break; 
            }
        }
        
        if (hasNul) {
            // ===== STEP 4A: SANITIZED TRANSFER (WITH NUL REPLACEMENT) =====
            // PURPOSE: Handle files with NUL bytes by creating a sanitized copy
            // WHY: NUL bytes must be replaced with spaces to prevent text editor issues
            // PERFORMANCE: This path is slower due to data copying, but necessary for safety
            
            LOG_INFO("kOpenFileTransfetToContentFast: NUL bytes detected, sanitizing");
            
            // Create a copy of the mapped data and replace NUL bytes with spaces
            QByteArray sanitized(reinterpret_cast<const char*>(mapped), static_cast<int>(fileSize));
            for (int i = 0; i < sanitized.size(); ++i) {
                if (sanitized[i] == '\0') sanitized[i] = ' ';
            }
            
            // Clear previous highlights before loading new file content
            fileContentView->clearExtraHighlights();
            
            // Transfer sanitized content to ScintillaEdit
            fileContentView->setUtf8Bytes(sanitized.constData(), sanitized.size());
            qint64 transferTime = timer.elapsed();
            logWidget->append(QString("[%1] Sanitized content transferred in %2ms").arg(timestamp, QString::number(transferTime - mapTime)));
        } else {
            // ===== STEP 4B: ZERO-COPY TRANSFER (OPTIMAL PATH) =====
            // PURPOSE: Direct transfer without data copying for maximum performance
            // WHY: When no NUL bytes present, we can transfer data directly without copying
            // PERFORMANCE: This is the fastest path - zero-copy transfer to ScintillaEdit
            
            // Clear previous highlights before loading new file content
            fileContentView->clearExtraHighlights();
            
            // Direct pointer transfer to ScintillaEdit - no data copying required
            // This is the fastest possible path for file loading
            fileContentView->setUtf8Bytes(reinterpret_cast<const char*>(mapped), static_cast<int>(fileSize));
            qint64 transferTime = timer.elapsed();
        }
        
        // ===== STEP 5: CLEANUP - UNMAP FILE =====
        // PURPOSE: Release memory mapping to free system resources
        // WHY: Memory mappings consume system resources and must be released
        // IMPORTANT: This prevents memory leaks and frees virtual memory space
        
        // Release the memory mapping to free system resources
        file.unmap(const_cast<uchar*>(mapped));
        qint64 unmapTime = timer.elapsed();
        
    } else {
        // ===== STEP 3B: FALLBACK - BULK READ (IF MEMORY MAPPING FAILS) =====
        // PURPOSE: Fallback method when memory mapping fails
        // WHY: Memory mapping can fail on some systems, file types, or large files
        // PERFORMANCE: This path is slower but ensures compatibility
        
        // Memory mapping can fail on some systems or file types
        // Fall back to traditional bulk file reading
        logWidget->append(QString("[%1] Memory mapping failed, using bulk read fallback").arg(timestamp));
        LOG_WARNING("kOpenFileTransfetToContentFast: Memory mapping failed, using bulk read fallback");
        
        // Read entire file into memory
        QByteArray fileData = file.readAll();
        qint64 readTime = timer.elapsed();
        logWidget->append(QString("[%1] Bulk read completed in %2ms (%3 bytes)").arg(timestamp, QString::number(readTime - openTime), QString::number(fileData.size())));
        
        // Sanitize NUL bytes if present in bulk read data
        int nulIndex = fileData.indexOf('\0');
        if (nulIndex != -1) {
            logWidget->append(QString("[%1] NUL bytes detected in bulk read, sanitizing...").arg(timestamp));
            for (int i = 0; i < fileData.size(); ++i) {
                if (fileData[i] == '\0') fileData[i] = ' ';
            }
        }
        
        // Clear previous highlights before loading new file content
        fileContentView->clearExtraHighlights();
        
        // Transfer bulk read data to ScintillaEdit
        fileContentView->setUtf8Bytes(fileData.constData(), fileData.size());
        qint64 transferTime = timer.elapsed();
    }
    
    // ===== STEP 6: FILE CLEANUP =====
    // PURPOSE: Close file handle to free system resources
    // WHY: File handles are limited system resources and must be released
    // IMPORTANT: This prevents file handle leaks and allows other processes to access the file
    
    // Close the file handle to free system resources
    file.close();
    
    // ===== STEP 7: UI STATE UPDATE =====
    // PURPOSE: Update UI state and file tracking after successful file load
    // WHY: User interface must reflect the current file and clear any loading states
    // IMPORTANT: This step ensures the UI is consistent with the loaded file
    
    // Update the filename display and current file tracking
    updateFilenameDisplay(filePath);  // Update the filename label in the UI
    m_currentFilePath = filePath;     // Store current file path for future operations
    
    // Clear any loading messages and reset styling
 //   fileContentView->setStyleSheet(""); // Reset styling to remove any loading indicators
 //   fileContentView->update();          // Force UI update to reflect changes
    
    // ===== STEP 8: PERFORMANCE REPORTING =====
    // PURPOSE: Report performance metrics and provide user feedback
    // WHY: Performance monitoring helps identify bottlenecks and user needs feedback
    // IMPORTANT: This step provides visibility into the file loading performance
    
    // Log total completion time and update status bar
    qint64 totalTime = timer.elapsed();
    logWidget->append(QString("[%1] === ULTRA-FAST FILE OPEN COMPLETED ===").arg(timestamp));
    logWidget->append(QString("[%1] Total time: %2ms").arg(timestamp, QString::number(totalTime)));
    LOG_INFO("kOpenFileTransfetToContentFast: Completed in " + QString::number(totalTime) + "ms");
    
    // Show completion message in status bar for user feedback
    statusBar()->showMessage(QString("File loaded: %1 (%2ms)").arg(QFileInfo(filePath).fileName(), QString::number(totalTime)), 3000);
    
    // Deactivate open file status lamp
    setOpenFileLamp(false);
    
    logFunctionEnd("kOpenFileTransfetToContentFast");
    return true;
}

/*
void MainWindow::FastScrollHighlight(int lineNumber, const QColor& highlightColor)
{
    // Activate scroll highlight status lamp
   
    QElapsedTimer timer;
    timer.start();
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    LOG_INFO("FastScrollHighlight: Starting fast scroll to line " + QString::number(lineNumber));
    logWidget->append(QString("[%1] === FAST SCROLL HIGHLIGHT START ===").arg(timestamp));
    
    // ===== STEP 1: VALIDATE INPUTS =====
    if (!fileContentView) {
        LOG_ERROR("FastScrollHighlight: fileContentView is null");
        logWidget->append(QString("[%1] ERROR: fileContentView is null").arg(timestamp));
        return;
    }
    
    if (lineNumber < 1) {
        LOG_ERROR("FastScrollHighlight: Invalid line number " + QString::number(lineNumber));
        logWidget->append(QString("[%1] ERROR: Invalid line number %2").arg(timestamp, QString::number(lineNumber)));
        return;
    }
    
    // ===== STEP 2: GET LINE COUNT FOR VALIDATION =====
    int totalLines = fileContentView->lineCount();
    if (lineNumber > totalLines) {
        LOG_WARNING("FastScrollHighlight: Line number " + QString::number(lineNumber) + " exceeds total lines " + QString::number(totalLines));
        logWidget->append(QString("[%1] WARNING: Line number %2 exceeds total lines %3, clamping to %3").arg(timestamp, QString::number(lineNumber), QString::number(totalLines)));
        lineNumber = totalLines; // Clamp to last line
    }
    
    qint64 validationTime = timer.elapsed();
    logWidget->append(QString("[%1] Validation completed in %2ms (total lines: %3)").arg(timestamp, QString::number(validationTime), QString::number(totalLines)));
    
    // ===== STEP 3: DETERMINE HIGHLIGHT COLOR =====
    QColor finalHighlightColor = highlightColor;
    LOG_INFO("FastScrollHighlight: Using provided highlight color: " + finalHighlightColor.name());
    
    // ===== STEP 4: PERFORM SCROLLING =====
    logWidget->append(QString("[%1] Scrolling to line %2...").arg(timestamp, QString::number(lineNumber)));
    
    
    // Direct scroll call - no validation overhead
    fileContentView->scrollToLine(lineNumber);
    
    qint64 scrollTime = timer.elapsed();
    logWidget->append(QString("[%1] Scroll completed in %2ms").arg(timestamp, QString::number(scrollTime - validationTime)));
    
    // ===== STEP 5: PERFORM HIGHLIGHTING =====
    logWidget->append(QString("[%1] Highlighting line %2...").arg(timestamp, QString::number(lineNumber)));
    
    // Set the highlight color and highlight the line
    fileContentView->setHighlightColor(finalHighlightColor);
    fileContentView->highlightLine(lineNumber);
    
    qint64 highlightTime = timer.elapsed();
    logWidget->append(QString("[%1] Highlight completed in %2ms").arg(timestamp, QString::number(highlightTime - scrollTime)));
    
    // ===== STEP 6: FORCE UI UPDATE =====
    // Ensure the viewport is updated immediately
    fileContentView->update();
    QApplication::processEvents();
    
    qint64 updateTime = timer.elapsed();
    logWidget->append(QString("[%1] UI update completed in %2ms").arg(timestamp, QString::number(updateTime - highlightTime)));
    
    // ===== STEP 7: COMPLETION REPORTING =====
    qint64 totalTime = timer.elapsed();
    logWidget->append(QString("[%1] === FAST SCROLL HIGHLIGHT COMPLETED ===").arg(timestamp));
    logWidget->append(QString("[%1] Total time: %2ms").arg(timestamp, QString::number(totalTime)));
    LOG_INFO("FastScrollHighlight: Completed in " + QString::number(totalTime) + "ms");
    
    // Update status bar with completion info
    statusBar()->showMessage(QString("Scrolled to line %1 (%2ms)").arg(QString::number(lineNumber), QString::number(totalTime)), 2000);
    LOG_INFO("FastScrollHighlight:222 Completed in " + QString::number(totalTime) + "ms");
    
    // Deactivate scroll highlight status lamp
}
*/


void MainWindow::updateFilenameDisplay(const QString &filePath)
{
    // Update File Viewer pane title with full path and filename in bold
    if (fileViewerPane) {
        if (filePath.isEmpty()) {
            fileViewerPane->setPaneTitle("File Viewer");
        } else {
            QFileInfo fileInfo(filePath);
            QString fileName = fileInfo.fileName();
            QString dirPath = fileInfo.path();
            fileViewerPane->setPaneTitle("File Viewer - " + dirPath + "/" + fileName);
        }
    }
}

// ===== DETACHABLE PANE SLOTS =====

void MainWindow::onSearchResultsPaneDetached(DetachablePane* pane)
{
    LOG_INFO("MainWindow: Search Results pane detached");
    statusBar()->showMessage("Search Results pane detached to separate window", 2000);
}

void MainWindow::onSearchResultsPaneAttached(DetachablePane* pane)
{
    LOG_INFO("MainWindow: Search Results pane attached");
    statusBar()->showMessage("Search Results pane attached to main window", 2000);
}

void MainWindow::onSearchResultsPaneClosed(DetachablePane* pane)
{
    LOG_INFO("MainWindow: Search Results pane closed");
    statusBar()->showMessage("Search Results pane closed", 2000);
}

void MainWindow::onFileViewerPaneDetached(DetachablePane* pane)
{
    LOG_INFO("MainWindow: File Viewer pane detached");
    statusBar()->showMessage("File Viewer pane detached to separate window", 2000);
}

void MainWindow::onFileViewerPaneAttached(DetachablePane* pane)
{
    LOG_INFO("MainWindow: File Viewer pane attached");
    statusBar()->showMessage("File Viewer pane attached to main window", 2000);
}

void MainWindow::onFileViewerPaneClosed(DetachablePane* pane)
{
    LOG_INFO("MainWindow: File Viewer pane closed");
    statusBar()->showMessage("File Viewer pane closed", 2000);
}

void MainWindow::applyLayoutSettingsFromConfig(bool enableLogWindow, bool isUpDownLayout)
{
    LOG_INFO("MainWindow: Applying layout settings - Log Window: " + QString(enableLogWindow ? "Enabled" : "Disabled") + 
             ", Layout: " + QString(isUpDownLayout ? "Up/Down" : "Side by Side"));
    
    // Apply log window visibility
    if (logWidget) {
        logWidget->parentWidget()->setVisible(enableLogWindow);
        LOG_INFO("MainWindow: Log window visibility set to " + QString(enableLogWindow ? "visible" : "hidden"));
    }
    
    // Apply layout orientation
    if (mainSplitter) {
        if (isUpDownLayout) {
            // Switch to Up/Down (vertical) layout
            applyVerticalLayout();
            LOG_INFO("MainWindow: Layout set to Up/Down (vertical)");
        } else {
            // Change to Side by Side (horizontal for content panes)
            applyHorizontalLayout();
            LOG_INFO("MainWindow: Layout set to Side by Side (horizontal)");
        }
    }
    
    // Update member variables for persistence
    m_showLogWindow = enableLogWindow;
    
    // Save the layout settings
    QSettings settings("app.ini", QSettings::IniFormat);
    settings.beginGroup("Configuration");
    settings.setValue("EnableLogWindow", enableLogWindow);
    settings.setValue("UpDownLayout", isUpDownLayout);
    settings.endGroup();
    
    statusBar()->showMessage("Layout settings applied", 2000);
}

void MainWindow::applyVerticalLayout()
{
    LOG_INFO("MainWindow: Applying vertical layout dynamically");
    LOG_INFO("MainWindow: Debug - searchResultsPane exists: " + QString(searchResultsPane ? "true" : "false"));
    LOG_INFO("MainWindow: Debug - fileViewerPane exists: " + QString(fileViewerPane ? "true" : "false"));
    
    // Get all widgets from the current layout
    QList<QWidget*> widgets;
    QList<QSplitter*> splitters;
    
    // Collect all widgets and splitters
    LOG_INFO("MainWindow: Debug - Starting widget collection, mainSplitter count: " + QString::number(mainSplitter->count()));
    for (int i = 0; i < mainSplitter->count(); ++i) {
        LOG_INFO("MainWindow: Debug - Processing widget " + QString::number(i));
        QWidget* widget = mainSplitter->widget(i);
        if (!widget) {
            LOG_ERROR("MainWindow: Debug - Widget " + QString::number(i) + " is null!");
            continue;
        }
        LOG_INFO("MainWindow: Debug - Widget " + QString::number(i) + " type: " + widget->metaObject()->className());
        
        if (QSplitter* splitter = qobject_cast<QSplitter*>(widget)) {
            LOG_INFO("MainWindow: Debug - Found splitter with " + QString::number(splitter->count()) + " widgets");
            splitters.append(splitter);
            // Get widgets from the content splitter
            for (int j = 0; j < splitter->count(); ++j) {
                QWidget* subWidget = splitter->widget(j);
                if (subWidget) {
                    LOG_INFO("MainWindow: Debug - Adding sub-widget " + QString::number(j) + " to collection");
                    widgets.append(subWidget);
                } else {
                    LOG_ERROR("MainWindow: Debug - Sub-widget " + QString::number(j) + " is null!");
                }
            }
        } else {
            LOG_INFO("MainWindow: Debug - Adding regular widget to collection");
            widgets.append(widget);
        }
    }
    LOG_INFO("MainWindow: Debug - Widget collection completed, total widgets: " + QString::number(widgets.size()));
    
    // Skip detachable pane parent reference updates to prevent crash
    LOG_INFO("MainWindow: Debug - Skipping detachable pane parent reference updates to prevent crash");
    
    // Clear the main splitter
    LOG_INFO("MainWindow: Debug - About to clear main splitter");
    while (mainSplitter->count() > 0) {
        QWidget* widget = mainSplitter->widget(0);
        LOG_INFO("MainWindow: Debug - Removing widget from main splitter");
        widget->setParent(nullptr);
        LOG_INFO("MainWindow: Debug - Widget removed from main splitter");
    }
    LOG_INFO("MainWindow: Debug - Main splitter cleared");
    
    // Clear splitters without deleting them (safer approach)
    LOG_INFO("MainWindow: Debug - About to clear splitters");
    for (QSplitter* splitter : splitters) {
        while (splitter->count() > 0) {
            QWidget* widget = splitter->widget(0);
            LOG_INFO("MainWindow: Debug - Removing widget from splitter");
            widget->setParent(nullptr);
        }
    }
    LOG_INFO("MainWindow: Debug - Splitters cleared");
    
    // Reorganize widgets for vertical layout
    // Order: searchControls, searchResults, fileContent, logWidget
    if (widgets.size() >= 3) {
        // Add search controls back to main splitter (top)
        mainSplitter->addWidget(widgets[0]); // searchControls
        
        // Add search results (second)
        mainSplitter->addWidget(widgets[1]); // searchResults
        
        // Add file content (third)
        mainSplitter->addWidget(widgets[2]); // fileContent
        
        // Add log widget back to main splitter (bottom) if enabled and exists
        if (m_showLogWindow && widgets.size() >= 4) {
            mainSplitter->addWidget(widgets[3]); // logWidget
        }
        
        // Set reasonable splitter proportions for vertical layout
        QList<int> mainSizes;
        mainSizes << 100; // search controls
        mainSizes << 300; // search results
        mainSizes << 400; // file content
        if (m_showLogWindow && widgets.size() >= 4) {
            mainSizes << 150; // log area
        }
        mainSplitter->setSizes(mainSizes);
        
        LOG_INFO("MainWindow: Vertical layout applied successfully");
        statusBar()->showMessage("Layout changed to Up/Down. Note: Detachable panes may need app restart for full functionality.", 3000);
    } else {
        LOG_ERROR("MainWindow: Not enough widgets for vertical layout change");
        // Restore original layout if something went wrong
        for (QWidget* widget : widgets) {
            mainSplitter->addWidget(widget);
        }
    }
}

void MainWindow::applyHorizontalLayout()
{
    LOG_INFO("MainWindow: Applying horizontal layout dynamically");
    LOG_INFO("MainWindow: Debug - searchResultsPane exists: " + QString(searchResultsPane ? "true" : "false"));
    LOG_INFO("MainWindow: Debug - fileViewerPane exists: " + QString(fileViewerPane ? "true" : "false"));
    
    // Store current splitter sizes for restoration
    QList<int> currentSizes = mainSplitter->sizes();
    
    // Create a new horizontal splitter for the content area
    QSplitter* contentSplitter = new QSplitter(Qt::Horizontal);
    
    // Get the widgets from the main splitter
    // The main splitter currently has: [searchControls, searchResults, fileContent, logWidget]
    QList<QWidget*> widgets;
    for (int i = 0; i < mainSplitter->count(); ++i) {
        widgets.append(mainSplitter->widget(i));
    }
    
    // Update detachable pane parent references for the new layout
    if (searchResultsPane && widgets.size() >= 2) {
        searchResultsPane->setOriginalParent(widgets[1], nullptr);
    }
    if (fileViewerPane && widgets.size() >= 3) {
        fileViewerPane->setOriginalParent(widgets[2], nullptr);
    }
    
    // Clear the main splitter
    while (mainSplitter->count() > 0) {
        mainSplitter->widget(0)->setParent(nullptr);
    }
    
    // Reorganize widgets for horizontal layout
    // Keep search controls at top, put search results and file content side by side, log at bottom
    if (widgets.size() >= 4) {
        // Add search controls back to main splitter (top)
        mainSplitter->addWidget(widgets[0]); // searchControls
        
        // Add content splitter with search results and file content side by side
        contentSplitter->addWidget(widgets[1]); // searchResults
        contentSplitter->addWidget(widgets[2]); // fileContent
        mainSplitter->addWidget(contentSplitter);
        
        // Add log widget back to main splitter (bottom) if enabled
        if (m_showLogWindow) {
            mainSplitter->addWidget(widgets[3]); // logWidget
        }
        
        // Set reasonable splitter proportions
        QList<int> mainSizes;
        mainSizes << 100; // search controls
        mainSizes << 600; // content area (search results + file content)
        if (m_showLogWindow) {
            mainSizes << 150; // log area
        }
        mainSplitter->setSizes(mainSizes);
        
        // Set content splitter proportions (search results on left, file content on right)
        QList<int> contentSizes;
        contentSizes << 300; // search results
        contentSizes << 300; // file content
        contentSplitter->setSizes(contentSizes);
        
        LOG_INFO("MainWindow: Horizontal layout applied successfully");
        statusBar()->showMessage("Layout changed to Side by Side. Note: Detachable panes may need app restart for full functionality.", 3000);
    } else {
        LOG_ERROR("MainWindow: Not enough widgets for layout change");
        // Restore original layout if something went wrong
        for (QWidget* widget : widgets) {
            mainSplitter->addWidget(widget);
        }
    }
}
