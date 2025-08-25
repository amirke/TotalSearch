#include "filelinemodel.h"
#include <QTextStream>
#include <QDebug>
#include <QFontMetrics>
#include <QFont>
#include <QApplication>
#include <QDateTime>
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include "logger.h"

#define LOG_TS qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
#define WARN_TS qWarning() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")

FileLineModel::FileLineModel(QObject *parent)
    : QAbstractListModel(parent) {
}

FileLineModel::~FileLineModel() {
    clear();
}

void FileLineModel::clear() {
    beginResetModel();
    m_file.close();
    m_lineOffsets.clear();
    m_filePath.clear();
    m_maxLineWidth = 0;
    m_maxLineLength = 0;
    m_segmentStart = 0;
    m_segmentEnd = 0;
    m_totalLines = 0;
    endResetModel();
}

bool FileLineModel::loadFile(const QString &filePath) {
    LOG_INFO("[FileLineModel] loadFile called with path: " + filePath);
    clear();
    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_ERROR("[FileLineModel] Failed to open file: " + filePath);
        return false;
    }
    LOG_INFO("[FileLineModel] File opened successfully: " + filePath);
    m_filePath = filePath;
    m_lineOffsets.clear();
    m_lineOffsets.push_back(0); // First line starts at 0
    
    // Load full file for display
    LOG_INFO("[FileLineModel] Loading full file for display...");
    int lineCount = 0;
    int maxLength = 0;
    
    // Index line offsets
    while (!m_file.atEnd()) {
        QByteArray line = m_file.readLine();
        QString lineStr = QString::fromUtf8(line).trimmed();
        int length = lineStr.length();
        maxLength = qMax(maxLength, length);
        m_lineOffsets.push_back(m_file.pos());
        ++lineCount;
    }
    m_file.seek(0);
    m_maxLineLength = maxLength;
    m_totalLines = lineCount;
    m_segmentStart = 0;
    m_segmentEnd = lineCount - 1;
    LOG_INFO("[FileLineModel] Finished loading full file. Total lines: " + QString::number(lineCount) + ", Max length: " + QString::number(maxLength));
    beginResetModel();
    endResetModel();
    return true;
}

bool FileLineModel::loadFileSegment(const QString &filePath, int targetLine) {
    LOG_INFO("[FileLineModel] loadFileSegment called with path: " + filePath + ", target line: " + QString::number(targetLine));
    
    // STEP 1: Check if we already have the right segment loaded
    // This prevents unnecessary reloading if user clicks multiple results in the same area
    if (isFileLoaded(filePath, targetLine)) {
        LOG_INFO("[FileLineModel] File already loaded with target line in segment - no reload needed");
        return true;
    }
    
    // STEP 2: Clear previous file data and prepare for new file
    clear();
    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_ERROR("[FileLineModel] Failed to open file: " + filePath);
        return false;
    }
    LOG_INFO("[FileLineModel] File opened successfully (FAST LOAD): " + filePath);
    m_filePath = filePath;
    m_lineOffsets.clear();
    m_lineOffsets.push_back(0); // First line starts at 0
    
    // STEP 3: Calculate segment bounds WITHOUT counting total lines
    // This is the key optimization - we don't need to know the total file size
    // We just load a window around the target line
    m_segmentStart = qMax(0, targetLine - 50);  // Start 50 lines before target
    m_segmentEnd = targetLine + 50;             // End 50 lines after target (no limit)
    
    LOG_INFO("[FileLineModel] Loading segment: lines " + QString::number(m_segmentStart) + " to " + QString::number(m_segmentEnd) + 
             " (target: " + QString::number(targetLine) + ")");
    LOG_INFO("[FileLineModel] PERFORMANCE: Skipped total line counting - loading segment directly");
    
    // STEP 4: Load the actual segment content
    QString segmentStartTime = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    LOG_INFO("[FileLineModel] Starting segment loading at " + segmentStartTime);
    
    // This method will:
    // - Skip to the start line
    // - Load lines until end line or end of file
    // - Build line offset index for fast access
    loadSegment(m_segmentStart, m_segmentEnd);
    
    QString segmentEndTime = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QDateTime segStart = QDateTime::fromString(segmentStartTime, "hh:mm:ss.zzz");
    QDateTime segEnd = QDateTime::fromString(segmentEndTime, "hh:mm:ss.zzz");
    qint64 segmentMs = segStart.msecsTo(segEnd);
    
    // STEP 5: Set total lines to actual loaded segment size
    // This is what the view will see - only the loaded segment, not the entire file
    m_totalLines = m_lineOffsets.size() - 1;
    
    LOG_INFO("[FileLineModel] Segment loading complete. Loaded " + QString::number(m_totalLines) + " lines in " + QString::number(segmentMs) + "ms");
    LOG_INFO("[FileLineModel] TIMING: Segment loading took " + QString::number(segmentMs) + "ms (no counting overhead)");
    
    // STEP 6: Notify Qt that the model data has changed
    // This triggers the view to update and show the new content
    beginResetModel();
    endResetModel();
    return true;
}

