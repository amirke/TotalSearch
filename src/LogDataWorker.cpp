#include "LogDataWorker.h"
#include "logger.h"
#include "mainwindow.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QIODevice>
#include <QRegularExpression>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>
#include "ULTRA_FAST_CONFIG.h"


LogDataWorker::LogDataWorker(QObject *parent)
    : QObject(parent)
    , decoder_(new QStringDecoder(QStringDecoder::Utf8))
    , interruptRequest_(false)
    , isIndexing_(false)
    , isFileLoaded_(false)
    , isShuttingDown_(false)
    , fileSize_(0)
    , totalLines_(0)
    , maxLineLength_(0)
    , isSearching_(false)
    , searchInterruptRequest_(false)
{
    // Move to worker thread
    moveToThread(&workerThread_);
    connect(this, &LogDataWorker::startIndexingRequested, this, &LogDataWorker::doIndexing, Qt::QueuedConnection);
    connect(this, &LogDataWorker::startSearchRequested, this, &LogDataWorker::doSearch, Qt::QueuedConnection);
    workerThread_.start();
    
    // Load search history
    loadSearchHistory();
}

LogDataWorker::~LogDataWorker()
{
    LOG_INFO("LogDataWorker::~LogDataWorker - Destructor called");
    
    // Skip thread management - let Qt handle thread cleanup
    // This prevents crashes from corrupted thread state or deadlocks
    LOG_INFO("LogDataWorker::~LogDataWorker - Skipping thread management");
    
    delete decoder_;
    LOG_INFO("LogDataWorker::~LogDataWorker - Destructor completed");
}

void LogDataWorker::startIndexing(const QString& fileName)
{
    LOG_DEBUG("LogDataWorker::startIndexing - " + fileName);
    QMutexLocker locker(&indexingMutex_);
    
    if (isIndexing_) {
        LOG_WARNING("LogDataWorker::startIndexing - Indexing already in progress");
        return;
    }
    
    // Close any previously open file
    if (file_.isOpen()) {
        LOG_DEBUG("LogDataWorker::startIndexing - Closing previous file");
        file_.close();
    }
    
    fileName_ = fileName;
    interruptRequest_ = false;
    isIndexing_ = true;
    isFileLoaded_ = false;
    totalLines_ = 0;
    
    // Clear previous data
    lineOffsets_.clear();
    LOG_DEBUG("LogDataWorker::startIndexing - State reset: totalLines=" + QString::number(totalLines_) + ", lineOffsets.size=" + QString::number(lineOffsets_.size()));
    // Amir  lineOffsets_.append(0); // First line starts at offset 0
        // Don't append 0 - first line will be added when we find the first line feed
    
    
    LOG_DEBUG("LogDataWorker::startIndexing - Starting indexing");
    
    // Emit signal to trigger doIndexing() in worker thread
    emit startIndexingRequested();
}

void LogDataWorker::interrupt()
{
    QMutexLocker locker(&indexingMutex_);
    interruptRequest_ = true;
    LOG_INFO("LogDataWorker::interrupt - Interrupted");
}

void LogDataWorker::stopAndWait()
{
    LOG_INFO("LogDataWorker::stopAndWait - Starting graceful shutdown");
    
    try {
    // Set shutdown flag to prevent new operations
    isShuttingDown_ = true;
    
    // Interrupt any ongoing operations
    interrupt();
    LOG_INFO("LogDataWorker::stopAndWait - Interrupted");
    
    // Wait for worker thread to finish
    if (workerThread_.isRunning()) {
        LOG_INFO("LogDataWorker::stopAndWait - Waiting for worker thread to finish");
        workerThread_.quit();
        if (!workerThread_.wait(100)) { // Wait up to 0.1 seconds
            LOG_WARNING("LogDataWorker::stopAndWait - Worker thread did not finish gracefully, terminating");
            workerThread_.terminate();
            workerThread_.wait();
        }
        LOG_INFO("LogDataWorker::stopAndWait - Worker thread finished");
    }
    } catch (...) {
        LOG_WARNING("LogDataWorker::stopAndWait - Exception caught during shutdown");
    }
    
    LOG_INFO("LogDataWorker::stopAndWait - Setting shutdown flag to true");
    // Close file if open
    if (file_.isOpen()) {
        file_.close();
        LOG_INFO("LogDataWorker::stopAndWait - Closed file");
    }
    
    LOG_INFO("LogDataWorker::stopAndWait - Graceful shutdown completed");
}

