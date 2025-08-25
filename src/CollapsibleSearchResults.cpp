#include "CollapsibleSearchResults.h"
#include "JsonParseWorker.h"
#include "logger.h"
#include "DetachablePane.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QFont>
#include <QColor>
#include <QApplication>
#include <QDebug>
#include <QScrollBar>
#include <QTimer>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QPushButton>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QElapsedTimer>
#include <QFileInfo>
#include <windows.h>
#include <psapi.h>

// JsonParseWorker implementation moved to JsonParseWorker.cpp

CollapsibleSearchResults::CollapsibleSearchResults(QWidget *parent)
    : QWidget(parent)
    , m_treeWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_currentFileLabel(nullptr)
    , m_toggleButton(nullptr)

    , m_isExpanded(false)
    , m_parseThread(nullptr)
    , m_parseWorker(nullptr)
    , m_isParsing(false)

    , m_pendingTotalMatchedLines(0)
    , m_actualFileCount(0)

{
    // Create main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create tree widget
    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels(QStringList() << "🔍 Search Results for pattern:");
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setRootIsDecorated(true);
    m_treeWidget->setExpandsOnDoubleClick(true);
    m_treeWidget->setSortingEnabled(false);
    
    // Hide the tree header to save space
    m_treeWidget->setHeaderHidden(true);
    
    // Force horizontal scroll bar to always be available
    m_treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_treeWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_treeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Set a reasonable minimum width for the tree widget
    m_treeWidget->setMinimumWidth(300);
    
    // Configure tree widget for better horizontal scrolling
    m_treeWidget->setWordWrap(false);
    m_treeWidget->setTextElideMode(Qt::ElideNone);
    
    // Force scroll bars to always be visible
    m_treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_treeWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    
    // Set tree widget to expand columns to content
    m_treeWidget->setUniformRowHeights(false);
    m_treeWidget->setItemsExpandable(true);
    
    // Force the tree to calculate proper column widths
    m_treeWidget->resizeColumnToContents(0);
    
    // Configure the tree to handle wide content properly
    m_treeWidget->setTextElideMode(Qt::ElideNone);
    m_treeWidget->setWordWrap(false);
    
    // Connect signals
    connect(m_treeWidget, &QTreeWidget::itemClicked, this, &CollapsibleSearchResults::onItemClicked);
    connect(m_treeWidget, &QTreeWidget::itemExpanded, this, &CollapsibleSearchResults::onItemExpanded);
    connect(m_treeWidget, &QTreeWidget::itemCollapsed, this, &CollapsibleSearchResults::onItemCollapsed);
    
    // Connect scroll signal for dynamic file path updates
    connect(m_treeWidget->verticalScrollBar(), &QScrollBar::valueChanged, 
            this, &CollapsibleSearchResults::updateCurrentFileDisplay);
    
    // Position updates no longer needed since widget is in layout
    
    // Create current file path display widget in top-left section
    m_currentFileLabel = new QLabel(this);
    
    // Initialize parse thread (will be created after cleanup)
    m_parseThread = nullptr;
    m_isParsing = false;
    m_currentFileLabel->setVisible(false);
    m_currentFileLabel->setStyleSheet(
        "QLabel { "
        "background-color: #e6f3ff; "  // Light blue background
        "color: #0066cc; "  // Blue color
        "font-weight: bold; "
        "font-size: 14px; "  // Slightly smaller for top section
        "padding: 4px 8px; "  // Less padding for compact look
        "border: 1px solid #0066cc; "
        "border-radius: 3px; "
        "}"
    );
    m_currentFileLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_currentFileLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_currentFileLabel->setMaximumHeight(25);  // Fixed height for top section
    
    // Create top section with current file path and buttons
    QHBoxLayout *topLayout = new QHBoxLayout();
    
    // Add current file path widget to the left
    topLayout->addWidget(m_currentFileLabel);
    topLayout->addStretch(); // Push buttons to the right
    
    // Create single toggle button for collapse/expand
    m_toggleButton = new QPushButton("📁 Collapse All", this);
    
    // Beautiful styling with press effects (smaller buttons)
    QString buttonStyle = 
        "QPushButton { "
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f8f9fa, stop:1 #e9ecef); "
        "    border: 1px solid #dee2e6; "
        "    border-radius: 4px; "
        "    padding: 4px 8px; "
        "    font-family: 'Segoe UI', Arial, sans-serif; "
        "    font-weight: 500; "
        "    font-size: 10px; "
        "    color: #495057; "
        "    min-width: 70px; "
        "    min-height: 24px; "
        "} "
        "QPushButton:hover { "
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #e9ecef, stop:1 #dee2e6); "
        "    border-color: #adb5bd; "
        "    color: #212529; "
        "    transform: translateY(-1px); "
        "} "
        "QPushButton:pressed { "
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #dee2e6, stop:1 #ced4da); "
        "    border-color: #6c757d; "
        "    color: #495057; "
        "    transform: translateY(1px); "
        "    padding: 5px 8px 3px 8px; "  // Slight padding adjustment for pressed effect
        "} "
        "QPushButton:focus { "
        "    outline: none; "
        "    border-color: #007bff; "
        "    box-shadow: 0 0 0 1px rgba(0, 123, 255, 0.25); "
        "}";
    
    m_toggleButton->setStyleSheet(buttonStyle);
    
    // Connect toggle button
    connect(m_toggleButton, &QPushButton::clicked, this, &CollapsibleSearchResults::toggleExpandCollapse);
    
    topLayout->addWidget(m_toggleButton);
    
    m_mainLayout->addLayout(topLayout);
    m_mainLayout->addWidget(m_treeWidget);
    
    // File path widget is now part of the layout, no manual positioning needed
}

