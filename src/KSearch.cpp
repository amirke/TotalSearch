#include "KSearch.h"

// Global search state variable definition
SearchState g_currentSearchState = SearchState::IDLE;
#include "mainwindow.h"
#include "CollapsibleSearchResults.h"
#include "KSearchBun.h"
#include "scintillaedit.h"
#include "LogDataWorker.h"
#include <QProcess>
#include <QThread>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QMetaObject>
#include <QMutexLocker>
#include "logger.h"
#include <QApplication>
#include <QTextEdit>
#include <QFile>
#include <QTextStream>
#include <QListWidget> // Added for QListWidget
#include <QColor> // Added for QColor
#include <QFileInfo> // Added for QFileInfo
#include <QMessageBox>
#include <QDir>
#include <QDateTime>
#include <QSettings>
#include <QStatusBar>
#include <QTimer>
#include <QEventLoop>


// Constructor
KSearch::KSearch(MainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_mainSearch(mainWindow)
{
}

// Destructor
KSearch::~KSearch()
{
}

void KSearch::KSsearchDo()
{
    m_functionTimer.start();
    m_totalSearchTimer.start();  // Start total search timer
    logFunctionStart("KSsearchDo");
    
    // Safety check for UI elements
    if (!m_mainSearch->patternEdit || !m_mainSearch->pathEdit) {
        LOG_ERROR("KSsearchDo: UI elements not available");
        return;
    }
    
    // Get search parameters
    QString pattern = m_mainSearch->patternEdit->text().trimmed();
    QString path = m_mainSearch->pathEdit->text().trimmed();
    
    // Validate pattern
    if (pattern.isEmpty()) {
        QMessageBox::warning(m_mainSearch, "Warning", "Please enter a search pattern.");
        LOG_WARNING("KSsearchDo: Empty pattern");
        logFunctionEnd("KSsearchDo");
        return;
    }
    
    // Validate path
    if (!QDir(path).exists()) {
        QMessageBox::warning(m_mainSearch, "Warning", "Please select a valid path.");
        LOG_WARNING("KSsearchDo: Invalid path: " + path);
        logFunctionEnd("KSsearchDo");
        return;
    }
        
    // Reset ERROR state to IDLE when starting new search
    if (m_mainSearch->m_currentState == SearchState::ERROR) {
        LOG_INFO("KSsearchDo: Resetting ERROR state to IDLE for new search");
        updateSearchState(SearchState::IDLE);
    }
    
    // Stage 1: Perform ripgrep search
    QElapsedTimer searchTimer;
    searchTimer.start();
    
    // Update search state
    updateSearchState(SearchState::SEARCHING);
    
    // Perform cleanup before search
    KCompleteCleanUp();
    
    // Add to history
    addToSearchHistory(pattern, path);
    
    // Apply extra highlight rules
    applyExtraHighlightParamsRules();
    
    
    // Get current persistent search parameters and update only path and pattern
    RGSearchParams params = m_mainSearch->m_searchBun->getCurrentSearchParams();
    params.path = path;
    params.pattern = pattern;
    
    // Update persistent search parameters in KSearchBun
    m_mainSearch->m_searchBun->updateSearchParams(params);
    LOG_INFO("KSsearchDo: Updated persistent search parameters in KSearchBun (path and pattern from UI, others from INI)");

    QElapsedTimer displayTimer;
    displayTimer.start();
    
    
    // Stage 1: Perform asynch ripgrep search and chain to addSearchResults
    // Disconnect any existing connections to prevent multiple calls
    disconnect(m_mainSearch->m_searchBun, &KSearchBun::asyncSearchCompleted, this, nullptr);
    
    connect(m_mainSearch->m_searchBun, &KSearchBun::asyncSearchCompleted, 
            this, [this, params](const QString &rawOutput) {

        // Check if search was stopped
        if (m_mainSearch->m_currentState == SearchState::STOP) {
        updateSearchState(SearchState::IDLE);
            LOG_INFO("KSsearchDo: Search was stopped, not continuing with search results processing");
            qint64 totalTime = m_functionTimer.elapsed();
            LOG_INFO("KSsearchDo: Total function time: " + QString::number(totalTime) + " ms");
            logFunctionEnd("KSsearchDo");
        
        return;
    }
    
        LOG_INFO("KSsearchDo: Async search completed, starting addSearchResults");
        updateSearchState(SearchState::PARSING_MAIN_SEARCH);
            
        // Stage 2: Display JSON results directly
        // Call addSearchResults asynchronously
        QMetaObject::invokeMethod(this, [this, params, rawOutput]() {
            if (m_mainSearch->collapsibleSearchResults) {

                // Check if search was stopped before parsing
                if (m_mainSearch->m_currentState == SearchState::STOP) {
                    LOG_INFO("KSsearchDo: Search was stopped, not continuing with parsing");
                    updateSearchState(SearchState::IDLE);
        return;
    }
    
                LOG_INFO("KSsearchDo: Setting button to Processing");
            	updateSearchState(SearchState::PARSING_MAIN_SEARCH);
            
                m_mainSearch->collapsibleSearchResults->addSearchResults(params.pattern, rawOutput, params.path);
                 // Note: State transition to IDLE will be handled by onParsingCompleted signal
                LOG_INFO("KSsearchDo: addSearchResults called, waiting for parsing completion signal");

        } else {
                LOG_ERROR("KSsearchDo: collapsibleSearchResults is null!");
            // Reset state even if collapsibleSearchResults is null - this case still needs immediate reset
                LOG_INFO("KSsearchDo: completed");


            }
            

        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);
    
    LOG_INFO("KSsearchDo: About to enter K_RGresults_method3_async at " + QString::number(displayTimer.elapsed()) + " ms");
    m_mainSearch->m_searchBun->K_RGresults_method3_async(params);

 //   QString rawOutput = m_mainSearch->m_searchBun->K_RGresults_method3(params);
    
    qint64 searchTime = searchTimer.elapsed();
    LOG_INFO("KSsearchDo: Stage 1 - Async ripgrep completed in " + QString::number(searchTime) + " ms");
    

    
    LOG_INFO("KSsearchDo: About to enter addSearchResults at " + QString::number(displayTimer.elapsed()) + " ms");
    
    //KDisplayResultWin(params.pattern, rawOutput, params.path);
    
    
    // addSearchResults will be called asynchronously when K_RGresults_method3_async completes
    LOG_INFO("KSsearchDo: Async search started, waiting for completion...");
    
    
   
    
    

    

    qint64 totalTime = m_functionTimer.elapsed();
    LOG_INFO("KSsearchDo: Total function time: " + QString::number(totalTime) + " ms");
    logFunctionEnd("KSsearchDo");
}

void KSearch::setSearchButton(SearchButtonState state)
{
    m_functionTimer.start();
    logFunctionStart("setSearchButton");
    
    try {
        LOG_INFO("KSearch-setSearchButton: setSearchButton called with state=" + QString::number(static_cast<int>(state)));

        if (!m_mainSearch->searchButton) {
            LOG_ERROR("KSearch-setSearchButton: setSearchButton - searchButton is null!");
            return;
        }
        
        switch (state) {
            case SearchButtonState::IDLE:
                m_mainSearch->searchButton->setEnabled(true);
                m_mainSearch->m_isSearching = false;
                m_mainSearch->searchButton->setText("🔍 Search");
                // Note: Stop button state is managed separately, not here
                
                m_mainSearch->searchButton->setStyleSheet(
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
                LOG_INFO("KSearch-setSearchButton: Search button set to IDLE state");
                QApplication::processEvents();
                break;
                
            case SearchButtonState::SEARCHING:
                m_mainSearch->searchButton->setEnabled(false);
                m_mainSearch->m_isSearching = true;
                m_mainSearch->searchButton->setText("⏳ Searching...");
                // Enable stop button when search is active
                
                m_mainSearch->searchButton->setStyleSheet(
                    "QPushButton {"
                    "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6c757d, stop:1 #495057);"
                    "    border: none;"
                    "    color: white;"
                    "    padding: 10px 20px;"
                    "    font-size: 14px;"
                    "    font-weight: 600;"
                    "    border-radius: 8px;"
                    "    min-width: 100px;"
                    "    max-width: 100px;"
                    "    opacity: 0.7;"
                    "    box-shadow: 0 1px 2px rgba(0,0,0,0.1);"
                    "}"
                );
                LOG_INFO("KSearch-setSearchButton: Search button set to SEARCHING state");
                QApplication::processEvents();
                break;
                
            case SearchButtonState::PROCESSING:
                m_mainSearch->searchButton->setEnabled(false);
                m_mainSearch->m_isSearching = true;
                m_mainSearch->searchButton->setText("⚙️ Processing...");
                // Keep stop button enabled during processing
                
                m_mainSearch->searchButton->setStyleSheet(
                    "QPushButton {"
                    "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #17a2b8, stop:1 #138496);"
                    "    border: none;"
                    "    color: white;"
                    "    padding: 10px 20px;"
                    "    font-size: 14px;"
                    "    font-weight: 600;"
                    "    border-radius: 8px;"
                    "    min-width: 100px;"
                    "    max-width: 100px;"
                    "    opacity: 0.7;"
                    "    box-shadow: 0 1px 2px rgba(0,0,0,0.1);"
                    "}"
                );
                LOG_INFO("KSearch-setSearchButton: Search button set to PROCESSING state");
                QApplication::processEvents();
                break;

        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("KSearch-setSearchButton: setSearchButton exception: " + QString(e.what()));
    } catch (...) {
        LOG_ERROR("KSearch-setSearchButton: setSearchButton unknown exception");
    }
    
    qint64 totalTime = m_functionTimer.elapsed();
    LOG_INFO("KSearch-setSearchButton: Total function time: " + QString::number(totalTime) + " ms");
    logFunctionEnd("setSearchButton");
}

void KSearch::setStopButton(StopButtonState state)
{
    m_functionTimer.start();
    logFunctionStart("setStopButton");
    
    try {
        LOG_INFO("KSearch-setStopButton: setStopButton called with state=" + QString::number(static_cast<int>(state)));

        if (!m_mainSearch->stopButton) {
            LOG_ERROR("KSearch-setStopButton: stopButton is null!");
            return;
        }
        
        switch (state) {
            case StopButtonState::DISABLED:
                m_mainSearch->stopButton->setEnabled(false);
                m_mainSearch->stopButton->setText("⏹️ Stop");
                
                m_mainSearch->stopButton->setStyleSheet(
                    "QPushButton {"
                    "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6c757d, stop:1 #495057);"
                    "    border: none;"
                    "    color: white;"
                    "    padding: 10px 20px;"
                    "    font-size: 14px;"
                    "    font-weight: 600;"
                    "    border-radius: 8px;"
                    "    min-width: 80px;"
                    "    max-width: 80px;"
                    "    opacity: 0.5;"
                    "    box-shadow: 0 1px 2px rgba(0,0,0,0.1);"
                    "}"
                );
                LOG_INFO("KSearch-setStopButton: Stop button set to DISABLED state");
                QApplication::processEvents();
                break;
                
            case StopButtonState::ENABLED:
                m_mainSearch->stopButton->setEnabled(true);
                m_mainSearch->stopButton->setText("⏹️ Stop");
                
                m_mainSearch->stopButton->setStyleSheet(
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
                    "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #c82333, stop:1 #a71e2a);"
                    "    transform: translateY(0px);"
                    "    box-shadow: 0 1px 2px rgba(0,0,0,0.2);"
                    "}"
                );
                LOG_INFO("KSearch-setStopButton: Stop button set to ENABLED state");
                QApplication::processEvents();
                break;
                
            case StopButtonState::STOPPING:
                m_mainSearch->stopButton->setEnabled(false);
                m_mainSearch->stopButton->setText("🛑 Stopping...");
                
                m_mainSearch->stopButton->setStyleSheet(
                    "QPushButton {"
                    "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fd7e14, stop:1 #e55a00);"
                    "    border: none;"
                    "    color: white;"
                    "    padding: 10px 20px;"
                    "    font-size: 14px;"
                    "    font-weight: 600;"
                    "    border-radius: 8px;"
                    "    min-width: 80px;"
                    "    max-width: 80px;"
                    "    opacity: 0.7;"
                    "    box-shadow: 0 1px 2px rgba(0,0,0,0.1);"
                    "}"
                );
                LOG_INFO("KSearch-setStopButton: Stop button set to STOPPING state");
                QApplication::processEvents();
                break;
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("KSearch-setStopButton: setStopButton exception: " + QString(e.what()));
    } catch (...) {
        LOG_ERROR("KSearch-setStopButton: setStopButton unknown exception");
    }
    
    qint64 totalTime = m_functionTimer.elapsed();
    LOG_INFO("KSearch-setStopButton: Total function time: " + QString::number(totalTime) + " ms");
    logFunctionEnd("setStopButton");
}

void KSearch::KCompleteCleanUp()
{
    m_functionTimer.start();
    logFunctionStart("KCompleteCleanUp");
    
    try {
        LOG_INFO("KSearch-KCompleteCleanUp: Starting cleanup process");
        
        // Check if we're in a stable state before cleanup
        if (m_mainSearch->m_currentState == SearchState::STOP) {
            LOG_INFO("KCompleteCleanUp: In STOP state, performing aggressive cleanup");
        } else {
            LOG_INFO("KCompleteCleanUp: In normal state, performing conservative cleanup");
        }
        
        // Clear ScintillaEdit extra highlights
        if (m_mainSearch->fileContentView) {
            try {
                LOG_INFO("KCompleteCleanUp: Clearing extra highlights");
                m_mainSearch->fileContentView->clearExtraHighlights();
                LOG_INFO("KCompleteCleanUp: Extra highlights cleared successfully");
            } catch (const std::exception& e) {
                LOG_ERROR("KCompleteCleanUp: Exception clearing extra highlights: " + QString(e.what()));
            } catch (...) {
                LOG_ERROR("KCompleteCleanUp: Unknown exception clearing extra highlights");
            }
        } else {
            LOG_WARNING("KCompleteCleanUp: fileContentView is null, skipping clearExtraHighlights");
        }
        
        // Clear KSearchBun data
        if (m_mainSearch->m_searchBun) {
            try {
                LOG_INFO("KCompleteCleanUp: Clearing KSearchBun data");
                m_mainSearch->m_searchBun->clearAllData();
                LOG_INFO("KCompleteCleanUp: KSearchBun data cleared successfully");
            } catch (const std::exception& e) {
                LOG_ERROR("KCompleteCleanUp: Exception clearing KSearchBun data: " + QString(e.what()));
            } catch (...) {
                LOG_ERROR("KCompleteCleanUp: Unknown exception clearing KSearchBun data");
            }
        } else {
            LOG_WARNING("KCompleteCleanUp: m_searchBun is null, skipping clearAllData");
        }
        
        // Clear file cache structure and force memory release
        try {
            LOG_INFO("KCompleteCleanUp: Clearing file cache structure and forcing memory release");
            {
                QMutexLocker cacheLocker(&m_mainSearch->m_cacheMutex);
                m_mainSearch->m_fileCache.clear();
                m_mainSearch->m_cachedFileContent.clear();
                m_mainSearch->m_fileAlreadyOpen4CacheMode.clear();
            }
            
            // Clear non-cache file path tracking
            m_mainSearch->m_currentFilePath4NonCached.clear();
            LOG_INFO("KCompleteCleanUp: Non-cache file path tracking cleared");
            
            // Force Qt to process events and potentially free memory from cleared containers
            QApplication::processEvents();
            
            LOG_INFO("KCompleteCleanUp: File cache cleared and memory cleanup triggered");
        } catch (const std::exception& e) {
            LOG_ERROR("KCompleteCleanUp: Exception clearing file cache: " + QString(e.what()));
        } catch (...) {
            LOG_ERROR("KCompleteCleanUp: Unknown exception clearing file cache");
        }
        
        // Delete all available workers
        try {
            LOG_INFO("KCompleteCleanUp: Deleting all available workers");
            if (m_mainSearch->m_logDataWorker) {
                m_mainSearch->m_logDataWorker->deleteLater();
                m_mainSearch->m_logDataWorker = nullptr;
                LOG_INFO("KCompleteCleanUp: LogDataWorker deleted successfully");
            } else {
                LOG_INFO("KCompleteCleanUp: LogDataWorker was null, skipping deletion");
            }
        } catch (const std::exception& e) {
            LOG_ERROR("KCompleteCleanUp: Exception deleting workers: " + QString(e.what()));
        } catch (...) {
            LOG_ERROR("KCompleteCleanUp: Unknown exception deleting workers");
        }
        
        // Stop background highlighting
        if (m_mainSearch->fileContentView) {
            try {
                LOG_INFO("KCompleteCleanUp: Stopping background highlighting");
                m_mainSearch->fileContentView->stopBackgroundHighlighting();
                LOG_INFO("KCompleteCleanUp: Background highlighting stopped successfully");
            } catch (const std::exception& e) {
                LOG_ERROR("KCompleteCleanUp: Exception stopping background highlighting: " + QString(e.what()));
            } catch (...) {
                LOG_ERROR("KCompleteCleanUp: Unknown exception stopping background highlighting");
            }
        } else {
            LOG_WARNING("KCompleteCleanUp: fileContentView is null, skipping stopBackgroundHighlighting");
        }
        
        // Clear search results tree
        try {
            if (m_mainSearch->collapsibleSearchResults) {
                LOG_INFO("KCompleteCleanUp: Clearing search results tree");
                m_mainSearch->collapsibleSearchResults->clear();
                // Header reset removed - header is now hidden to save space
                LOG_INFO("KCompleteCleanUp: Search results tree cleared successfully");
            } else {
                LOG_WARNING("KCompleteCleanUp: collapsibleSearchResults is null, skipping clear");
            }
        } catch (const std::exception& e) {
            LOG_ERROR("KCompleteCleanUp: Exception clearing search results tree: " + QString(e.what()));
        } catch (...) {
            LOG_ERROR("KCompleteCleanUp: Unknown exception clearing search results tree");
        }
        
        // Clear preview file content and force memory release
        if (m_mainSearch->fileContentView) {
            try {
                LOG_INFO("KCompleteCleanUp: Clearing preview file content and forcing memory release");
                m_mainSearch->fileContentView->clearText();
                
                // Force memory cleanup by sending additional Scintilla commands
                // This helps free memory for large files
                m_mainSearch->fileContentView->send(SCI_EMPTYUNDOBUFFER);  // Clear undo buffer
                m_mainSearch->fileContentView->send(SCI_CLEARDOCUMENTSTYLE);  // Clear styling
                m_mainSearch->fileContentView->send(SCI_RELEASEALLEXTENDEDSTYLES);  // Release extended styles
                
                // Force Qt to process events and potentially free memory
                QApplication::processEvents();
                
                LOG_INFO("KCompleteCleanUp: Memory cleanup commands sent to fileContentView");
            } catch (const std::exception& e) {
                LOG_ERROR("KCompleteCleanUp: Exception clearing file content: " + QString(e.what()));
            } catch (...) {
                LOG_ERROR("KCompleteCleanUp: Unknown exception clearing file content");
            }
        } else {
            LOG_WARNING("KCompleteCleanUp: fileContentView is null, skipping file content cleanup");
        }
        
        // Reset File Viewer pane title
        if (m_mainSearch->fileViewerPane) {
            m_mainSearch->fileViewerPane->setPaneTitle("File Viewer");
            LOG_INFO("KCompleteCleanUp: File Viewer pane title reset to default");
        }
        
        LOG_INFO("KCompleteCleanUp: Cleanup completed successfully");
        
    } catch (const std::exception& e) {
        LOG_ERROR("KCompleteCleanUp: Exception: " + QString(e.what()));
    } catch (...) {
        LOG_ERROR("KCompleteCleanUp: Unknown exception");
    }
    
    qint64 totalTime = m_functionTimer.elapsed();
    LOG_INFO("KCompleteCleanUp: Total function time: " + QString::number(totalTime) + " ms");
    logFunctionEnd("KCompleteCleanUp");
}

void KSearch::updateSearchState(SearchState newState)
{
    QElapsedTimer functionTimer;
    functionTimer.start();
    logFunctionStart("updateSearchState");
    
    try {
        m_mainSearch->m_currentState = newState;
        g_currentSearchState = newState;  // Update global state
        
        QString stateName;
    QString lampColor;
    
    switch (newState) {
        case SearchState::IDLE:
                stateName = "Ready";
                lampColor = "#007bff";
            break;
        case SearchState::SEARCHING:
                stateName = "Searching...";
                lampColor = "#28a745";
            break;
            
        case SearchState::PARSING_MAIN_SEARCH:
                stateName = "Parsing search results...";
                lampColor = "#17a2b8";
            break;
        case SearchState::MAPPING_IN_BACKGROUND:
                stateName = "Mapping in background...";
                lampColor = "#ffc107";
            break;
        case SearchState::NAVIGATING_FILES:
                stateName = "Navigating files...";
                lampColor = "#17a2b8";
            break;
        case SearchState::ERROR:
                stateName = "Error";
                lampColor = "#dc3545";
                break;
            case SearchState::STOP:
                stateName = "Stopped";
                lampColor = "#6c757d";
            break;
    }
    
        LOG_INFO("KSearch-updateSearchState: Search state updated to: " + stateName + " (Lamp: " + lampColor + ")");
        
        // Update appropriate lamp based on search state
        switch (newState) {
            case SearchState::IDLE:
                // Turn off all lamps when going to idle state
                m_mainSearch->setSearchResultsLamp(false);
                m_mainSearch->setOpenFileLamp(false);
                m_mainSearch->setBackgroundHighlightLamp(false);
                m_mainSearch->setCleanLamp(false);
                m_mainSearch->setParsingLamp(false, 0);


                setSearchButton(SearchButtonState::IDLE);
                setStopButton(StopButtonState::DISABLED);

                LOG_INFO("KSearch-updateSearchState: Idle state - turned off all lamps");
                break;
            case SearchState::SEARCHING:
                // Use search results lamp to indicate searching
                m_mainSearch->setSearchResultsLamp(true);

                setSearchButton(SearchButtonState::SEARCHING);
                setStopButton(StopButtonState::ENABLED);
                LOG_INFO("KSearch-updateSearchState: Set search results lamp to active (searching)");
                break;
            case SearchState::PARSING_MAIN_SEARCH:
                m_mainSearch->setParsingLamp(true, 0);
                m_mainSearch->setSearchResultsLamp(false);

                setSearchButton(SearchButtonState::PROCESSING);
                setStopButton(StopButtonState::ENABLED);
                LOG_INFO("KSearch-updateSearchState: Set parsing lamp to active (parsing)");
                break;
                
            case SearchState::MAPPING_IN_BACKGROUND:
                // Use background highlight lamp to indicate background work
                m_mainSearch->setBackgroundHighlightLamp(true);
                LOG_INFO("KSearch-updateSearchState: Set background highlight lamp to active (mapping)");
                break;
            case SearchState::NAVIGATING_FILES:
                // Use open file lamp to indicate file navigation
                m_mainSearch->setOpenFileLamp(true);
                LOG_INFO("KSearch-updateSearchState: Set open file lamp to active (navigating)");
                break;
            case SearchState::ERROR:
                // Use clean lamp to indicate error state
                m_mainSearch->setCleanLamp(true);
                LOG_INFO("KSearch-updateSearchState: Set clean lamp to active (error)");
                break;
            case SearchState::STOP:
                // Turn off all lamps when stopped
                m_mainSearch->setSearchResultsLamp(false);
                m_mainSearch->setOpenFileLamp(false);
                m_mainSearch->setBackgroundHighlightLamp(false);
                m_mainSearch->setCleanLamp(false);
                m_mainSearch->setParsingLamp(false, 0);

                setStopButton(StopButtonState::STOPPING);

                LOG_INFO("KSearch-updateSearchState: Stop state - turned off all lamps");
                break;
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("KSearch-updateSearchState: Exception: " + QString(e.what()));
    } catch (...) {
        LOG_ERROR("KSearch-updateSearchState: Unknown exception");
    }
    
    logFunctionEnd("updateSearchState");
    LOG_INFO("KSearch-updateSearchState: Function completed successfully");
}

void KSearch::addToSearchHistory(const QString &pattern, const QString &path)
{
    m_functionTimer.start();
    logFunctionStart("addToSearchHistory");
    
    try {
        LOG_INFO("KSearch-addToSearchHistory: Added pattern '" + pattern + "' and path '" + path + "' to history");
        
        // Add to pattern history
        addToPatternHistory(pattern);
        
        // Add to path history
        addToPathHistory(path);
        
    } catch (const std::exception& e) {
        LOG_ERROR("KSearch-addToSearchHistory: Exception: " + QString(e.what()));
    } catch (...) {
        LOG_ERROR("KSearch-addToSearchHistory: Unknown exception");
    }
    
    qint64 totalTime = m_functionTimer.elapsed();
    LOG_INFO("KSearch-addToSearchHistory: Total function time: " + QString::number(totalTime) + " ms");
    logFunctionEnd("addToSearchHistory");
}

void KSearch::addToPatternHistory(const QString &pattern)
{
    m_functionTimer.start();
    logFunctionStart("addToPatternHistory");
    
    try {
        LOG_INFO("KSearch-addToPatternHistory: Adding pattern to history: " + pattern);
        
        // Add to pattern history list
        if (!m_mainSearch->m_searchPatternHistory.contains(pattern)) {
            m_mainSearch->m_searchPatternHistory.prepend(pattern);
            if (m_mainSearch->m_searchPatternHistory.size() > m_mainSearch->m_maxHistorySize) {
                m_mainSearch->m_searchPatternHistory.removeLast();
            }
        }
        
        // Update completer
        m_mainSearch->updatePatternCompleter();
    
    // Save to file
        m_mainSearch->saveHistoryToFile();
        
        LOG_INFO("KSearch-addToPatternHistory: Pattern history updated, total items: " + QString::number(m_mainSearch->m_searchPatternHistory.size()));
        
    } catch (const std::exception& e) {
        LOG_ERROR("KSearch-addToPatternHistory: Exception: " + QString(e.what()));
    } catch (...) {
        LOG_ERROR("KSearch-addToPatternHistory: Unknown exception");
    }
    
    qint64 totalTime = m_functionTimer.elapsed();
    LOG_INFO("KSearch-addToPatternHistory: Total function time: " + QString::number(totalTime) + " ms");
    logFunctionEnd("addToPatternHistory");
}

void KSearch::addToPathHistory(const QString &path)
{
    m_functionTimer.start();
    logFunctionStart("addToPathHistory");
    
    try {
        LOG_INFO("KSearch-addToPathHistory: Adding path to history: " + path);
        
        // Add to path history list
        if (!m_mainSearch->m_searchPathHistory.contains(path)) {
            m_mainSearch->m_searchPathHistory.prepend(path);
            if (m_mainSearch->m_searchPathHistory.size() > m_mainSearch->m_maxHistorySize) {
                m_mainSearch->m_searchPathHistory.removeLast();
            }
        }
        
        // Update completer
        m_mainSearch->updatePathCompleter();
    
    // Save to file
        m_mainSearch->saveHistoryToFile();
        
        LOG_INFO("KSearch-addToPathHistory: Path history updated, total items: " + QString::number(m_mainSearch->m_searchPathHistory.size()));
        
    } catch (const std::exception& e) {
        LOG_ERROR("KSearch-addToPathHistory: Exception: " + QString(e.what()));
    } catch (...) {
        LOG_ERROR("KSearch-addToPathHistory: Unknown exception");
    }
    
    qint64 totalTime = m_functionTimer.elapsed();
    LOG_INFO("KSearch-addToPathHistory: Total function time: " + QString::number(totalTime) + " ms");
    logFunctionEnd("addToPathHistory");
}

void KSearch::applyExtraHighlightParamsRules()
{
    m_functionTimer.start();
    logFunctionStart("applyExtraHighlightParamsRules");
    
    try {
        // Only load from INI if no rules are currently set (first time or after reset)
        if (m_mainSearch->m_extraHighlightRules.isEmpty()) {
            LOG_INFO("KSearch-applyExtraHighlightParamsRules: No rules currently set, loading from INI file");
            
            // Load highlight rules from settings
            QSettings settings("app.ini", QSettings::IniFormat);
            settings.beginGroup("HighlightDialog");
            
            m_mainSearch->m_extraHighlightRules.clear();
        
        int ruleCount = settings.value("RuleCount", 0).toInt();
        LOG_INFO("KSearch-applyExtraHighlightParamsRules: Loaded " + QString::number(ruleCount) + " rules from settings");
        
        for (int i = 0; i < ruleCount; ++i) {
            QString groupKey = QString("Rule%1").arg(i);
            settings.beginGroup(groupKey);
            
            QString pattern = settings.value("Pattern", "").toString();
            QColor color = settings.value("Color", QColor(255, 255, 0)).value<QColor>();
            bool enabled = settings.value("Enabled", true).toBool();
            
            if (!pattern.isEmpty()) {
                m_mainSearch->m_extraHighlightRules.append(HighlightRule(pattern, color, enabled));
            }
            
            settings.endGroup();
        }
        
        // Load global options
        m_mainSearch->m_highlightCaseSensitive = settings.value("CaseSensitive", false).toBool();
        m_mainSearch->m_highlightSentence = settings.value("HighlightSentence", true).toBool();
        m_mainSearch->m_useRGHighlight = settings.value("UseRGHighlight", false).toBool();
        m_mainSearch->m_useQtHighlight = settings.value("UseQtHighlight", false).toBool();
        m_mainSearch->m_useViewportHighlight = settings.value("UseViewportHighlight", true).toBool();
        
            settings.endGroup();
            
            LOG_INFO("KSearch-applyExtraHighlightParamsRules: Loaded " + QString::number(m_mainSearch->m_extraHighlightRules.size()) + " rules from INI file");
            LOG_INFO("KSearch-applyExtraHighlightParamsRules: Global options - Case Sensitive: " + QString(m_mainSearch->m_highlightCaseSensitive ? "Yes" : "No") + 
                     ", Highlight Sentence: " + QString(m_mainSearch->m_highlightSentence ? "Yes" : "No") + 
                     ", Use RG Highlight: " + QString(m_mainSearch->m_useRGHighlight ? "Yes" : "No") + 
                     ", Use Qt Highlight: " + QString(m_mainSearch->m_useQtHighlight ? "Yes" : "No") + 
                     ", Use Viewport Highlight: " + QString(m_mainSearch->m_useViewportHighlight ? "Yes" : "No"));
        } else {
            LOG_INFO("KSearch-applyExtraHighlightParamsRules: Using existing rules from dialog (" + QString::number(m_mainSearch->m_extraHighlightRules.size()) + " rules)");
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("KSearch-applyExtraHighlightParamsRules: Exception: " + QString(e.what()));
    } catch (...) {
        LOG_ERROR("KSearch-applyExtraHighlightParamsRules: Unknown exception");
    }
    
    qint64 totalTime = m_functionTimer.elapsed();
    LOG_INFO("KSearch-applyExtraHighlightParamsRules: Total function time: " + QString::number(totalTime) + " ms");
    logFunctionEnd("applyExtraHighlightParamsRules");
}

void KSearch::applyExtraHighlights()
{
    m_functionTimer.start();
    logFunctionStart("applyExtraHighlights");
    
    try {
        LOG_INFO("KSearch-applyExtraHighlights: Starting extra highlights application");
        
        if (!m_mainSearch->fileContentView) {
            LOG_ERROR("KSearch-applyExtraHighlights: fileContentView is null!");
        return;
    }
    
        // Apply highlights to current file view
        QElapsedTimer highlightTimer;
        highlightTimer.start();
        
        // Call highlightExtra with viewport-only mode for immediate highlighting
        if (m_mainSearch->fileContentView) {
            m_mainSearch->fileContentView->highlightExtra(m_mainSearch->m_extraHighlightRules, 
                                                         m_mainSearch->m_highlightCaseSensitive, 
                                                         m_mainSearch->m_highlightSentence, 
                                                         false, // useScintillaSearch
                                                         true); // useViewportOnly - enable viewport highlighting
            LOG_INFO("KSearch-applyExtraHighlights: Called highlightExtra with viewport-only mode");
        }
        
        qint64 highlightTime = highlightTimer.elapsed();
        LOG_INFO("KSearch-applyExtraHighlights: Highlighting completed in " + QString::number(highlightTime) + "ms");
    
    // Start background highlighting if enabled and not already fully highlighted
    // ===== BACKGROUND HIGHLIGHTING SETUP =====
    //
    // WHY: Background highlighting processes the entire file in chunks when the user is idle.
    //      This provides complete file coverage without blocking the UI during initial highlighting.
    //
    // WHAT: 1. Reads user preferences for chunk processing (lines vs duration mode)
    //       2. Configures timing and performance settings
    //       3. Starts background processing that continues until the entire file is highlighted
    //
    // FLOW: Settings read → Background highlighting started → Chunks processed during idle time
    //
    // Debug: Check if file is fully highlighted
        bool isFullyHighlighted = m_mainSearch->fileContentView ? m_mainSearch->fileContentView->isFullyHighlighted() : true;
        LOG_INFO("KSearch-applyExtraHighlights: File fully highlighted: " + QString(isFullyHighlighted ? "YES" : "NO"));
    
        if (m_mainSearch->fileContentView && !isFullyHighlighted) {
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
            LOG_INFO("KSearch-applyExtraHighlights: Background highlighting setting read: " + QString(backgroundEnabled ? "ENABLED" : "DISABLED"));
            LOG_INFO("KSearch-applyExtraHighlights: Background highlighting will " + QString(backgroundEnabled ? "START" : "NOT START"));
        
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
            
                LOG_INFO("KSearch-applyExtraHighlights: Starting background highlighting - Mode: " + chunkMode + 
                     ", Chunk Size: " + QString::number(chunkSize) + ", Duration: " + QString::number(duration) + "ms" +
                     ", Idle Delay: " + QString::number(idleDelay) + "ms, Performance: " + performanceMode);
            
                m_mainSearch->fileContentView->startBackgroundHighlighting(m_mainSearch->m_extraHighlightRules, 
                                                                          m_mainSearch->m_highlightCaseSensitive, 
                                                                          m_mainSearch->m_highlightSentence, 
                                                        chunkSize, 
                                                        idleDelay,
                                                        performanceMode,
                                                        chunkMode,
                                                        duration);
        }
    }
    
    // Update status bar
        m_mainSearch->statusBar()->showMessage(QString("Extra highlights applied in %1ms").arg(highlightTime), 2000);
    
    // Deactivate highlight extra indicator
    
    } catch (const std::exception& e) {
        LOG_ERROR("KSearch-applyExtraHighlights: Exception: " + QString(e.what()));
    } catch (...) {
        LOG_ERROR("KSearch-applyExtraHighlights: Unknown exception");
    }
    
    qint64 totalTime = m_functionTimer.elapsed();
    LOG_INFO("KSearch-applyExtraHighlights: Total function time: " + QString::number(totalTime) + " ms");
    logFunctionEnd("applyExtraHighlights");
}

void KSearch::logFunctionStart(const QString& functionName)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    LOG_INFO("KSearch.cpp-" + functionName + ": START");
    if (m_mainSearch->logWidget) {
        m_mainSearch->logWidget->append(QString("[%1] KSearch.cpp-%2: START").arg(timestamp, functionName));
    }
}

void KSearch::logFunctionEnd(const QString& functionName)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    LOG_INFO("KSearch.cpp-" + functionName + ": END");
    if (m_mainSearch->logWidget) {
        m_mainSearch->logWidget->append(QString("[%1] KSearch.cpp-%2: END").arg(timestamp, functionName));
    }
}

qint64 KSearch::getCurrentMemoryUsage() const
{
    // Simple memory usage approximation
    // For now, return a placeholder value
    // TODO: Implement proper memory tracking
    return 0;
}

KSearchBun* KSearch::getSearchBun() const
{
    return m_mainSearch->m_searchBun;
}

// Process state querying function
QString KSearch::getProcessStateInfo(QProcess* process) const
{
    if (!process) {
        return "NULL_PROCESS";
    }
    
    QProcess::ProcessState state = process->state();
    QString stateStr;
    
    switch (state) {
        case QProcess::NotRunning:
            stateStr = "NotRunning";
            break;
        case QProcess::Starting:
            stateStr = "Starting";
            break;
        case QProcess::Running:
            stateStr = "Running";
            break;
        default:
            stateStr = "Unknown";
            break;
    }
    
    // Include process ID and additional info
    qint64 pid = process->processId();
    QString result = QString("%1 (PID: %2)").arg(stateStr).arg(pid);
    
    // Add exit code if process has finished
    if (state == QProcess::NotRunning) {
        int exitCode = process->exitCode();
        QProcess::ExitStatus exitStatus = process->exitStatus();
        QString exitStatusStr = (exitStatus == QProcess::NormalExit) ? "Normal" : "Crashed";
        result += QString(" [Exit: %1, Status: %2]").arg(exitCode).arg(exitStatusStr);
    }
    
    return result;
}

// Process termination function
void KSearch::KKillProcess(QProcess* process, const QString& processName)
{
    if (!process) {
        LOG_INFO("KKillProcess: " + processName + " - Process is null, nothing to kill");
        return;
    }
    
    // Query initial process state
    QString initialState = getProcessStateInfo(process);
    LOG_INFO("KKillProcess: " + processName + " - Initial state: " + initialState);
    
    // Check if process is running
    if (process->state() != QProcess::NotRunning) {
        LOG_INFO("KKillProcess: " + processName + " - Process is running, killing it");
        process->kill();
        
        // Monitor process state every 10ms until it stops running
        QElapsedTimer monitorTimer;
        monitorTimer.start();
        int checkCount = 0;
        
        while (monitorTimer.elapsed() < 5000) { // 5 seconds timeout
            QApplication::processEvents(); // Keep UI responsive
            
            QString processStateInfo = getProcessStateInfo(process);
            checkCount++;
             
            // Check if process has stopped running
            if (process->state() == QProcess::NotRunning) {
                LOG_INFO("KKillProcess: " + processName + " - Process successfully terminated after " + 
                        QString::number(monitorTimer.elapsed()) + "ms");
                break;
            }
            
            // Wait 10ms before next check
            QThread::msleep(10);
        }
        
        // Final state check
        QString finalState = getProcessStateInfo(process);
        LOG_INFO("KKillProcess: " + processName + " - Final state: " + finalState);
        
        if (process->state() != QProcess::NotRunning) {
            LOG_WARNING("KKillProcess: " + processName + " - Process termination timeout after " + 
                       QString::number(monitorTimer.elapsed()) + "ms");
            process->close(); // Force close handles
        }
    } else {
        LOG_INFO("KKillProcess: " + processName + " - Process was already not running");
    }
}

// Thread termination function
void KSearch::KKillThread(QThread* thread, const QString& threadName)
{
    if (!thread) {
        LOG_INFO("KKillThread: " + threadName + " - Thread is null, nothing to kill");
        return;
    }
    
    // Query initial thread state
    bool isRunning = thread->isRunning();
    bool isFinished = thread->isFinished();
    LOG_INFO("KKillThread: " + threadName + " - Initial state: Running=" + QString(isRunning ? "Yes" : "No") + 
             ", Finished=" + QString(isFinished ? "Yes" : "No"));
    
    // Check if thread is running
    if (thread->isRunning()) {
        LOG_INFO("KKillThread1: " + threadName + " - Thread is running, terminating it");
        
        // Request thread termination
        thread->quit();
        
        // Monitor thread state every 10ms until it stops running
        QElapsedTimer monitorTimer;
        monitorTimer.start();
        int checkCount = 0;
        
        
        while (monitorTimer.elapsed() < 5000) { // 5 seconds timeout
            QApplication::processEvents(); // Keep UI responsive
            
            bool currentRunning = thread->isRunning();
            bool currentFinished = thread->isFinished();
            checkCount++;
            LOG_INFO("KKillThread: " + threadName + " - Check #" + QString::number(checkCount) + 
                    " (after " + QString::number(monitorTimer.elapsed()) + "ms): " +
                    "Running=" + QString(currentRunning ? "Yes" : "No") + 
                    ", Finished=" + QString(currentFinished ? "Yes" : "No"));
            
            // Check if thread has stopped running
            if (!thread->isRunning()) {
                LOG_INFO("KKillThread1: " + threadName + " - Thread successfully terminated after " + 
                        QString::number(monitorTimer.elapsed()) + "ms");
                break;
            }
            
            // Wait 10ms before next check
            QThread::msleep(10);
        }
        
 
    } else {
        LOG_INFO("KKillThread: " + threadName + " - Thread was already not running");
    }
}


// Stop search - terminate running ripgrep processes
void KSearch::stopSearch()
{
    m_functionTimer.start();
    logFunctionStart("stopSearch");
    
    try {
        

        // ===== STATE RESET =====
        // Set search state to STOP
        LOG_INFO("stopSearch: Setting search state to STOP");
        updateSearchState(SearchState::STOP);


        
        // ===== STATUS UPDATES =====
        // Update status file to "STOP" state
        LOG_INFO("stopSearch: Updating status file to STOP state");
        m_mainSearch->writeStatusFile("STOP", 0, "Search stopped by user");
        
 //       setSearchButton(SearchButtonState::IDLE);
        
        // Update status bar
        m_mainSearch->statusBar()->showMessage("Search stopped", 3000);

 /*       
        // Stop background highlighting via fileContentView
        if (m_mainSearch->fileContentView) {
            LOG_INFO("stopSearch: Stopping file highlighting via fileContentView->stopBackgroundHighlighting()");
            m_mainSearch->fileContentView->stopBackgroundHighlighting();
        }
       


        // Stop any running worker threads
        LOG_INFO("stopSearch: Stopping any running worker threads");
        // TODO: Implement worker thread cleanup
        // This will stop LogDataWorker and other background threads
        
*/         
        // ===== TERMINATE RIPGREP PROCESSES =====
        // Kill search process through KSearchBun
        LOG_INFO("KSearch: stopSearch - Terminating running ripgrep processes");

        // Kill search process using dedicated function if the process exists
        KKillProcess(m_mainSearch->m_searchBun->m_currentSearchProcess, "Search Process");
  /*      
        // Kill parse thread using dedicated function if it exists
        if (m_mainSearch->collapsibleSearchResults) {
            QThread* parseThread = m_mainSearch->collapsibleSearchResults->getParseThread();
            if (parseThread) {
                KKillThread(parseThread, "Parse Thread");
                m_mainSearch->collapsibleSearchResults->resetParseThread();
                LOG_INFO("KSearch: Parse thread and worker killed");
            }
        }
*/
        

        LOG_INFO("KSearch: stopSearch - All processes terminated");
    

        
        LOG_INFO("stopSearch: Comprehensive cleanup completed successfully");
        
    } catch (const std::exception& e) {
        LOG_ERROR("stopSearch: Exception during cleanup: " + QString(e.what()));
        m_mainSearch->statusBar()->showMessage("Error stopping search: " + QString(e.what()), 5000);
    } catch (...) {
        LOG_ERROR("stopSearch: Unknown exception during cleanup");
        m_mainSearch->statusBar()->showMessage("Unknown error stopping search", 5000);
    }
    
    qint64 totalTime = m_functionTimer.elapsed();
    LOG_INFO("stopSearch: Total function time: " + QString::number(totalTime) + " ms");
    logFunctionEnd("stopSearch");
}

void KSearch::updateRule1Pattern(const QString &pattern)
{
    LOG_INFO("KSearch: Updating Rule 1 pattern to: '" + pattern + "'");
    
    // Update Rule 1 in the in-memory rules
    if (m_mainSearch->m_extraHighlightRules.size() > 0) {
        m_mainSearch->m_extraHighlightRules[0].pattern = pattern;
        m_mainSearch->m_extraHighlightRules[0].color = QColor(255, 100, 100);  // Light Red for Rule 1
        m_mainSearch->m_extraHighlightRules[0].enabled = true;
        LOG_INFO("KSearch: Updated Rule 1 in in-memory rules with pattern: '" + pattern + "'");
    } else {
        // If no rules exist, add Rule 1
        m_mainSearch->m_extraHighlightRules.append(
            HighlightRule(pattern, QColor(255, 100, 100), true));
        LOG_INFO("KSearch: Added Rule 1 to in-memory rules with pattern: '" + pattern + "'");
    }
}