bool LogDataWorker::isShuttingDown() const
{
    return isShuttingDown_;
}

bool LogDataWorker::isIndexing() const
{
    QMutexLocker locker(&indexingMutex_);
    return isIndexing_;
}

QString LogDataWorker::getLine(int lineIndex)
{
    QMutexLocker locker(&dataMutex_);
    
    if (!isFileLoaded_ || lineIndex < 0 || lineIndex >= totalLines_) {
        return QString();
    }
    
    return loadLineContent(lineIndex);
}

QList<QString> LogDataWorker::getLines(int firstLine, int count)
{
    QMutexLocker locker(&dataMutex_);
    
    QList<QString> lines;
    if (!isFileLoaded_) {
        return lines;
    }
    
    int endLine = qMin(firstLine + count, totalLines_);
    lines.reserve(endLine - firstLine);
    
    for (int i = firstLine; i < endLine; ++i) {
        lines.append(loadLineContent(i));
    }
    
    return lines;
}

int LogDataWorker::getTotalLines() const
{
    QMutexLocker locker(&dataMutex_);
    return totalLines_;
}

bool LogDataWorker::isFileLoaded() const
{
    QMutexLocker locker(&dataMutex_);
    return isFileLoaded_;
}

QString LogDataWorker::getFilePath() const
{
    return fileName_;
}

void LogDataWorker::searchInFile(const QString& pattern, bool caseSensitive, bool inverse, 
                                bool boolean, bool plainText, int startLine, int endLine)
{
    QMutexLocker locker(&dataMutex_);
    
    if (!isFileLoaded_) {
        LOG_WARNING("LogDataWorker::searchInFile - No file loaded");
        return;
    }
    
    if (isSearching_) {
        LOG_WARNING("LogDataWorker::searchInFile - Search already in progress");
        return;
    }
    
    // Clear previous search results
    searchResults_.clear();
    
    // Store search parameters
    lastSearchPattern_ = pattern;
    lastSearchCaseSensitive_ = caseSensitive;
    lastSearchInverse_ = inverse;
    lastSearchBoolean_ = boolean;
    lastSearchPlainText_ = plainText;
    
    // Validate line range
    startLine = qMax(1, startLine);
    endLine = qMin(endLine, totalLines_);
    
    if (startLine > endLine) {
        LOG_WARNING("LogDataWorker::searchInFile - Invalid line range");
        return;
    }
    
    isSearching_ = true;
    searchInterruptRequest_ = false;
    
    // Emit signal to trigger doSearch() in worker thread
    emit startSearchRequested();
}