CollapsibleSearchResults::~CollapsibleSearchResults()
{
}

void CollapsibleSearchResults::clear()
{
    LOG_INFO("CollapsibleSearchResults: clear() called");
    
    logMemoryUsage("Clear - before");
    
    m_treeWidget->clear();
    m_fileItems.clear();
    m_fileStats.clear();
    
    // Force Qt memory cleanup
    QApplication::processEvents();
    logMemoryUsage("Clear - after processEvents");
    
    logMemoryUsage("Clear - after");
}

void CollapsibleSearchResults::stopParsing()
{
    LOG_INFO("CollapsibleSearchResults: stopParsing - Terminating background parsing thread");
    
    // Stop parsing flag
    m_isParsing = false;
    

    
    // Terminate parse thread
    if (m_parseThread && m_parseThread->isRunning()) {
        LOG_INFO("CollapsibleSearchResults: stopParsing - Terminating parse thread");
        m_parseThread->quit();
        m_parseThread->wait(5000); // Wait up to 5 seconds
        
        if (m_parseThread->isRunning()) {
            LOG_WARNING("CollapsibleSearchResults: stopParsing - Force terminating parse thread");
            m_parseThread->terminate();
            m_parseThread->wait(2000); // Wait up to 2 more seconds
        }
        
        m_parseThread->deleteLater();
        m_parseThread = nullptr;
    }
    
    // Delete parse worker
    if (m_parseWorker) {
        m_parseWorker->deleteLater();
        m_parseWorker = nullptr;
    }
    
    // Clear pending data
    m_pendingSummaryText.clear();
    m_pendingTotalMatchedLines = 0;
    m_actualFileCount = 0;
    m_pendingFileItems.clear();
    m_pendingMatchItems.clear();
    m_pendingFileStats.clear();
    
    LOG_INFO("CollapsibleSearchResults: stopParsing - Background parsing terminated");
}

////===========================================================================================================
////===========================================================================================================
////===========================================================================================================





void CollapsibleSearchResults::addSearchResults(const QString &pattern, const QString &jsonData, const QString &searchPath)
{
    LOG_INFO("addSearchResults: Function started");
    QElapsedTimer timer;
    timer.start();
    
    try {
        // Check if tree widget exists
        if (!m_treeWidget) {
            LOG_ERROR("addSearchResults: m_treeWidget is null - cannot proceed!");
            return;
        }
        
        clear();
        
        LOG_INFO("addSearchResults: Starting async parsing");
        addSearchResultsAsync(pattern, jsonData, searchPath);
        
        LOG_INFO("addSearchResults: Function completed");
        LOG_INFO("addSearchResults: Total function time: " + QString::number(timer.elapsed()) + " ms");
        
    } catch (const std::exception &e) {
        LOG_ERROR("addSearchResults: Exception: " + QString(e.what()));
    } catch (...) {
        LOG_ERROR("addSearchResults: Unknown exception");
    }
}

