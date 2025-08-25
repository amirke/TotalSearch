#ifndef COLLAPSIBLESEARCHRESULTS_H
#define COLLAPSIBLESEARCHRESULTS_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QString>
#include <QMap>
#include <QLabel>
#include <QPushButton>
#include <QThread> // Added for QThread
#include <QProgressBar>
#include <QObject>

// Forward declaration
class JsonParseWorker;

// No longer using JsonParseWorker - JSON parsing is now done directly in createParseThread

class CollapsibleSearchResults : public QWidget
{
    Q_OBJECT

public:
    explicit CollapsibleSearchResults(QWidget *parent = nullptr);
    ~CollapsibleSearchResults();

    // Clear all results
    void clear();
    
    // Add search results from JSON data
    void addSearchResults(const QString &pattern, const QString &jsonData, const QString &searchPath = QString());
    
    // Get selected file path and line number
    QString getSelectedFilePath() const;
    int getSelectedLineNumber() const;
    
    // Collapse/Expand all results
    void collapseAll();
    void expandAll();
    void toggleExpandCollapse();
    void showSearchingMessage();

    // New method for non-blocking parsing
    void addSearchResultsAsync(const QString &pattern, const QString &jsonData, const QString &searchPath);
    
    // Create parse thread (called after cleanup)
    void createParseThread();
    
    // Stop parsing - terminate background parsing thread
    void stopParsing();
    
    // Get parse thread for direct access (for KSearch)
    QThread* getParseThread() const { return m_parseThread; }
    bool isParsing() const { return m_isParsing; }
    
    // Reset parse thread and worker (for cleanup)
    void resetParseThread();
    
    // Header methods removed - header is now hidden to save space
    
    // Update pane title with search results info
    void updatePaneTitle(const QString& title);
    
    // Update pane title with search time
    void updatePaneTitleWithSearchTime(qint64 totalSearchTimeMs);
    
    // Memory tracking
    void logMemoryUsage(const QString& stage);
    qint64 getCurrentMemoryUsage() const;
    


signals:
    void resultSelected(const QString &filePath, int lineNumber);
    
    // New signals for async parsing
    void parsingStarted();
    void parsingProgress(int percentage, int files);
    void parsingCompleted(int totalMatches, int totalFiles);
    void parsingError(const QString &error);
    void startParsing(const QString &pattern, const QString &jsonData, const QString &searchPath);
    void querySearchStateFromMain();

private slots:
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onItemExpanded(QTreeWidgetItem *item);
    void onItemCollapsed(QTreeWidgetItem *item);
    
    // New slots for async parsing
    Q_INVOKABLE void onParsingStarted();
    Q_INVOKABLE void onParsingProgress(int percentage, int files);
    Q_INVOKABLE void onParsingCompleted(int totalMatches, int totalFiles);
    Q_INVOKABLE void onParsingError(const QString &error);
    Q_INVOKABLE void onFileItemCreated(const QString &filePath, const QString &displayText);
    Q_INVOKABLE void onMatchItemCreated(const QString &filePath, int lineNumber, const QString &lineText);
    Q_INVOKABLE void onFileStatsUpdated(const QString &filePath, const QString &elapsedTime, int matchedLines);
    Q_INVOKABLE void onSummaryParsed(const QString &summaryText, int totalMatchedLines);


private:
    QTreeWidget *m_treeWidget;
    QMap<QString, QTreeWidgetItem*> m_fileItems; // Map file path to tree item
    QMap<QString, QPair<QString, int>> m_fileStats; // Map file path to (elapsed_time, matched_lines)
    QVBoxLayout *m_mainLayout;
    
    // Helper methods
    QTreeWidgetItem* createFileItem(const QString &filePath);
    QTreeWidgetItem* createMatchItem(const QString &filePath, int lineNumber, const QString &lineText);
    QString parseRGSummary(const QString &jsonData, int* outMatchedLines = nullptr);

    QString createFileDisplayText(const QString &filePath, const QString &elapsedTime = QString(), int matchedLines = 0);
    QString createMatchDisplayText(const QString &filePath, int lineNumber, const QString &lineText);
    // Current file display functionality
    void updateCurrentFileDisplay();
    QString getCurrentVisibleFile() const;
    
    // Member variables
    QLabel *m_currentFileLabel;
    QPushButton *m_toggleButton;
    bool m_isExpanded;
    
    // New members for async parsing
    QThread *m_parseThread;
    JsonParseWorker *m_parseWorker;
    bool m_isParsing;
    QElapsedTimer m_parseStartTime;
    
    // Temporary storage for parsed data
    QString m_pendingSummaryText;
    int m_pendingTotalMatchedLines;
    int m_actualFileCount;

    QList<QPair<QString, QString>> m_pendingFileItems; // filePath, displayText
    QList<QPair<QString, QPair<int, QString>>> m_pendingMatchItems; // filePath, (lineNumber, lineText)
    QList<QPair<QString, QPair<QString, int>>> m_pendingFileStats; // filePath, (elapsedTime, matchedLines)
    
    QString m_lastSummaryText; // Store last summary text for search time updates
};

#endif // COLLAPSIBLESEARCHRESULTS_H