void LogDataWorker::searchInFileSync(const QString& pattern, bool caseSensitive, bool inverse, 
                                    bool boolean, bool plainText, int startLine, int endLine)
{
    QMutexLocker locker(&dataMutex_);
    
    if (!isFileLoaded_) {
        LOG_WARNING("LogDataWorker::searchInFileSync - No file loaded");
        return;
    }
    
    // Clear previous search results
    searchResults_.clear();
    
    // Store search parameters
    lastSearchPattern_ = pattern;
    lastSearchCaseSensitive_ = caseSensitive;
    lastSearchInverse_ = inverse;
    lastSearchBoolean_ = boolean;
    lastSearchPlainText_ = plainText;
    
    // Validate line range
    startLine = qMax(1, startLine);
    endLine = qMin(endLine, totalLines_);
    
    if (startLine > endLine) {
        LOG_WARNING("LogDataWorker::searchInFileSync - Invalid line range");
        return;
    }
    
    // Convert to 0-based indexing
    int startIndex = startLine - 1;
    int endIndex = endLine - 1;
    
    // Create regex pattern
    QRegularExpression regex;
    if (plainText) {
        regex = QRegularExpression(QRegularExpression::escape(pattern));
    } else {
        regex = QRegularExpression(pattern);
    }
    
    if (!caseSensitive) {
        regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }
    
    if (!regex.isValid()) {
        LOG_ERROR("LogDataWorker::searchInFileSync - Invalid regex pattern: " + regex.errorString());
        return;
    }
    
    
    // Use ripgrep for search instead of line-by-line
    QProcess *process = new QProcess();
    
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
    if (caseSensitive) {
        arguments << "--case-sensitive";
    } else {
        arguments << "--ignore-case";
    }
    
    if (plainText) {
        arguments << "-F";                // Fixed strings mode
    }
    // else: ripgrep uses regex by default
    
    // Add the pattern and file path
    arguments << pattern;
    arguments << fileName_;
    
    // Execute ripgrep synchronously
    process->start("rg", arguments);
    LOG_INFO("Amir: ripgrep started with command: rg " + arguments.join(" "));
    process->waitForFinished();
    
    QString output = QString::fromUtf8(process->readAllStandardOutput());
    LOG_INFO("Amir: ripgrep output: " + output);
    QString error = QString::fromUtf8(process->readAllStandardError());
    LOG_INFO("Amir: ripgrep error: " + error);
    
    if (!error.isEmpty()) {
        LOG_WARNING("LogDataWorker::searchInFileSync - Ripgrep error: " + error);
    }
    
    // Parse ripgrep JSON output to extract line numbers
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (line.trimmed().isEmpty()) continue;
        
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &jsonError);
        if (jsonError.error != QJsonParseError::NoError) {
            continue;
        }
        
        QJsonObject obj = doc.object();
        QString messageType = obj["type"].toString();
        
        if (messageType == "match") {
            QJsonObject data = obj["data"].toObject();
            
            // Get line number
            int lineNumber = 0;
            if (data.contains("line_number")) {
                lineNumber = data["line_number"].toInt();
            }
            
            // Convert to 0-based index and check if it's in our range
            int lineIndex = lineNumber - 1;
            if (lineIndex >= startIndex && lineIndex <= endIndex) {
                searchResults_.append(lineNumber); // Store 1-based line number
            }
        }
    }
    
    process->deleteLater();
    
    LOG_INFO("LogDataWorker::searchInFileSync - Found " + QString::number(searchResults_.size()) + " matches");
}

QList<int> LogDataWorker::getSearchResults() const
{
    QMutexLocker locker(&dataMutex_);
    return searchResults_;
}

void LogDataWorker::clearSearchResults()
{
    QMutexLocker locker(&dataMutex_);
    searchResults_.clear();
}

bool LogDataWorker::hasSearchResults() const
{
    QMutexLocker locker(&dataMutex_);
    return !searchResults_.isEmpty();
}

bool LogDataWorker::isSearching() const
{
    QMutexLocker locker(&dataMutex_);
    return isSearching_;
}