// parseJsonDataInternal method removed - now handled by JsonParseWorker in background thread

QString CollapsibleSearchResults::createFileDisplayText(const QString &filePath, const QString &elapsedTime, int matchedLines)
{
    if (!elapsedTime.isEmpty() && matchedLines > 0) {
        return QString("📁 %1 (%2, %3 lines)").arg(filePath).arg(elapsedTime).arg(matchedLines);
    } else {
        return QString("📁 %1").arg(filePath);
    }
}

QString CollapsibleSearchResults::createMatchDisplayText(const QString &filePath, int lineNumber, const QString &lineText)
{
    return QString("  Line %1: %2").arg(lineNumber).arg(lineText.trimmed());
}



// createParseThread method removed - now handled in addSearchResultsAsync

void CollapsibleSearchResults::addSearchResultsAsync(const QString &pattern, const QString &jsonData, const QString &searchPath)
{
    LOG_INFO("CollapsibleSearchResults: addSearchResultsAsync started");
    
    try {
        // Safety checks
        if (!m_treeWidget) {
            LOG_ERROR("CollapsibleSearchResults: addSearchResultsAsync - m_treeWidget is null!");
            return;
        }
        
        if (m_isParsing) {
            LOG_INFO("CollapsibleSearchResults: Already parsing, ignoring new request");
            return;
        }
        
        // Clear previous results
        clear();
        LOG_INFO("CollapsibleSearchResults: addSearchResultsAsync - clear completed");
        
        // Set initial header
        QString headerText;
        if (pattern.isEmpty()) {
            headerText = "🔍 Search Results for pattern:";
        } else {
            headerText = QString("🔍 Search Results for pattern: '%1'").arg(pattern);
        }
        m_treeWidget->setHeaderLabels(QStringList() << headerText);
        
        // Create parse thread and worker if they don't exist
        if (!m_parseThread) {
            LOG_INFO("CollapsibleSearchResults: Creating parse thread");
            m_parseThread = new QThread(this);
            m_parseWorker = new JsonParseWorker();
            m_parseWorker->moveToThread(m_parseThread);
            
            // Connect worker signals to main thread slots
            connect(m_parseWorker, &JsonParseWorker::parsingStarted, 
                    this, &CollapsibleSearchResults::onParsingStarted, Qt::QueuedConnection);
            connect(m_parseWorker, &JsonParseWorker::parsingProgress, 
                    this, &CollapsibleSearchResults::onParsingProgress, Qt::QueuedConnection);
            connect(m_parseWorker, &JsonParseWorker::parsingCompleted, 
                    this, &CollapsibleSearchResults::onParsingCompleted, Qt::QueuedConnection);
            connect(m_parseWorker, &JsonParseWorker::parsingError, 
                    this, &CollapsibleSearchResults::onParsingError, Qt::QueuedConnection);
            connect(m_parseWorker, &JsonParseWorker::fileItemCreated, 
                    this, &CollapsibleSearchResults::onFileItemCreated, Qt::QueuedConnection);
            connect(m_parseWorker, &JsonParseWorker::matchItemCreated, 
                    this, &CollapsibleSearchResults::onMatchItemCreated, Qt::QueuedConnection);
            connect(m_parseWorker, &JsonParseWorker::fileStatsUpdated, 
                    this, &CollapsibleSearchResults::onFileStatsUpdated, Qt::QueuedConnection);
            connect(m_parseWorker, &JsonParseWorker::summaryParsed, 
                    this, &CollapsibleSearchResults::onSummaryParsed, Qt::QueuedConnection);
            
            // Connect thread finished signal to cleanup
            connect(m_parseThread, &QThread::finished, m_parseWorker, &QObject::deleteLater);
            connect(m_parseThread, &QThread::finished, m_parseThread, &QObject::deleteLater);
            
            m_parseThread->start();
            LOG_INFO("CollapsibleSearchResults: Parse thread created and started");
        }
        
        // Start async parsing
        m_isParsing = true;
        LOG_INFO("CollapsibleSearchResults: Starting background JSON parsing");
        
        // Call the worker's parse method in the background thread
        QMetaObject::invokeMethod(m_parseWorker, "parseJsonData", Qt::QueuedConnection,
                                 Q_ARG(QString, pattern),
                                 Q_ARG(QString, jsonData),
                                 Q_ARG(QString, searchPath));
        
        LOG_INFO("CollapsibleSearchResults: addSearchResultsAsync - parsing started in background");
        
    } catch (const std::exception& e) {
        LOG_ERROR("CollapsibleSearchResults: addSearchResultsAsync exception: " + QString(e.what()));
        m_isParsing = false;
    } catch (...) {
        LOG_ERROR("CollapsibleSearchResults: addSearchResultsAsync unknown exception");
        m_isParsing = false;
    }
}