bool FileLineModel::isFileLoaded(const QString &filePath, int targetLine) const {
    // Check if the same file is already loaded
    if (m_filePath != filePath || !m_file.isOpen()) {
        return false;
    }
    
    // Check if target line is within the current segment
    if (targetLine >= m_segmentStart && targetLine <= m_segmentEnd) {
        return true;
    }
    
    return false;
}

void FileLineModel::loadSegment(int startLine, int endLine) {
    LOG_INFO("[FileLineModel] loadSegment called: startLine=" + QString::number(startLine) + ", endLine=" + QString::number(endLine));
    m_file.seek(0); // this is the first line of the file
    
    // STEP 1: Skip to the start line
    // We need to read through all lines before our target to reach the start position
    // This is necessary because we can't jump to a specific line without knowing its offset
    for (int i = 0; i < startLine; i++) {
        if (m_file.atEnd()) {
            LOG_INFO("[FileLineModel] loadSegment: Reached end of file while skipping to start line " + QString::number(startLine));
            return;
        }
        m_file.readLine(); // Read and discard lines we don't need
    }
    
    // STEP 2: Load the actual segment lines
    // Now we're at the start line, so we can read the lines we actually want
    int maxLength = 0;
    int actualLinesLoaded = 0;
    for (int i = startLine; i <= endLine && !m_file.atEnd(); i++) {
        // Read the line content
        QByteArray line = m_file.readLine();
        QString lineStr = QString::fromUtf8(line).trimmed();
        
        // Track the longest line for UI sizing
        int length = lineStr.length();
        maxLength = qMax(maxLength, length);
        
        // Store the file position after this line for fast access later
        m_lineOffsets.push_back(m_file.pos());
        actualLinesLoaded++;
    }
    
    // STEP 3: Update segment end to reflect actual lines loaded
    // If we hit end-of-file before reaching endLine, adjust the segment bounds
    m_segmentEnd = m_segmentStart + actualLinesLoaded - 1;
    
    // STEP 4: Update model statistics
    m_maxLineLength = qMax(m_maxLineLength, maxLength);
    LOG_INFO("[FileLineModel] loadSegment completed: loaded " + QString::number(actualLinesLoaded) + " lines, maxLength=" + QString::number(maxLength) + 
             ", actual segment range: " + QString::number(m_segmentStart) + " to " + QString::number(m_segmentEnd));
}

int FileLineModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_lineOffsets.size() > 0 ? m_lineOffsets.size() - 1 : 0;
}

QVariant FileLineModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) return QVariant();
    int row = index.row();
    if (row < 0 || row >= rowCount()) return QVariant();
    
    return getLine(row);
}

QString FileLineModel::getLine(int row) const {
    if (!m_file.isOpen() || row < 0 || row >= rowCount()) return QString();
    m_file.seek(m_lineOffsets[row]);
    QByteArray line = m_file.readLine();
    m_lastLine = QString::fromUtf8(line).trimmed();
    return m_lastLine;
}

int FileLineModel::getActualLineNumber(int row) const {
    if (row < 0 || row >= rowCount()) return -1;
    return m_segmentStart + row + 1; // +1 because line numbers are 1-based
}

int FileLineModel::lineCount() const {
    return rowCount();
} 