void LogDataWorker::doSearch()
{
    QMutexLocker locker(&dataMutex_);
    
    // Get the search parameters that were stored in searchInFile
    QString pattern = lastSearchPattern_;
    bool caseSensitive = lastSearchCaseSensitive_;
    bool inverse = lastSearchInverse_;
    bool boolean = lastSearchBoolean_;
    bool plainText = lastSearchPlainText_;
    
    // For now, search the entire file (we'll add line range later)
    int startIndex = 0;
    int endIndex = totalLines_ - 1;
    
    // Create regex pattern
    QRegularExpression regex;
    if (plainText) {
        regex = QRegularExpression(QRegularExpression::escape(pattern));
    } else {
        regex = QRegularExpression(pattern);
    }
    
    if (!caseSensitive) {
        regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    }
    
    if (!regex.isValid()) {
        LOG_ERROR("LogDataWorker::doSearch - Invalid regex pattern: " + regex.errorString());
        isSearching_ = false;
        emit searchFinished(false, 0);
        return;
    }
    
    // Search through the specified line range
    int totalLines = endIndex - startIndex + 1;
    int processedLines = 0;
    
    for (int i = startIndex; i <= endIndex; ++i) {
        if (searchInterruptRequest_) {
            LOG_DEBUG("LogDataWorker::doSearch - Search interrupted");
            break;
        }
        
        QString line = loadLineContent(i);
        
        bool match = regex.match(line).hasMatch();
        
        // Apply inverse logic
        if (inverse) {
            match = !match;
        }
        
        if (match) {
            searchResults_.append(i + 1); // Convert back to 1-based line numbers
        }
        
        processedLines++;
        
        // Report progress every 1000 lines
        if (processedLines % 1000 == 0) {
            int progress = (processedLines * 100) / totalLines;
            emit searchProgressed(progress);
        }
    }
    
    isSearching_ = false;
    int resultCount = searchResults_.size();
    
    LOG_INFO("LogDataWorker::doSearch - Found " + QString::number(resultCount) + " matches");
    emit searchFinished(true, resultCount);
}

void LogDataWorker::doIndexing()
{
    LOG_DEBUG("LogDataWorker::doIndexing - Starting indexing of: " + fileName_);
    emit progressMessage("Starting file indexing...");
    
    QElapsedTimer timer;
    timer.start();
    
    // STEP 2: Open file
    file_.setFileName(fileName_);
    LOG_DEBUG("LogDataWorker::doIndexing - Attempting to open file: " + fileName_);
    
    if (!file_.open(QIODevice::ReadOnly)) {
        QString errorMsg = file_.errorString();
        LOG_ERROR("LogDataWorker::doIndexing - Cannot open file: " + fileName_ + " - Error: " + errorMsg);
        emit progressMessage("ERROR: Cannot open file! Error: " + errorMsg);
        if (!isShuttingDown_) {
            emit indexingFinished(false);
        }
        return;
    }
    
    LOG_DEBUG("LogDataWorker::doIndexing - File opened successfully");
    
    fileSize_ = file_.size();
    LOG_DEBUG("LogDataWorker::doIndexing - File opened, size: " + QString::number(fileSize_ / (1024*1024)) + " MB");
    emit progressMessage(QString("File opened: %1 MB").arg(fileSize_ / (1024*1024)));
    
    // STEP 3: Initialize indexing
    LOG_DEBUG("LogDataWorker::doIndexing - Starting block processing");
    emit progressMessage("Starting block processing...");
    
    // Process file in blocks like KLOGG
    QByteArray buffer(IndexingBlockSize, Qt::Uninitialized);
    qint64 pos = 0;
    int lineCount = 0;
    int chunkNumber = 0;
    
    while (!file_.atEnd() && !interruptRequest_) {
        qint64 bytesRead = file_.read(buffer.data(), IndexingBlockSize);
        if (bytesRead <= 0) break;
        
        chunkNumber++;
        
        QByteArray block = buffer.left(static_cast<int>(bytesRead));
        parseDataBlock(pos, block);
        
        pos += bytesRead;
        lineCount = lineOffsets_.size(); // No need to subtract 1 since we don't append 0 initially
        
        // Update progress
        int progress = fileSize_ > 0 ? static_cast<int>((pos * 100) / fileSize_) : 100;
        emit indexingProgressed(progress);
        
        #if ULTRA_FAST_LOGGING
        // Ultra-fast progress logging - only log at completion
        if (progress == 100) {
            LOG_DEBUG("LogDataWorker::doIndexing - Progress: " + QString::number(progress) + "%, " + QString::number(lineCount) + " lines");
            emit progressMessage(QString("Progress: %1% - %2 lines processed").arg(progress).arg(lineCount));
        }
        #else
        // Standard progress logging - only log every 50% for minimal verbosity
        if (progress % 50 == 0 && chunkNumber % 100 == 0) {
            LOG_DEBUG("LogDataWorker::doIndexing - Progress: " + QString::number(progress) + "%, " + QString::number(lineCount) + " lines");
            emit progressMessage(QString("Progress: %1% - %2 lines processed").arg(progress).arg(lineCount));
        }
        #endif
    }
    
    // STEP 5: Handle non-LF terminated files like KLOGG
    if (!interruptRequest_ && fileSize_ > 0) {
        // Check if the file ends with a newline
        bool endsWithNewline = false;
        if (fileSize_ > 0) {
            file_.seek(fileSize_ - 1);
            char lastChar;
            if (file_.read(&lastChar, 1) == 1) {
                endsWithNewline = (lastChar == '\n');
            }
        }
        
        // If file doesn't end with newline, add the last line
        if (!endsWithNewline) {
            lineOffsets_.append(fileSize_);
            lineCount++;
        }
    }
    
    // STEP 6: Update final state
    {
        QMutexLocker locker(&dataMutex_);
        totalLines_ = lineCount;
        isFileLoaded_ = !interruptRequest_;
    }
    
    {
        QMutexLocker locker(&indexingMutex_);
        isIndexing_ = false;
    }
    
    qint64 elapsedMs = timer.elapsed();
    LOG_DEBUG("LogDataWorker::doIndexing - Indexing completed: " + QString::number(totalLines_) + " lines in " + QString::number(elapsedMs) + " ms");
    emit progressMessage(QString("Indexing completed: %1 lines in %2 ms").arg(totalLines_).arg(elapsedMs));
    
    if (!isShuttingDown_) {
        emit indexingFinished(isFileLoaded_);
    }
    
    if (isFileLoaded_) {
        LOG_DEBUG("LogDataWorker::doIndexing - Emitting viewportUpdateRequested");
        emit progressMessage("File loaded successfully - updating viewport...");
        emit viewportUpdateRequested();
    }
}