QString CollapsibleSearchResults::parseRGSummary(const QString &jsonData, int* outMatchedLines)
{
    LOG_INFO("parseRGSummary: Function started");

    try {
        if (jsonData.isEmpty()) {
            LOG_ERROR("parseRGSummary: Input JSON data is empty!");
            return QString();
        }

        QStringList lines = jsonData.split('\n', Qt::SkipEmptyParts);
        LOG_INFO("parseRGSummary: Processing " + QString::number(lines.size()) + " JSON lines");
        
        // Look for summary in the last few lines
        for (int i = lines.size() - 1; i >= qMax(0, lines.size() - 10); --i) {
            const QString &line = lines[i];
            if (line.trimmed().isEmpty()) continue;
            
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);
            
            if (error.error != QJsonParseError::NoError) {
                LOG_INFO("parseRGSummary: Skipping invalid JSON at line " + QString::number(i));
                continue; // Skip invalid JSON
            }
            
            QJsonObject obj = doc.object();
            QString type = obj["type"].toString();
            
            if (type == "summary") {
                QJsonObject dataObj = obj["data"].toObject();
                
                // Extract summary information
                QJsonObject elapsedTotalObj = dataObj["elapsed_total"].toObject();
                QString elapsedTotalStr = elapsedTotalObj["human"].toString();
                
                QJsonObject statsObj = dataObj["stats"].toObject();
                int matchedLines = statsObj["matched_lines"].toInt();
                int searchesWithMatch = statsObj["searches_with_match"].toInt();
                
                // Log all extracted summary data
                LOG_INFO("parseRGSummary: Found summary data:");
                LOG_INFO("  - Total elapsed time: " + elapsedTotalStr);
                LOG_INFO("  - Matched lines: " + QString::number(matchedLines));
                LOG_INFO("  - Files with matches: " + QString::number(searchesWithMatch));
                LOG_INFO("parseRGSummary: *** RG OFFICIAL COUNT *** " + QString::number(matchedLines) + " matched lines");
                
                // Create summary text with the requested format
                QString summaryText = QString("🔍 RG Summary: %1 matched lines in %2 files (%3 total time)")
                                        .arg(matchedLines)
                                        .arg(searchesWithMatch)
                                        .arg(elapsedTotalStr);
                
                LOG_INFO("parseRGSummary: Generated summary text: " + summaryText);
                LOG_INFO("parseRGSummary: Summary processing completed successfully");
                LOG_INFO("parseRGSummary: Function completed");
                
                // Return match count if requested
                if (outMatchedLines) {
                    *outMatchedLines = matchedLines;
                }
                
                return summaryText; // Return the summary text instead of making UI changes
            }
        }
        
        LOG_INFO("parseRGSummary: Function completed - no summary found");
        return QString(); // Return empty string if no summary found
        
    } catch (const std::exception& e) {
        LOG_ERROR("parseRGSummary: Exception caught: " + QString(e.what()));
        return QString();
    } catch (...) {
        LOG_ERROR("parseRGSummary: Unknown exception caught");
        return QString();
    }
}





// Old parseJsonData method removed - JSON parsing is now done in createParseThread

// Async parsing slots
void CollapsibleSearchResults::onParsingStarted()
{
    LOG_INFO("CollapsibleSearchResults: onParsingStarted - SIGNAL RECEIVED!");
    
    m_parseStartTime.start();
    
    // Update header with processing message
    if (m_treeWidget) {
        m_treeWidget->setHeaderLabels(QStringList() << "⚙️ Processing search results...");
    }
    LOG_INFO("CollapsibleSearchResults: onParsingStarted - completed");
}

