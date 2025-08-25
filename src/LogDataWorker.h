#ifndef LOGDATAWORKER_H
#define LOGDATAWORKER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QStringDecoder>
#include <QVector>
#include <QFile>
#include <memory>

class LogDataWorker : public QObject {
    Q_OBJECT
    
public:
    explicit LogDataWorker(QObject *parent = nullptr);
    ~LogDataWorker();
    
    // Start indexing a file
    void startIndexing(const QString& fileName);
    
    // Interrupt indexing
    void interrupt();
    
    // Stop and wait for worker thread to finish
    void stopAndWait();
    
    // Check if worker is shutting down
    bool isShuttingDown() const;
    
    // Check if indexing is in progress
    bool isIndexing() const;
    
    // Get line by index (random access using LinePositionArray)
    QString getLine(int lineIndex);
    
    // Get multiple lines for viewport
    QList<QString> getLines(int firstLine, int count);
    
    // Get total number of lines
    int getTotalLines() const;
    
    // NEW: Bulk load entire file content for fast display
    QString loadEntireFileContent();
    
    // Check if file is loaded
    bool isFileLoaded() const;
    
    // Get file path
    QString getFilePath() const;
    
    // Search functionality
    void searchInFile(const QString& pattern, bool caseSensitive, bool inverse, 
                     bool boolean, bool plainText, int startLine, int endLine);
    void searchInFileSync(const QString& pattern, bool caseSensitive, bool inverse, 
                         bool boolean, bool plainText, int startLine, int endLine);
    QList<int> getSearchResults() const;
    void clearSearchResults();
    bool hasSearchResults() const;
    bool isSearching() const;
    
    // Search history functionality
    void addToSearchHistory(const QString& pattern, const QString& path);
    QStringList getSearchHistory() const;
    void clearSearchHistory();
    void saveSearchHistory();
    void loadSearchHistory();
    
signals:
    void indexingProgressed(int percent);
    void indexingFinished(bool success);
    void viewportUpdateRequested();
    void progressMessage(const QString& message);
    void startIndexingRequested();
    void searchProgressed(int percent);
    void searchFinished(bool success, int resultCount);
    void startSearchRequested();
    
private slots:
    void doIndexing();
    void doSearch();
    
private:
    // Parse a block of data and find line positions
    void parseDataBlock(qint64 blockBeginning, const QByteArray& block);
    
    // Find next line feed in the data
    int findNextLineFeed(const QByteArray& block, int posWithinBlock);
    
    // Load line content using line offsets
    QString loadLineContent(int lineIndex);
    
private:
    QString fileName_;
    QStringDecoder* decoder_;
    QThread workerThread_;
    mutable QMutex indexingMutex_;
    mutable QMutex dataMutex_;
    bool interruptRequest_;
    bool isIndexing_;
    bool isFileLoaded_;
    bool isShuttingDown_;
    
    // File handle
    QFile file_;
    
    // Line position array (KLOGG's core data structure)
    QVector<qint64> lineOffsets_;
    
    // Indexing state
    qint64 fileSize_;
    int totalLines_;
    int maxLineLength_;
    
    // Search state
    QList<int> searchResults_;
    QString lastSearchPattern_;
    bool lastSearchCaseSensitive_;
    bool lastSearchInverse_;
    bool lastSearchBoolean_;
    bool lastSearchPlainText_;
    bool isSearching_;
    bool searchInterruptRequest_;
    
    // Search history
    QStringList searchHistory_;
    static constexpr int MAX_SEARCH_HISTORY_ENTRIES = 1000;
    static constexpr int MaxSearchHistory = MAX_SEARCH_HISTORY_ENTRIES;
    
    static constexpr int ULTRA_FAST_BLOCK_SIZE = 100 * 1024 * 1024; // 100MB blocks for ultra-fast performance on strong computers
    static constexpr int IndexingBlockSize = ULTRA_FAST_BLOCK_SIZE;
};

#endif // LOGDATAWORKER_H 