void LogDataWorker::parseDataBlock(qint64 blockBeginning, const QByteArray& block)
{
    int posWithinBlock = 0;
    int linesFoundInBlock = 0;
    
    while (posWithinBlock < block.size() && !interruptRequest_) {
        int nextLinePos = findNextLineFeed(block, posWithinBlock);
        
        if (nextLinePos == -1) {
            // No more line feeds in this block
            break;
        }
        
        // Calculate line end position
        qint64 lineEnd = blockBeginning + nextLinePos + 1; // +1 for newline
        lineOffsets_.append(lineEnd);
        
        linesFoundInBlock++;
        posWithinBlock = nextLinePos + 1;
    }
    
//    if (linesFoundInBlock > 0) {
//        LOG_DEBUG("STEP 4: Block processed -" + QString::number(linesFoundInBlock) + " lines found, total lines:" + QString::number(lineOffsets_.size() - 1));
//    }
}

int LogDataWorker::findNextLineFeed(const QByteArray& block, int posWithinBlock)
{
    // Simple implementation: find next '\n' character
    // KLOGG has more sophisticated handling for different encodings
    
    for (int i = posWithinBlock; i < block.size(); ++i) {
        if (block[i] == '\n') {
            return i;
        }
    }
    
    return -1; // No line feed found
}

QString LogDataWorker::loadLineContent(int lineIndex) {
    // Ultra-fast logging - only log every 1M lines for minimal spam
    static int callCount = 0;
    callCount++;
    
    #if ULTRA_FAST_LOGGING
    // Only log every 1M lines for ultra-minimal logging
    if (callCount % LOG_EVERY_N_LINES == 1) {
        LOG_DEBUG("LogDataWorker::loadLineContent - Processed " + QString::number(callCount) + " lines");
    }
    #endif
    
    QString result;
    
    if (lineIndex < 0 || lineIndex >= lineOffsets_.size() ) {
        return QString();
    }
    
    // Get line start and end positions using LinePositionArray
    qint64 lineStart = (lineIndex > 0) ? lineOffsets_[lineIndex - 1] : 0;
    qint64 lineEnd = lineOffsets_[lineIndex];
    
    // Seek to line start
    if (!file_.seek(lineStart)) {
        return QString();
    }
    
    // Read line content
    qint64 lineLength = lineEnd - lineStart;
    QByteArray lineData = file_.read(lineLength);
    
    // Remove trailing newline character if present
    if (!lineData.isEmpty() && lineData.endsWith('\n')) {
        lineData.chop(1);
    }
    
    // Sanitize NUL characters before decoding
    lineData.replace('\0', ' ');
    
    // Decode using the decoder
    if (decoder_) {
        result = decoder_->decode(lineData);
    } else {
        result = QString::fromUtf8(lineData);
    }
    
    return result;
}