void CollapsibleSearchResults::onParsingProgress(int percentage, int files)
{
    LOG_INFO("CollapsibleSearchResults: onParsingProgress - " + QString::number(percentage) + "% in " + QString::number(files) + " files");
    
    // Progress tracking moved to main window
    
    // Emit progress signal for main window
    emit parsingProgress(percentage, files);
}

void CollapsibleSearchResults::onParsingCompleted(int totalMatches, int totalFiles)
{
    LOG_INFO("CollapsibleSearchResults: onParsingCompleted - SIGNAL RECEIVED! " + QString::number(totalMatches) + " matches in " + QString::number(totalFiles) + " files");
    
    logMemoryUsage("Parsing completed - start");
    
    // Progress tracking moved to main window
    LOG_INFO("CollapsibleSearchResults: Progress completed - 100% (" + QString::number(m_pendingTotalMatchedLines) + "/" + QString::number(m_pendingTotalMatchedLines) + " matches)");
    
    // Add summary item
    if (totalMatches > 0) {
        QTreeWidgetItem *summaryItem = new QTreeWidgetItem(m_treeWidget);
        QString summaryText = QString("✅ Found %1 matches in %2 files").arg(totalMatches).arg(totalFiles);
        summaryItem->setText(0, summaryText);
        summaryItem->setBackground(0, QColor(100, 255, 100));
        
        QFont summaryFont = summaryItem->font(0);
        summaryFont.setBold(true);
        summaryFont.setPointSize(10);
        summaryItem->setFont(0, summaryFont);
        summaryItem->setFlags(summaryItem->flags() & ~Qt::ItemIsSelectable);
        
            // Update pane title with "Search Results" + summary information
    updatePaneTitle("Search Results - " + summaryText);
    
    // Store the summary text for later use with search time
    m_lastSummaryText = summaryText;
    
    // Simple horizontal scroll setup - make column much wider than needed
    m_treeWidget->setColumnWidth(0, m_treeWidget->width() * 2);
    }
    
    // Expand file items
    m_treeWidget->setUpdatesEnabled(false);
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_treeWidget->topLevelItem(i);
        if (item && item->text(0).startsWith("📁")) {
            item->setExpanded(true);
        }
    }
    m_treeWidget->setUpdatesEnabled(true);
    
    // Update current file display - only use delayed call to avoid blocking
    QTimer::singleShot(100, this, &CollapsibleSearchResults::updateCurrentFileDisplay);
    

    
    m_isParsing = false;
    LOG_INFO("CollapsibleSearchResults: onParsingCompleted - parsing finished");
    
    logMemoryUsage("Parsing completed - end");
    
    // Emit completion signal to main window
    LOG_INFO("CollapsibleSearchResults: Emitting parsingCompleted signal to main window");
    emit parsingCompleted(totalMatches, totalFiles);
    LOG_INFO("CollapsibleSearchResults: parsingCompleted signal emitted");
}

void CollapsibleSearchResults::onParsingError(const QString &error)
{
    LOG_INFO("CollapsibleSearchResults: onParsingError - " + error);
    

    
    // Update header with error message
    if (m_treeWidget) {
        QString errorHeader = "❌ Parsing error: " + error;
        m_treeWidget->setHeaderLabels(QStringList() << errorHeader);
        
        // Also update pane title with "Search Results" + error information
        updatePaneTitle("Search Results - " + errorHeader);
    }
    m_isParsing = false;
}

void CollapsibleSearchResults::onFileItemCreated(const QString &filePath, const QString &displayText)
{
    LOG_INFO("CollapsibleSearchResults: onFileItemCreated - " + filePath);
    
    // Track actual file count
    m_actualFileCount++;
    
    QTreeWidgetItem *fileItem = new QTreeWidgetItem(m_treeWidget);
    fileItem->setText(0, displayText);
    fileItem->setBackground(0, QColor(240, 240, 240));
    
    QFont fileFont = fileItem->font(0);
    fileFont.setBold(true);
    fileFont.setPointSize(10);
    fileItem->setFont(0, fileFont);
    
    fileItem->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_FileIcon));
    fileItem->setFlags(fileItem->flags() & ~Qt::ItemIsSelectable);
    
    // Store reference for later updates
    m_fileItems[filePath] = fileItem;
}

void CollapsibleSearchResults::onMatchItemCreated(const QString &filePath, int lineNumber, const QString &lineText)
{
    
    if (m_fileItems.contains(filePath)) {
        QTreeWidgetItem *fileItem = m_fileItems[filePath];
        QTreeWidgetItem *matchItem = new QTreeWidgetItem();
        
        // Format the text with line number - this creates the visible text in the tree
        matchItem->setText(0, QString("  Line %1: %2").arg(lineNumber).arg(lineText.trimmed()));
        matchItem->setData(0, Qt::UserRole, filePath);
        matchItem->setData(0, Qt::UserRole + 1, lineNumber);
        
        fileItem->addChild(matchItem);
        
        // No recalculation during parsing - will be done once at the end
    }
}

void CollapsibleSearchResults::onSummaryParsed(const QString &summaryText, int totalMatchedLines)
{
    LOG_INFO("CollapsibleSearchResults: onSummaryParsed - " + summaryText + " (" + QString::number(totalMatchedLines) + " lines)");
    
    logMemoryUsage("Summary parsed");
    
    // Store summary data for later use
    m_pendingSummaryText = summaryText;
    m_pendingTotalMatchedLines = totalMatchedLines;
    
    // Update header with the summary text from JsonParseWorker
    if (!summaryText.isEmpty()) {
        m_treeWidget->setHeaderLabels(QStringList() << summaryText);
    } else {
        m_treeWidget->setHeaderLabels(QStringList() << "⚙️ Processing search results...");
    }
    
    // Progress bar removed - now handled in main window
    LOG_INFO("CollapsibleSearchResults: Progress tracking moved to main window");
}



void CollapsibleSearchResults::onFileStatsUpdated(const QString &filePath, const QString &elapsedTime, int matchedLines)
{
    LOG_INFO("CollapsibleSearchResults: onFileStatsUpdated - " + filePath + " (" + elapsedTime + ", " + QString::number(matchedLines) + " lines)");
    
    if (m_fileItems.contains(filePath)) {
        QTreeWidgetItem *fileItem = m_fileItems[filePath];
        QString newText = QString("📁 %1 (%2, %3 lines)").arg(filePath).arg(elapsedTime).arg(matchedLines);
        fileItem->setText(0, newText);
    }
}




////===========================================================================================================
////===========================================================================================================
////===========================================================================================================



QString CollapsibleSearchResults::getSelectedFilePath() const
{
    QTreeWidgetItem *currentItem = m_treeWidget->currentItem();
    if (currentItem && currentItem->data(0, Qt::UserRole).isValid()) {
        return currentItem->data(0, Qt::UserRole).toString();
    }
    return QString();
}

int CollapsibleSearchResults::getSelectedLineNumber() const
{
    QTreeWidgetItem *currentItem = m_treeWidget->currentItem();
    if (currentItem && currentItem->data(0, Qt::UserRole + 1).isValid()) {
        return currentItem->data(0, Qt::UserRole + 1).toInt();
    }
    return -1;
}

void CollapsibleSearchResults::onItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    
    if (!item) return;
    
    // Check if this is a match item (has file path and line number data)
    QString filePath = item->data(0, Qt::UserRole).toString();
    int lineNumber = item->data(0, Qt::UserRole + 1).toInt();
    
    if (!filePath.isEmpty() && lineNumber > 0) {
        emit resultSelected(filePath, lineNumber);
    }
}

void CollapsibleSearchResults::onItemExpanded(QTreeWidgetItem *item)
{
    if (item) {
        item->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon));
        // Update file display when items are expanded
        updateCurrentFileDisplay();
    }
}

void CollapsibleSearchResults::onItemCollapsed(QTreeWidgetItem *item)
{
    if (item) {
        item->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon));
    }
}