QString LogDataWorker::loadEntireFileContent() {
    LOG_INFO("LogDataWorker::loadEntireFileContent - Starting bulk file load");
    
    if (!isFileLoaded_ || fileName_.isEmpty()) {
        LOG_ERROR("LogDataWorker::loadEntireFileContent - No file loaded");
        return QString();
    }
    
    QElapsedTimer timer;
    timer.start();
    
    // Open file for reading
    QFile file(fileName_);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR("LogDataWorker::loadEntireFileContent - Cannot open file: " + file.errorString());
        return QString();
    }
    
    qint64 fileSize = file.size();
    LOG_INFO("LogDataWorker::loadEntireFileContent - File size: " + QString::number(fileSize / (1024*1024)) + " MB");
    
    // Read entire file in one operation
    QByteArray fileData = file.readAll();
    qint64 readTime = timer.elapsed();
    
    LOG_INFO("LogDataWorker::loadEntireFileContent - File read completed: " + QString::number(fileData.size()) + " bytes in " + QString::number(readTime) + "ms");
    
    // Sanitize NUL characters
    QElapsedTimer sanitizeTimer;
    sanitizeTimer.start();
    fileData.replace('\0', ' ');
    qint64 sanitizeTime = sanitizeTimer.elapsed();
    
    LOG_INFO("LogDataWorker::loadEntireFileContent - NUL sanitization completed in " + QString::number(sanitizeTime) + "ms");
    
    // Decode using the decoder
    QString result;
    QElapsedTimer decodeTimer;
    decodeTimer.start();
    
    if (decoder_) {
        result = decoder_->decode(fileData);
    } else {
        result = QString::fromUtf8(fileData);
    }
    
    qint64 decodeTime = decodeTimer.elapsed();
    qint64 totalTime = timer.elapsed();
    
    LOG_INFO("LogDataWorker::loadEntireFileContent - Decoding completed in " + QString::number(decodeTime) + "ms");
    LOG_INFO("LogDataWorker::loadEntireFileContent - Total bulk load time: " + QString::number(totalTime) + "ms");
    
    return result;
}

// Search History Implementation
void LogDataWorker::addToSearchHistory(const QString& pattern, const QString& path) {
    QString historyEntry = QString("%1|%2").arg(pattern, path);
    
    // Remove if already exists (to move to top)
    searchHistory_.removeAll(historyEntry);
    
    // Add to beginning
    searchHistory_.prepend(historyEntry);
    
    // Keep only the last MaxSearchHistory entries
    if (searchHistory_.size() > MaxSearchHistory) {
        searchHistory_.resize(MaxSearchHistory);
    }
    
    // Save to file
    saveSearchHistory();
}

QStringList LogDataWorker::getSearchHistory() const {
    return searchHistory_;
}

void LogDataWorker::clearSearchHistory() {
    searchHistory_.clear();
    saveSearchHistory();
}

void LogDataWorker::saveSearchHistory() {
    QFile file("search_history.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const QString& entry : searchHistory_) {
            out << entry << "\n";
        }
        file.close();
    }
}

void LogDataWorker::loadSearchHistory() {
    QFile file("search_history.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        searchHistory_.clear();
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty()) {
                searchHistory_.append(line);
            }
        }
        file.close();
    }
} 