QTreeWidgetItem* CollapsibleSearchResults::createFileItem(const QString &filePath)
{
    if (!m_treeWidget) {
        return nullptr;
    }
    
    QTreeWidgetItem *fileItem = new QTreeWidgetItem(m_treeWidget);
    
    // Check if we have statistics for this file
    QString displayText;
    if (m_fileStats.contains(filePath)) {
        QPair<QString, int> stats = m_fileStats[filePath];
        displayText = QString("📁 %1 (Search tool %2, Found %3 lines)")
                        .arg(filePath)
                        .arg(stats.first)  // elapsed time
                        .arg(stats.second); // matched lines
    } else {
        displayText = QString("📁 %1").arg(filePath);
    }
    
    // ===== VISIBLE UPDATE: TREE ITEM TEXT =====
    // Set the initial text for the file item - this creates the visible text in the tree
    fileItem->setText(0, displayText);
    
    // ===== VISIBLE UPDATE: TREE ITEM STYLING =====
    // Set background color and font styling - this changes the visual appearance
    fileItem->setBackground(0, QColor(240, 240, 240));
    
    QFont fileFont = fileItem->font(0);
    fileFont.setBold(true);
    fileFont.setPointSize(10);
    fileItem->setFont(0, fileFont);
    
    // ===== VISIBLE UPDATE: TREE ITEM ICON =====
    // Set folder icon - this changes the visual appearance
    fileItem->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon));
    fileItem->setFlags(fileItem->flags() & ~Qt::ItemIsSelectable);
    
    return fileItem;
}

QTreeWidgetItem* CollapsibleSearchResults::createMatchItem(const QString &filePath, int lineNumber, const QString &lineText)
{
    QTreeWidgetItem *matchItem = new QTreeWidgetItem();
    
    // ===== VISIBLE UPDATE: TREE ITEM TEXT =====
    // Set the text for the match item - this creates the visible text in the tree
    matchItem->setText(0, QString("  Line %1: %2").arg(lineNumber).arg(lineText.trimmed()));
    
    // Store file path and line number in item data (not visible, used for navigation)
    matchItem->setData(0, Qt::UserRole, filePath);
    matchItem->setData(0, Qt::UserRole + 1, lineNumber);
    
    return matchItem;
}
void CollapsibleSearchResults::updateCurrentFileDisplay()
{
    if (!m_treeWidget || m_treeWidget->topLevelItemCount() == 0) {
        if (m_currentFileLabel) {
            m_currentFileLabel->setVisible(false);
        }
        return;
    }
    
    QString currentFile = getCurrentVisibleFile();
    
    if (!currentFile.isEmpty()) {
        if (m_currentFileLabel) {
            m_currentFileLabel->setText(currentFile);
            m_currentFileLabel->setVisible(true);
        }
    } else {
        if (m_currentFileLabel) {
            m_currentFileLabel->setVisible(false);
        }
    }
}

QString CollapsibleSearchResults::getCurrentVisibleFile() const
{
    if (!m_treeWidget) return QString();
    
    // For large datasets, just return the first file item to avoid performance issues
    // The itemAt() method is extremely slow with 2+ million items
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *topItem = m_treeWidget->topLevelItem(i);
        if (topItem && topItem->text(0).startsWith("📁")) {
            QString text = topItem->text(0);
            if (text.startsWith("📁 ")) {
                return text.mid(3); // Remove "📁 " prefix
            }
        }
    }
    
    return QString();
}

// Positioning methods removed - widget is now part of layout

void CollapsibleSearchResults::collapseAll()
{
    if (m_treeWidget) {
        QElapsedTimer timer;
        timer.start();
        
        // Temporarily disable updates for better performance
        m_treeWidget->setUpdatesEnabled(false);
        
        // Collapse only file items (top-level items that start with 📁)
        for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
            QTreeWidgetItem *item = m_treeWidget->topLevelItem(i);
            if (item && item->text(0).startsWith("📁")) {
                item->setExpanded(false);
            }
        }
        
        m_treeWidget->setUpdatesEnabled(true);
        
        qint64 elapsed = timer.elapsed();
    
    }
}

void CollapsibleSearchResults::expandAll()
{
    if (m_treeWidget) {
        QElapsedTimer timer;
        timer.start();
        
        // Temporarily disable updates for better performance
        m_treeWidget->setUpdatesEnabled(false);
        
        // Expand only file items (top-level items that start with 📁)
        for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
            QTreeWidgetItem *item = m_treeWidget->topLevelItem(i);
            if (item && item->text(0).startsWith("📁")) {
                item->setExpanded(true);
            }
        }
        
        m_treeWidget->setUpdatesEnabled(true);
        
        qint64 elapsed = timer.elapsed();
    
    }
}

void CollapsibleSearchResults::toggleExpandCollapse()
{
    if (m_treeWidget) {
        QElapsedTimer timer;
        timer.start();
        
        // Toggle the state
        m_isExpanded = !m_isExpanded;
        
        // Update button text and icon
        if (m_isExpanded) {
            m_toggleButton->setText("📂 Collapse All");
        } else {
            m_toggleButton->setText("📁 Expand All");
        }
        
        // Temporarily disable updates for better performance
        m_treeWidget->setUpdatesEnabled(false);
        
        // Expand or collapse all file items based on current state
        for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
            QTreeWidgetItem *item = m_treeWidget->topLevelItem(i);
            if (item && item->text(0).startsWith("📁")) {
                item->setExpanded(m_isExpanded);
            }
        }
        
        m_treeWidget->setUpdatesEnabled(true);
        
        qint64 elapsed = timer.elapsed();
    
    }
}

void CollapsibleSearchResults::showSearchingMessage()
{
    if (m_treeWidget) {
        // Clear existing items
        m_treeWidget->clear();
        
        // Create a special item to show the searching message
        QTreeWidgetItem *searchingItem = new QTreeWidgetItem(m_treeWidget);
        searchingItem->setText(0, "🔍 Searching...");
        searchingItem->setText(1, "Please wait while searching for matches");
        
        // Style the searching message
        searchingItem->setBackground(0, QColor(240, 240, 240));  // Light gray background
        searchingItem->setForeground(0, QColor(102, 102, 102));  // Dark gray text
        searchingItem->setFont(0, QFont("Arial", 12, QFont::Bold));
        
        // Make it non-selectable and non-expandable
        searchingItem->setFlags(searchingItem->flags() & ~Qt::ItemIsSelectable);
        
        // Expand to show the message
        searchingItem->setExpanded(true);
        
    
    }
}

// Header methods removed - header is now hidden to save space

void CollapsibleSearchResults::logMemoryUsage(const QString& stage)
{
    qint64 memoryUsage = getCurrentMemoryUsage();
    LOG_INFO("CollapsibleSearchResults: Memory usage at " + stage + ": " + QString::number(memoryUsage) + " bytes (" + QString::number(memoryUsage / 1024 / 1024) + " MB)");
}

qint64 CollapsibleSearchResults::getCurrentMemoryUsage() const
{
    // Get process memory usage on Windows
    HANDLE process = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
}

void CollapsibleSearchResults::resetParseThread()
{
    // Reset parse thread and worker pointers
    m_parseThread = nullptr;
    m_parseWorker = nullptr;
    m_isParsing = false;
    LOG_INFO("CollapsibleSearchResults: Parse thread and worker reset to null");
}

void CollapsibleSearchResults::updatePaneTitle(const QString& title)
{
    // Find the parent DetachablePane and update its title
    QWidget* parent = this->parentWidget();
    while (parent) {
        // Try to cast to DetachablePane
        DetachablePane* pane = qobject_cast<DetachablePane*>(parent);
        if (pane) {
            pane->setPaneTitle(title);
            LOG_INFO("CollapsibleSearchResults: Updated pane title to: " + title);
            return;
        }
        parent = parent->parentWidget();
    }
    
    // If no DetachablePane found, log a warning
    LOG_INFO("CollapsibleSearchResults: Could not find DetachablePane parent to update title");
}

void CollapsibleSearchResults::updatePaneTitleWithSearchTime(qint64 totalSearchTimeMs)
{
    if (!m_lastSummaryText.isEmpty()) {
        double totalSearchTimeSec = totalSearchTimeMs / 1000.0;
        QString titleWithTime = "Search Results - " + m_lastSummaryText + " | Search took " + QString::number(totalSearchTimeSec, 'f', 3) + " sec";
        updatePaneTitle(titleWithTime);
        LOG_INFO("CollapsibleSearchResults: Updated pane title with search time: " + QString::number(totalSearchTimeMs) + " ms");
    }
}






