#include "scintillaedit.h"
#include "highlightdialog.h"
#include "logger.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QThread>
#include <QMutexLocker>
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QMessageBox>
#include <QElapsedTimer> // Added for fast search/highlight timing
#include <QRegularExpression>

#ifdef Q_OS_WIN
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#endif

namespace {
#ifdef Q_OS_WIN
bool openFileWithWindowsSharing(QFile* file)
{
    // Enable full sharing including FILE_SHARE_DELETE (like KLOGG)
    DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    int accessRights = GENERIC_READ;
    DWORD creationDisp = OPEN_EXISTING;
    
    // Create the file handle with Windows sharing
    SECURITY_ATTRIBUTES securityAtts = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE };
    HANDLE fileHandle = CreateFileW(
        (const wchar_t*)file->fileName().utf16(),
        accessRights,
        shareMode,
        &securityAtts,
        creationDisp,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (fileHandle != INVALID_HANDLE_VALUE) {
        // Convert HANDLE to file descriptor and pass to QFile
        int fd = _open_osfhandle((intptr_t)fileHandle, _O_RDONLY);
        if (fd != -1) {
            bool success = file->open(fd, QIODevice::ReadOnly, QFile::AutoCloseHandle);
            if (success) {
                qDebug() << "Opened file with Windows sharing:" << file->fileName();
                return true;
            } else {
                qWarning() << "Failed to open file with Windows sharing:" << file->fileName();
                ::CloseHandle(fileHandle);
            }
        } else {
            qWarning() << "Failed to get file descriptor for:" << file->fileName();
            ::CloseHandle(fileHandle);
        }
    } else {
        qWarning() << "Failed to create Windows file handle for:" << file->fileName();
    }
    
    // Fallback to normal QFile opening
    return file->open(QIODevice::ReadOnly);
}
#else
bool openFileWithWindowsSharing(QFile* file)
{
    // On non-Windows platforms, use normal file opening
    return file->open(QIODevice::ReadOnly);
}
#endif
}

ScintillaEdit::ScintillaEdit(QWidget *parent) : ScintillaEditBase(parent), m_highlightedLine(-1), m_lineOffset(0), m_showFileLineNumbers(true), m_loadedStartLine(0), m_loadedEndLine(0), m_chunkSize(KLOGG_INDEXING_BLOCK_SIZE), m_firstChunkSize(KLOGG_INDEXING_BLOCK_SIZE), m_loadingTimer(nullptr), m_fileStream(nullptr), m_progressiveFile(nullptr), m_totalLines(0), m_currentChunk(0), m_totalChunks(0), m_targetLine(0), m_contextLines(0), m_isProgressiveLoading(false), m_abortLoading(false), m_isIndexed(false), m_scrollTimer(nullptr), m_lastFirstVisibleLine(-1), m_lastVisibleLineCount(0), m_useViewportHighlighting(false), m_scrollingInProgress(false), m_cachedCaseSensitive(false), m_cachedHighlightSentence(false), m_cachedUseScintillaSearch(false), m_backgroundHighlightTimer(nullptr), m_idleTimer(nullptr), m_continuousTimer(nullptr), m_backgroundHighlightLine(0), m_backgroundChunkSize(1000), m_userActive(false), m_fullyHighlightedFile(false), m_backgroundHighlightingActive(false), m_backgroundCaseSensitive(false), m_backgroundHighlightSentence(false)
{

    
    // Initialize progressive loading timer
    m_loadingTimer = new QTimer(this);
    m_loadingTimer->setSingleShot(true);
    connect(m_loadingTimer, &QTimer::timeout, this, &ScintillaEdit::loadNextChunk);
    
    // Initialize scroll timer for viewport highlighting
    m_scrollTimer = new QTimer(this);
    m_scrollTimer->setSingleShot(true);
    m_scrollTimer->setInterval(150); // 150ms delay after scroll stops
    connect(m_scrollTimer, &QTimer::timeout, this, &ScintillaEdit::onScrollStopped);
    
    // Initialize background highlighting timers
    m_backgroundHighlightTimer = new QTimer(this);
    m_backgroundHighlightTimer->setSingleShot(true);
    m_backgroundHighlightTimer->setInterval(10); // 10ms for continuous operation when idle
    connect(m_backgroundHighlightTimer, &QTimer::timeout, this, &ScintillaEdit::onBackgroundHighlightChunk);
    
    m_idleTimer = new QTimer(this);
    m_idleTimer->setSingleShot(true);
    m_idleTimer->setInterval(2000); // 2 seconds idle before starting background highlighting
    connect(m_idleTimer, &QTimer::timeout, this, &ScintillaEdit::onUserIdle);
    
    m_continuousTimer = new QTimer(this);
    m_continuousTimer->setSingleShot(true);
    m_continuousTimer->setInterval(20); // 20ms for continuous processing when user is idle
    connect(m_continuousTimer, &QTimer::timeout, this, &ScintillaEdit::onBackgroundHighlightChunk);
    
    setupScintilla();
    setupDefaultStyle();
    
    // Connect signals
    connect(this, &ScintillaEditBase::doubleClick, this, &ScintillaEdit::onDoubleClick);
    connect(this, &ScintillaEditBase::modified, this, &ScintillaEdit::onModified);
    
    // Connect user activity detection for background highlighting
    connect(this, &ScintillaEdit::verticalScrolled, this, &ScintillaEdit::onUserActivity);
    connect(this, &ScintillaEditBase::modified, this, &ScintillaEdit::onUserActivity);
    connect(this, &ScintillaEditBase::doubleClick, this, &ScintillaEdit::onUserActivity);
    
    // Connect viewport highlighting scroll detection
    connect(this, &ScintillaEdit::verticalScrolled, this, &ScintillaEdit::onScrolled);
    

}

ScintillaEdit::~ScintillaEdit()
{

    
    // Clean up progressive loading resources
    if (m_fileStream) {
        delete m_fileStream;
        m_fileStream = nullptr;
    }
    if (m_progressiveFile) {
        m_progressiveFile->close();
        delete m_progressiveFile;
        m_progressiveFile = nullptr;
    }
}

void ScintillaEdit::setupScintilla()
{

    
    // Set up margins
    send(SCI_SETMARGINWIDTHN, 0, 50); // Line numbers margin
    send(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
    
    // Set up indicators for highlighting
    send(SCI_INDICSETSTYLE, 0, INDIC_BOX);
    // Use custom highlight color if set, otherwise use default light gray
    QColor defaultHighlightColor = m_highlightColor.isValid() ? m_highlightColor : QColor(224, 224, 224);
    send(SCI_INDICSETFORE, 0, defaultHighlightColor.rgb());
    send(SCI_INDICSETALPHA, 0, 100);
    
    // Set up search indicators
    send(SCI_INDICSETSTYLE, 1, INDIC_ROUNDBOX);
    send(SCI_INDICSETFORE, 1, 0xFFFF00); // Yellow
    send(SCI_INDICSETALPHA, 1, 100);
    
    // Enable scroll notifications for viewport highlighting
    send(SCI_SETMOUSEDWELLTIME, 100); // Enable notifications
    
    // Add a simple scroll detection mechanism using a timer
    QTimer *scrollDetectionTimer = new QTimer(this);
    scrollDetectionTimer->setInterval(100); // Check every 100ms
    connect(scrollDetectionTimer, &QTimer::timeout, [this]() {
        static int lastFirstVisibleLine = -1;
        int currentFirstVisibleLine = send(SCI_GETFIRSTVISIBLELINE);
        if (lastFirstVisibleLine != -1 && lastFirstVisibleLine != currentFirstVisibleLine) {
            // Always call onScrolled when scroll is detected, regardless of m_useViewportHighlighting
            onScrolled();
        }
        lastFirstVisibleLine = currentFirstVisibleLine;
    });
    scrollDetectionTimer->start();
    
    // Set up performance options
    send(SCI_SETBUFFEREDDRAW, 1);
    send(SCI_SETLAYOUTCACHE, SC_CACHE_PAGE);
    
    // Set up scroll width
    send(SCI_SETSCROLLWIDTH, 4000);
    send(SCI_SETSCROLLWIDTHTRACKING, 1); // Enable dynamic scroll width tracking
    
    // Configure tabs (4 spaces)
    send(SCI_SETTABWIDTH, 4);
    send(SCI_SETUSETABS, 0); // Use spaces instead of tabs
    
    // Set up styling
    send(SCI_STYLESETFONT, STYLE_DEFAULT, reinterpret_cast<sptr_t>("Consolas"));
    send(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
    send(SCI_STYLECLEARALL);
    

}

void ScintillaEdit::setupDefaultStyle()
{
    qDebug() << "ScintillaEdit: Setting up default styling";
    
    // Set default style
    send(SCI_STYLESETFONT, STYLE_DEFAULT, reinterpret_cast<sptr_t>("Consolas"));
    send(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
    send(SCI_STYLESETFORE, STYLE_DEFAULT, 0x000000); // Black text
    send(SCI_STYLESETBACK, STYLE_DEFAULT, 0xFFFFFF); // White background
    
    // Set selection style
    send(SCI_SETSELBACK, 1, 0x0000FF); // Blue selection
    send(SCI_SETSELFORE, 1, 0xFFFFFF); // White text on selection
    
    qDebug() << "ScintillaEdit: Default styling complete";
}

bool ScintillaEdit::loadFile(const QString &filePath, int targetLine, int contextLines)
{
    emit debugMessage("ScintillaEdit: Loading file: " + filePath + " target line: " + QString::number(targetLine));
    
    // Check if file exists
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QString error = "File does not exist: " + filePath;
        emit debugMessage("ScintillaEdit: " + error);
        emit fileLoadError(error);
        return false;
    }
    
    emit debugMessage("ScintillaEdit: File exists, size: " + QString::number(fileInfo.size()) + " bytes, readable: " + QString::number(fileInfo.isReadable()));
    
    // Log file size for debugging (no limit)
    emit debugMessage("ScintillaEdit: File size: " + QString::number(fileInfo.size()) + " bytes (" + QString::number(fileInfo.size() / 1024 / 1024) + "MB)");
    
    // Check if this is the same file and target line is already loaded
    if (filePath == m_currentFilePath && m_loadedStartLine > 0 && m_loadedEndLine > 0) {
        int newStartLine = qMax(1, targetLine - contextLines);
        int newEndLine = targetLine + contextLines;
        
        emit debugMessage("ScintillaEdit: Smart loading - current range: " + QString::number(m_loadedStartLine) + "-" + QString::number(m_loadedEndLine) + ", requested range: " + QString::number(newStartLine) + "-" + QString::number(newEndLine));
        
        // Check if we need to expand the loaded range
        if (newStartLine < m_loadedStartLine || newEndLine > m_loadedEndLine) {
            emit debugMessage("ScintillaEdit: Expanding loaded range for smart loading");
            
            // Calculate the actual range to expand to (union of current and new ranges)
            int expandedStartLine = qMin(m_loadedStartLine, newStartLine);
            int expandedEndLine = qMax(m_loadedEndLine, newEndLine);
            
            emit debugMessage("ScintillaEdit: Expanding to range: " + QString::number(expandedStartLine) + "-" + QString::number(expandedEndLine));
            
            expandLoadedRange(expandedStartLine, expandedEndLine);
            
            // Update line numbers display
            if (m_showFileLineNumbers) {
                updateLineNumbers();
            }
            
            // Scroll to the target line after expansion
            int displayLine = targetLine - m_lineOffset - 1;
            if (displayLine >= 0) {
                emit debugMessage("ScintillaEdit: Scrolling to target line after expansion, display line: " + QString::number(displayLine));
                
                // Get the position of the target line
                Scintilla::Position pos = send(SCI_POSITIONFROMLINE, displayLine);
                if (pos != INVALID_POSITION) {
                    // Scroll to the line and center it
                    send(SCI_GOTOPOS, pos);
                    send(SCI_SCROLLCARET);
                    
                    // Center the line in the view
                    int linesOnScreen = send(SCI_LINESONSCREEN);
                    int lineToShow = displayLine - (linesOnScreen / 2);
                    if (lineToShow < 0) lineToShow = 0;
                    
                    Scintilla::Position posToShow = send(SCI_POSITIONFROMLINE, lineToShow);
                    send(SCI_GOTOPOS, posToShow);
                    
                    emit debugMessage("ScintillaEdit: Target line centered in view");
                } else {
                    emit debugMessage("ScintillaEdit: Invalid position for target line");
                }
            } else {
                emit debugMessage("ScintillaEdit: Invalid display line for target: " + QString::number(targetLine) + " offset: " + QString::number(m_lineOffset));
            }
            
            emit fileLoaded(filePath);
            return true;
        } else {
            emit debugMessage("ScintillaEdit: Target line already in loaded range, no loading needed");
            // Just scroll to the line
            int displayLine = targetLine - m_lineOffset - 1;
            if (displayLine >= 0) {
                emit debugMessage("ScintillaEdit: Scrolling to existing target line, display line: " + QString::number(displayLine));
                send(SCI_GOTOPOS, send(SCI_POSITIONFROMLINE, displayLine));
                send(SCI_SCROLLCARET);
            }
            return true;
        }
    }
    
    // Use progressive loading for ALL files (no size threshold)
    emit debugMessage("ScintillaEdit: Using progressive loading for file: " + filePath);
    startProgressiveLoading(filePath, targetLine, contextLines);
    return true;
}

bool ScintillaEdit::loadFileKloggStyle(const QString &filePath, int targetLine, int contextLines)
{
    emit debugMessage("ScintillaEdit: Loading file with KLOGG style: " + filePath);
    
    // Check if file exists
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QString error = "File does not exist: " + filePath;
        emit debugMessage("ScintillaEdit: " + error);
        emit fileLoadError(error);
        return false;
    }
    
    emit debugMessage("ScintillaEdit: File exists, size: " + QString::number(fileInfo.size()) + " bytes");
    
    // Clear previous content and reset indexing
    send(SCI_CLEARALL);
    m_isIndexed = false;
    m_lineOffsets.clear();
    
    // Store file path - THIS WAS MISSING!
    m_currentFilePath = filePath;
    m_targetLine = targetLine;
    m_contextLines = contextLines;
    
    // Reset loaded range
    m_loadedStartLine = 0;
    m_loadedEndLine = 0;
    m_lineOffset = 0;
    
    // KLOGG-style: Start background indexing immediately
    emit debugMessage("ScintillaEdit: Starting KLOGG-style background indexing...");
    
    // Start background indexing in a separate thread (like KLOGG does)
    QThread* indexingThread = new QThread();
    QObject* worker = new QObject();
    worker->moveToThread(indexingThread);
    
    connect(indexingThread, &QThread::started, [worker, this, filePath]() {
        buildLineOffsetIndex(filePath);
        worker->deleteLater();
    });
    
    connect(indexingThread, &QThread::finished, [this, filePath, targetLine, contextLines, indexingThread]() {
        // Indexing completed, now load the initial content
        if (m_isIndexed) {
            emit debugMessage("ScintillaEdit: Background indexing completed, loading initial content");
            
            // Load initial content around target line using the index
            if (targetLine > 0) {
                int startLine = qMax(1, targetLine - contextLines);
                int endLine = targetLine + contextLines;
                
                emit debugMessage("ScintillaEdit: Loading lines " + QString::number(startLine) + " to " + QString::number(endLine) + " using KLOGG index");
                
                // Load the specific lines using the index
                loadLinesFromIndex(startLine, endLine);
                
                // Scroll to target line
                scrollToLine(targetLine);
                highlightLine(targetLine);
                
                m_loadedStartLine = startLine;
                m_loadedEndLine = endLine;
            } else {
                // No target line, load first chunk
                loadLinesFromIndex(1, KLOGG_SEARCH_BUFFER_SIZE_LINES);
                m_loadedStartLine = 1;
                m_loadedEndLine = KLOGG_SEARCH_BUFFER_SIZE_LINES;
            }
            
            emit fileLoaded(filePath);
        } else {
            emit debugMessage("ScintillaEdit: Background indexing failed, falling back to progressive loading");
            startProgressiveLoading(filePath, targetLine, contextLines);
        }
        
        indexingThread->deleteLater();
    });
    
    indexingThread->start();
    
    // Show immediate feedback (like KLOGG does)
    emit debugMessage("ScintillaEdit: File opening started, indexing in background...");
    
    return true;
}

void ScintillaEdit::scrollToLine(int lineNumber)
{
    qDebug() << "ScintillaEdit: Scrolling to line:" << lineNumber << "with offset:" << m_lineOffset;
    
    if (lineNumber < 0) {
        qWarning() << "ScintillaEdit: Invalid line number for scrolling:" << lineNumber;
        return;
    }
    
    // Get current scroll position before scrolling
    int currentFirstVisibleLine = send(SCI_GETFIRSTVISIBLELINE);
    qDebug() << "ScintillaEdit: Current first visible line:" << currentFirstVisibleLine;
    
    // Subtract 1 from target line (convert from 1-based to 0-based)
    int adjustedLineNumber = lineNumber - 1;
    
    // Adjust line number for the offset (convert from file line to display line)
    int displayLine = adjustedLineNumber - m_lineOffset;
    
    qDebug() << "ScintillaEdit: File line" << lineNumber << "adjusted to" << adjustedLineNumber << "maps to display line" << displayLine;
    qDebug() << "ScintillaEdit: m_lineOffset:" << m_lineOffset;
    
    if (displayLine < 0) {
        qWarning() << "ScintillaEdit: Line number" << lineNumber << "is before the loaded portion (offset:" << m_lineOffset << ")";
        return;
    }
    
    // Check total line count
    int totalLines = send(SCI_GETLINECOUNT);
    qDebug() << "ScintillaEdit: Total lines in Scintilla:" << totalLines;
    
    if (displayLine >= totalLines) {
        qWarning() << "ScintillaEdit: Display line number" << displayLine << "is beyond total lines" << totalLines;
        return;
    }
    
    // Get the position of the target line
    Scintilla::Position pos = send(SCI_POSITIONFROMLINE, displayLine);
    if (pos == INVALID_POSITION) {
        qWarning() << "ScintillaEdit: Invalid position for display line:" << displayLine;
        return;
    }
    
    // Get the number of lines visible on screen
    int linesOnScreen = send(SCI_LINESONSCREEN);
    qDebug() << "ScintillaEdit: Lines on screen:" << linesOnScreen;
    
    // Calculate the line to show at the top to center the target line
    int lineToShow = displayLine - (linesOnScreen / 2);
    qDebug() << "ScintillaEdit: Target display line:" << displayLine << ", calculated line to show:" << lineToShow;
    
    // Ensure we don't go below 0 or above the total lines
    if (lineToShow < 0) {
        lineToShow = 0;
        qDebug() << "ScintillaEdit: Adjusted lineToShow to 0 (was negative)";
    } else if (lineToShow + linesOnScreen > totalLines) {
        lineToShow = qMax(0, totalLines - linesOnScreen);
        qDebug() << "ScintillaEdit: Adjusted lineToShow to" << lineToShow << "(would exceed total lines)";
    }
    
    qDebug() << "ScintillaEdit: Final lineToShow:" << lineToShow << "out of" << totalLines << "total lines";
    
    // Get the position of the line to show at the top
    Scintilla::Position posToShow = send(SCI_POSITIONFROMLINE, lineToShow);
    if (posToShow != INVALID_POSITION) {
        // Try method 1: Scroll to center the target line using GOTOPOS
        send(SCI_GOTOPOS, posToShow);
        send(SCI_SCROLLCARET);
        
        qDebug() << "ScintillaEdit: Method 1 - GOTOPOS to position:" << posToShow;
        
        // Check if scroll actually worked
        int newFirstVisibleLine = send(SCI_GETFIRSTVISIBLELINE);
        qDebug() << "ScintillaEdit: New first visible line after GOTOPOS:" << newFirstVisibleLine;
        
        if (newFirstVisibleLine != lineToShow) {
            qWarning() << "ScintillaEdit: GOTOPOS failed! Expected first visible line:" << lineToShow << "but got:" << newFirstVisibleLine;
            
            // Try method 2: Use LINESCROLL
            qDebug() << "ScintillaEdit: Trying method 2 - LINESCROLL";
            send(SCI_LINESCROLL, 0, lineToShow);
            
            int newFirstVisibleLine2 = send(SCI_GETFIRSTVISIBLELINE);
            qDebug() << "ScintillaEdit: New first visible line after LINESCROLL:" << newFirstVisibleLine2;
            
            if (newFirstVisibleLine2 != lineToShow) {
                qWarning() << "ScintillaEdit: LINESCROLL also failed! Expected:" << lineToShow << "but got:" << newFirstVisibleLine2;
                
                // Try method 3: Use SETFIRSTVISIBLELINE
                qDebug() << "ScintillaEdit: Trying method 3 - SETFIRSTVISIBLELINE";
                send(SCI_SETFIRSTVISIBLELINE, lineToShow);
                
                int newFirstVisibleLine3 = send(SCI_GETFIRSTVISIBLELINE);
                qDebug() << "ScintillaEdit: New first visible line after SETFIRSTVISIBLELINE:" << newFirstVisibleLine3;
                
                if (newFirstVisibleLine3 != lineToShow) {
                    qWarning() << "ScintillaEdit: All scroll methods failed! Expected:" << lineToShow << "but got:" << newFirstVisibleLine3;
                } else {
                    qDebug() << "ScintillaEdit: SETFIRSTVISIBLELINE successful!";
                }
            } else {
                qDebug() << "ScintillaEdit: LINESCROLL successful!";
            }
        } else {
            qDebug() << "ScintillaEdit: GOTOPOS successful! First visible line is now:" << newFirstVisibleLine;
        }
        
        qDebug() << "ScintillaEdit: Successfully scrolled to center line" << displayLine << "in viewport (showing line" << lineToShow << "at top)";
        qDebug() << "ScintillaEdit: Position scrolled to:" << posToShow;
    } else {
        qWarning() << "ScintillaEdit: Invalid position for line to show:" << lineToShow;
    }
}

void ScintillaEdit::highlightLine(int lineNumber)
{
    qDebug() << "ScintillaEdit: Highlighting line:" << lineNumber;
    
    // Clear previous highlight
    clearHighlight();
    
    if (lineNumber < 0) {
        qWarning() << "ScintillaEdit: Invalid line number for highlighting:" << lineNumber;
        return;
    }
    
    // Subtract 1 from target line (convert from 1-based to 0-based)
    int adjustedLineNumber = lineNumber - 1;
    
    // Adjust line number for the offset (convert from file line to display line)
    int displayLine = adjustedLineNumber - m_lineOffset;
    
    qDebug() << "ScintillaEdit: File line" << lineNumber << "adjusted to" << adjustedLineNumber << "maps to display line" << displayLine;
    
    if (displayLine < 0) {
        qWarning() << "ScintillaEdit: Line number" << lineNumber << "is before the loaded portion (offset:" << m_lineOffset << ")";
        return;
    }
    
    // Check total line count
    int totalLines = send(SCI_GETLINECOUNT);
    if (displayLine >= totalLines) {
        qWarning() << "ScintillaEdit: Display line number" << displayLine << "is beyond total lines" << totalLines;
        return;
    }
    
    // Get line start and end positions
    Scintilla::Position lineStart = send(SCI_POSITIONFROMLINE, displayLine);
    Scintilla::Position lineEnd = send(SCI_GETLINEENDPOSITION, displayLine);
    
    if (lineStart == INVALID_POSITION || lineEnd == INVALID_POSITION) {
        qWarning() << "ScintillaEdit: Invalid line positions for display line:" << displayLine;
        return;
    }
    
    // Use custom highlight color if set, otherwise use default gray
    QColor highlightColor = m_highlightColor.isValid() ? m_highlightColor : QColor(224, 224, 224);
//    LOG_INFO("ScintillaEdit: Using highlight color: " + highlightColor.name() + " RGB:" + QString::number(highlightColor.rgb()));
    
    // Configure indicator with the highlight color
    send(SCI_INDICATORCLEARRANGE, 0, send(SCI_GETTEXTLENGTH));
    send(SCI_SETINDICATORCURRENT, 0);
    send(SCI_INDICSETSTYLE, 0, INDIC_FULLBOX);
    
    // Convert QColor to Scintilla RGB format (BGR)
    int r = highlightColor.red();
    int g = highlightColor.green();
    int b = highlightColor.blue();
    int scintillaColor = (b << 16) | (g << 8) | r;  // BGR format for Scintilla
    
    send(SCI_INDICSETFORE, 0, scintillaColor);
    send(SCI_INDICSETALPHA, 0, 100);
    
  //  LOG_INFO("ScintillaEdit: Color conversion - QColor RGB(" + QString::number(r) + "," + QString::number(g) + "," + QString::number(b) + 
  //           ") -> Scintilla BGR: " + QString::number(scintillaColor));
    
    // Apply highlight
    send(SCI_INDICATORFILLRANGE, lineStart, lineEnd - lineStart);
    
    m_highlightedLine = lineNumber; // Store the original file line number
    
    qDebug() << "ScintillaEdit: Display line highlighted:" << displayLine << "with color:" << highlightColor.name();
}

void ScintillaEdit::clearHighlight()
{
    if (m_highlightedLine >= 0) {
        qDebug() << "ScintillaEdit: Clearing highlight from line:" << m_highlightedLine;
        
        // Clear indicator
        send(SCI_SETINDICATORCURRENT, 0);
        send(SCI_INDICATORCLEARRANGE, 0, send(SCI_GETTEXTLENGTH));
        
        m_highlightedLine = -1;
    }
}

int ScintillaEdit::getFileLineNumber(int displayLine) const
{
    return m_lineOffset + displayLine + 1;
}

bool ScintillaEdit::isLineInLoadedRange(int lineNumber) const
{
    return lineNumber >= m_loadedStartLine && lineNumber <= m_loadedEndLine;
}

void ScintillaEdit::expandLoadedRange(int newStartLine, int newEndLine)
{
    qDebug() << "ScintillaEdit: Expanding loaded range from" << m_loadedStartLine << "-" << m_loadedEndLine << "to" << newStartLine << "-" << newEndLine;
    
    // Determine what needs to be loaded
    bool needLoadBefore = newStartLine < m_loadedStartLine;
    bool needLoadAfter = newEndLine > m_loadedEndLine;
    
    if (!needLoadBefore && !needLoadAfter) {
        qDebug() << "ScintillaEdit: No expansion needed, line already in loaded range";
        return;
    }
    
    // Load additional content from file
    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ScintillaEdit: Failed to open file for range expansion";
        return;
    }
    
    QTextStream stream(&file);
    QString beforeContent;
    QString afterContent;
    int currentLine = 1;
    
    // Load content before current range if needed
    if (needLoadBefore) {
        qDebug() << "ScintillaEdit: Loading additional content before line" << m_loadedStartLine;
        while (currentLine < m_loadedStartLine && !stream.atEnd()) {
            QString line = stream.readLine();
            if (currentLine >= newStartLine) {
                beforeContent += line + "\n";
            }
            currentLine++;
        }
        qDebug() << "ScintillaEdit: Loaded" << beforeContent.count('\n') << "lines before current range";
    }
    
    // Skip current loaded content
    while (currentLine <= m_loadedEndLine && !stream.atEnd()) {
        stream.readLine();
        currentLine++;
    }
    
    // Load content after current range if needed
    if (needLoadAfter) {
        qDebug() << "ScintillaEdit: Loading additional content after line" << m_loadedEndLine;
        while (currentLine <= newEndLine && !stream.atEnd()) {
            QString line = stream.readLine();
            afterContent += line + "\n";
            currentLine++;
        }
        qDebug() << "ScintillaEdit: Loaded" << afterContent.count('\n') << "lines after current range";
    }
    
    file.close();
    
    // Insert the additional content
    bool contentInserted = false;
    
    if (!beforeContent.isEmpty()) {
        qDebug() << "ScintillaEdit: Inserting" << beforeContent.count('\n') << "lines at beginning";
        
        // Insert at the beginning
        send(SCI_SETREADONLY, 0);
        QByteArray utf8Content = beforeContent.toUtf8();
        send(SCI_INSERTTEXT, 0, reinterpret_cast<sptr_t>(utf8Content.data()));
        send(SCI_SETREADONLY, 1);
        
        // Update line offset and loaded range
        int additionalLines = beforeContent.count('\n');
        m_lineOffset -= additionalLines;
        m_loadedStartLine = newStartLine;
        contentInserted = true;
        
        qDebug() << "ScintillaEdit: Inserted" << additionalLines << "lines at beginning, new offset:" << m_lineOffset;
    }
    
    if (!afterContent.isEmpty()) {
        qDebug() << "ScintillaEdit: Appending" << afterContent.count('\n') << "lines at end";
        
        // Append at the end
        send(SCI_SETREADONLY, 0);
        Scintilla::Position endPos = send(SCI_GETTEXTLENGTH);
        QByteArray utf8Content = afterContent.toUtf8();
        send(SCI_INSERTTEXT, endPos, reinterpret_cast<sptr_t>(utf8Content.data()));
        send(SCI_SETREADONLY, 1);
        
        m_loadedEndLine = newEndLine;
        contentInserted = true;
        
        int additionalLines = afterContent.count('\n');
        qDebug() << "ScintillaEdit: Appended" << additionalLines << "lines at end";
    }
    
    if (contentInserted) {
        // Force a refresh of the widget
        send(SCI_COLOURISE, 0, -1);
        update();
        
        qDebug() << "ScintillaEdit: Range expanded successfully. New range:" << m_loadedStartLine << "-" << m_loadedEndLine;
        qDebug() << "ScintillaEdit: New line offset:" << m_lineOffset;
        qDebug() << "ScintillaEdit: Total lines in display:" << send(SCI_GETLINECOUNT);
    } else {
        qDebug() << "ScintillaEdit: No content to insert";
    }
}

QString ScintillaEdit::getText() const
{
    Scintilla::Position length = send(SCI_GETTEXTLENGTH);
    if (length == 0) return QString();
    
    QByteArray buffer(length + 1, 0);
    send(SCI_GETTEXT, length + 1, reinterpret_cast<sptr_t>(buffer.data()));
    
    return QString::fromUtf8(buffer);
}

void ScintillaEdit::setText(const QString &text)
{
    qDebug() << "ScintillaEdit: Setting text, length:" << text.length();
    
    QByteArray utf8Data = text.toUtf8();
    send(SCI_SETTEXT, 0, reinterpret_cast<sptr_t>(utf8Data.data()));
    
    emit textChanged();
}

void ScintillaEdit::setUtf8Bytes(const char* data, int length)
{
    if (data == nullptr || length <= 0) {
        qWarning() << "ScintillaEdit::setUtf8Bytes: invalid input";
        return;
    }
    // Note: SCI_SETTEXT expects a NUL-terminated buffer; for raw bytes of known length,
    // we prefer SCI_ADDTEXT after clearing or SCI_SETREADONLY/SCI_CLEARALL + SCI_ADDTEXT.
    send(SCI_CLEARALL);
    send(SCI_ADDTEXT, length, reinterpret_cast<sptr_t>(data));
    emit textChanged();
}

void ScintillaEdit::appendText(const QString &text)
{
    qDebug() << "ScintillaEdit: Appending text, length:" << text.length();
    
    Scintilla::Position length = send(SCI_GETTEXTLENGTH);
    send(SCI_GOTOPOS, length);
    
    QByteArray utf8Data = text.toUtf8();
    send(SCI_ADDTEXT, utf8Data.length(), reinterpret_cast<sptr_t>(utf8Data.data()));
    
    emit textChanged();
}

void ScintillaEdit::clearText()
{
    qDebug() << "ScintillaEdit: Clearing text";
    
    send(SCI_CLEARALL);
    emit textChanged();
}

int ScintillaEdit::lineCount() const
{
    return send(SCI_GETLINECOUNT);
}

void ScintillaEdit::setReadOnly(bool readOnly)
{
    qDebug() << "ScintillaEdit: Setting read-only:" << readOnly;
    send(SCI_SETREADONLY, readOnly ? 1 : 0);
}

void ScintillaEdit::setLineNumbers(bool show)
{
    qDebug() << "ScintillaEdit: Setting line numbers:" << show;
    send(SCI_SETMARGINWIDTHN, 0, show ? 50 : 0);
}

void ScintillaEdit::setWrapMode(bool wrap)
{
    qDebug() << "ScintillaEdit: Setting wrap mode:" << wrap;
    send(SCI_SETWRAPMODE, wrap ? SC_WRAP_WORD : SC_WRAP_NONE);
}

void ScintillaEdit::setFont(const QString &fontFamily, int fontSize)
{
    qDebug() << "ScintillaEdit: Setting font to" << fontFamily << "size" << fontSize;
    
    // Set font for default style
    send(SCI_STYLESETFONT, STYLE_DEFAULT, reinterpret_cast<sptr_t>(fontFamily.toUtf8().data()));
    send(SCI_STYLESETSIZE, STYLE_DEFAULT, fontSize);
    
    // Set font for line numbers
    send(SCI_STYLESETFONT, STYLE_LINENUMBER, reinterpret_cast<sptr_t>(fontFamily.toUtf8().data()));
    send(SCI_STYLESETSIZE, STYLE_LINENUMBER, fontSize);
    
    // Set font for all other styles
    for (int style = 0; style <= STYLE_MAX; style++) {
        send(SCI_STYLESETFONT, style, reinterpret_cast<sptr_t>(fontFamily.toUtf8().data()));
        send(SCI_STYLESETSIZE, style, fontSize);
    }
    
    qDebug() << "ScintillaEdit: Font set successfully";
}

void ScintillaEdit::setTabWidth(int width, bool useSpaces)
{
    qDebug() << "ScintillaEdit: Setting tab width to" << width << "spaces, use spaces:" << useSpaces;
    
    // Set tab width
    send(SCI_SETTABWIDTH, width);
    
    // Set whether to use spaces or tabs
    send(SCI_SETUSETABS, useSpaces ? 0 : 1);
    
    qDebug() << "ScintillaEdit: Tab settings applied";
}

void ScintillaEdit::setHighlightColor(const QColor &color)
{
    m_highlightColor = color;
//    LOG_INFO("ScintillaEdit: Highlight color set to " + color.name() + " RGB:" + QString::number(color.rgb()));
}

void ScintillaEdit::setShowFileLineNumbers(bool show)
{
    m_showFileLineNumbers = show;
    qDebug() << "ScintillaEdit: Show file line numbers set to" << show;
    
    if (show) {
        // Enable custom line number display
        send(SCI_SETMARGINWIDTHN, 1, 80); // Custom margin for 8-digit file line numbers
        send(SCI_SETMARGINTYPEN, 1, SC_MARGIN_TEXT);
        updateLineNumbers();
    } else {
        // Disable custom line number display
        send(SCI_SETMARGINWIDTHN, 1, 0);
        send(SCI_MARGINTEXTCLEARALL);
    }
}

void ScintillaEdit::updateLineNumbers()
{
    if (!m_showFileLineNumbers) {
        return; // No need to update if not showing file line numbers
    }
    
    qDebug() << "ScintillaEdit: Updating line numbers with offset" << m_lineOffset;
    
    // Get the total number of lines in the current display
    int totalLines = send(SCI_GETLINECOUNT);
    
    // Create a custom margin to show file line numbers
    // We'll use margin 1 for custom line numbers
    send(SCI_SETMARGINWIDTHN, 1, 80); // Make it wider for 8-digit line numbers
    send(SCI_SETMARGINTYPEN, 1, SC_MARGIN_TEXT);
    
    // Clear any existing text in the margin
    send(SCI_MARGINTEXTCLEARALL);
    
    // Add file line numbers to the margin
    for (int i = 0; i < totalLines; i++) {
        int fileLineNumber = m_lineOffset + i + 1;
        QString lineNumberText = QString::number(fileLineNumber);
        
        // Always show the calculated line number, even if it's the same as display line
        // This ensures users can see the actual file line numbers
        
        // Set the text for this line in the margin
        QByteArray textData = lineNumberText.toUtf8();
        send(SCI_MARGINSETTEXT, i, reinterpret_cast<sptr_t>(textData.data()));
        
        // Style the margin text
        send(SCI_MARGINSETSTYLE, i, 1); // Use style 1 for margin text
    }
    
    // Set up margin text styling
    send(SCI_STYLESETFONT, STYLE_LINENUMBER, reinterpret_cast<sptr_t>("Consolas"));
    send(SCI_STYLESETSIZE, STYLE_LINENUMBER, 9);
    send(SCI_STYLESETFORE, STYLE_LINENUMBER, 0x808080); // Gray color
    
    qDebug() << "ScintillaEdit: Updated line numbers for" << totalLines << "lines with offset" << m_lineOffset;
}

void ScintillaEdit::findText(const QString &text, bool caseSensitive, bool wholeWord)
{
    qDebug() << "ScintillaEdit: Finding text:" << text;
    
    // Clear previous search indicators
    clearSearchIndicators();
    
    if (text.isEmpty()) return;
    
    // Set search flags
    int searchFlags = 0;
    if (caseSensitive) searchFlags |= SCFIND_MATCHCASE;
    if (wholeWord) searchFlags |= SCFIND_WHOLEWORD;
    
    send(SCI_SETSEARCHFLAGS, searchFlags);
    
    // Find all occurrences
    Scintilla::Position pos = 0;
    Scintilla::Position length = send(SCI_GETTEXTLENGTH);
    
    QByteArray searchData = text.toUtf8();
    
    while (pos < length) {
        Scintilla::Position found = send(SCI_SEARCHNEXT, searchData.length(), reinterpret_cast<sptr_t>(searchData.data()));
        if (found == INVALID_POSITION) break;
        
        Scintilla::Position end = send(SCI_GETTARGETEND);
        
        // Set indicator
        send(SCI_SETINDICATORCURRENT, 1);
        send(SCI_INDICATORFILLRANGE, found, end - found);
        
        pos = end;
    }
}

void ScintillaEdit::replaceText(const QString &findText, const QString &replaceText, bool caseSensitive)
{
    qDebug() << "ScintillaEdit: Replacing text:" << findText << "with:" << replaceText;
    
    if (findText.isEmpty()) return;
    
    // Set search flags
    int searchFlags = 0;
    if (caseSensitive) searchFlags |= SCFIND_MATCHCASE;
    send(SCI_SETSEARCHFLAGS, searchFlags);
    
    // Replace all occurrences
    QByteArray findData = findText.toUtf8();
    QByteArray replaceData = replaceText.toUtf8();
    
    send(SCI_TARGETWHOLEDOCUMENT);
    send(SCI_REPLACETARGET, replaceData.length(), reinterpret_cast<sptr_t>(replaceData.data()));
}

void ScintillaEdit::clearSearchIndicators()
{
    qDebug() << "ScintillaEdit: Clearing search indicators";
    send(SCI_SETINDICATORCURRENT, 1);
    send(SCI_INDICATORCLEARRANGE, 0, send(SCI_GETTEXTLENGTH));
}

void ScintillaEdit::fastSearchAndHighlight(const QString &pattern, bool caseSensitive, bool wholeWord, bool useRegex)
{
    qDebug() << "ScintillaEdit: Fast search and highlight - Pattern:" << pattern;
    LOG_INFO("ScintillaEdit: Fast search and highlight - Pattern: " + pattern);
    
    // Clear previous search indicators
    clearSearchIndicators();
    
    if (pattern.isEmpty()) {
        LOG_INFO("ScintillaEdit: Empty pattern, clearing highlights");
        return;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    // Set search flags
    int searchFlags = 0;
    if (caseSensitive) searchFlags |= SCFIND_MATCHCASE;
    if (wholeWord) searchFlags |= SCFIND_WHOLEWORD;
    if (useRegex) searchFlags |= SCFIND_REGEXP;
    
    send(SCI_SETSEARCHFLAGS, searchFlags);
    
    // Configure indicator for highlighting
    send(SCI_SETINDICATORCURRENT, 1);
    send(SCI_INDICSETSTYLE, 1, INDIC_FULLBOX);
    send(SCI_INDICSETFORE, 1, m_highlightColor.rgb());
    send(SCI_INDICSETALPHA, 1, 100);
    
    // Find all occurrences
    QByteArray searchData = pattern.toUtf8();
    Scintilla::Position pos = 0;
    Scintilla::Position length = send(SCI_GETTEXTLENGTH);
    int matchCount = 0;
    
    while (pos < length) {
        Scintilla::Position found = send(SCI_SEARCHNEXT, searchData.length(), 
                                        reinterpret_cast<sptr_t>(searchData.data()));
        if (found == INVALID_POSITION) break;
        
        Scintilla::Position end = send(SCI_GETTARGETEND);
        
        // Highlight the match
        send(SCI_INDICATORFILLRANGE, found, end - found);
        
        pos = end;
        matchCount++;
    }
    
    qint64 searchTime = timer.elapsed();
    LOG_INFO("ScintillaEdit: Fast search completed in " + QString::number(searchTime) + "ms with " + QString::number(matchCount) + " matches");
    qDebug() << "ScintillaEdit: Fast search completed in" << searchTime << "ms with" << matchCount << "matches";
}

void ScintillaEdit::highlightSearchResults(const QList<QPair<int, int>> &matches)
{
    qDebug() << "ScintillaEdit: Highlighting" << matches.size() << "search results";
    LOG_INFO("ScintillaEdit: Highlighting " + QString::number(matches.size()) + " search results");
    
    // Clear previous search indicators
    clearSearchIndicators();
    
    if (matches.isEmpty()) {
        return;
    }
    
    // Configure indicator for highlighting
    send(SCI_SETINDICATORCURRENT, 1);
    send(SCI_INDICSETSTYLE, 1, INDIC_FULLBOX);
    send(SCI_INDICSETFORE, 1, m_highlightColor.rgb());
    send(SCI_INDICSETALPHA, 1, 100);
    
    // Highlight each match
    for (const auto &match : matches) {
        int start = match.first;
        int length = match.second;
        
        if (start >= 0 && length > 0) {
            send(SCI_INDICATORFILLRANGE, start, length);
        }
    }
    
    LOG_INFO("ScintillaEdit: Search results highlighting completed");
}

QString ScintillaEdit::convertToUtf8(const QByteArray &data)
{
    // Simple UTF-8 conversion - assume the data is already UTF-8 or can be converted
    // Check for BOM
    if (data.startsWith("\xEF\xBB\xBF")) {
        // UTF-8 BOM, remove it
        return QString::fromUtf8(data.mid(3));
    } else if (data.startsWith("\xFF\xFE")) {
        // UTF-16LE BOM
        return QString::fromUtf16(reinterpret_cast<const ushort*>(data.constData() + 2), (data.size() - 2) / 2);
    } else if (data.startsWith("\xFE\xFF")) {
        // UTF-16BE BOM
        QByteArray swapped = data.mid(2);
        for (int i = 0; i < swapped.size() - 1; i += 2) {
            std::swap(swapped[i], swapped[i + 1]);
        }
        return QString::fromUtf16(reinterpret_cast<const ushort*>(swapped.constData()), swapped.size() / 2);
    }
    
    // Try UTF-8 first
    QString result = QString::fromUtf8(data);
    if (!result.contains(QChar::ReplacementCharacter)) {
        return result;
    }
    
    // If UTF-8 fails, try system locale
    return QString::fromLocal8Bit(data);
}

void ScintillaEdit::onDoubleClick(Scintilla::Position position, Scintilla::Position line)
{
    int displayLine = static_cast<int>(line);
    int fileLine = m_lineOffset + displayLine + 1;
    
    qDebug() << "ScintillaEdit: Double click on display line" << displayLine << "-> file line" << fileLine;
    
    emit lineClicked(displayLine);
    emit fileLineNumberChanged(fileLine);
}

void ScintillaEdit::onModified(Scintilla::ModificationFlags type, Scintilla::Position position, 
                               Scintilla::Position length, Scintilla::Position linesAdded, 
                               const QByteArray &text, Scintilla::Position line, 
                               Scintilla::FoldLevel foldNow, Scintilla::FoldLevel foldPrev)
{
    // Handle text modification events
    int typeInt = static_cast<int>(type);
    if ((typeInt & SC_MOD_INSERTTEXT) || (typeInt & SC_MOD_DELETETEXT)) {
        emit textChanged();
    }
}

void ScintillaEdit::onScrolled()
{
    LOG_INFO("ScintillaEdit: onScrolled() called - scroll detected");
    
    // Mark scrolling in progress but DON'T clear highlights immediately
    // This prevents blinking - we'll update highlights when scrolling stops
    if (!m_scrollingInProgress) {
        m_scrollingInProgress = true;
        LOG_INFO("ScintillaEdit: Scrolling started");
    }
    
    // Start/restart timer to detect when scrolling stops
    m_scrollTimer->start();
    LOG_INFO("ScintillaEdit: Scroll timer started (150ms timeout)");
}

void ScintillaEdit::onScrollStopped()
{
 //   if (!m_useViewportHighlighting) return;
    
    // Reset scrolling flag
    m_scrollingInProgress = false;
    LOG_INFO("ScintillaEdit: Scroll stopped");
    
    // Always try to highlight new visible area (the function will check if already highlighted)
    if (!m_lastAppliedViewportRules.isEmpty()) {
        LOG_INFO("ScintillaEdit: Scroll stopped, checking if new area needs highlighting");
        LOG_INFO("ScintillaEdit: Using cached settings - Case sensitive: " + QString(m_cachedCaseSensitive ? "Yes" : "No") + 
                 ", Highlight sentence: " + QString(m_cachedHighlightSentence ? "Yes" : "No") + 
                 ", Use Scintilla search: " + QString(m_cachedUseScintillaSearch ? "Yes" : "No"));
        highlightViewportOnly(m_lastAppliedViewportRules, m_cachedCaseSensitive, m_cachedHighlightSentence, m_cachedUseScintillaSearch);
    }
}

void ScintillaEdit::setChunkSize(int chunkSize)
{
    if (chunkSize >= KLOGG_INDEXING_BLOCK_SIZE && chunkSize <= 100 * 1024 * 1024) { // 5MB to 100MB
        m_chunkSize = chunkSize;
        emit debugMessage("ScintillaEdit: Chunk size set to " + QString::number(chunkSize) + " bytes (KLOGG-style)");
    } else {
        emit debugMessage("ScintillaEdit: Invalid chunk size " + QString::number(chunkSize) + ", using KLOGG default " + QString::number(KLOGG_INDEXING_BLOCK_SIZE));
        m_chunkSize = KLOGG_INDEXING_BLOCK_SIZE;
    }
}

void ScintillaEdit::setFirstChunkSize(int firstChunkSize)
{
    if (firstChunkSize >= KLOGG_INDEXING_BLOCK_SIZE && firstChunkSize <= 100 * 1024 * 1024) { // 5MB to 100MB
        m_firstChunkSize = firstChunkSize;
        emit debugMessage("ScintillaEdit: First chunk size set to " + QString::number(firstChunkSize) + " bytes (KLOGG-style)");
    } else {
        emit debugMessage("ScintillaEdit: Invalid first chunk size " + QString::number(firstChunkSize) + ", using KLOGG default " + QString::number(KLOGG_INDEXING_BLOCK_SIZE));
        m_firstChunkSize = KLOGG_INDEXING_BLOCK_SIZE;
    }
}

void ScintillaEdit::startProgressiveLoading(const QString &filePath, int targetLine, int contextLines)
{
    emit debugMessage("ScintillaEdit: Starting progressive loading for: " + filePath);
    
    // Abort any existing progressive loading
    abortProgressiveLoading();
    
    // Reset abort flag for new loading
    m_abortLoading = false;
    
    // Store parameters
    m_currentFilePath = filePath;
    m_targetLine = targetLine;
    m_contextLines = contextLines;
    m_isProgressiveLoading = true;
    
    // Open file for reading with Windows file sharing
    m_progressiveFile = new QFile(filePath);
    if (!openFileWithWindowsSharing(m_progressiveFile)) {
        QString error = "Failed to open file for progressive loading: " + filePath;
        emit debugMessage("ScintillaEdit: " + error);
        emit fileLoadError(error);
        delete m_progressiveFile;
        m_progressiveFile = nullptr;
        m_isProgressiveLoading = false;
        return;
    }
    
    m_fileStream = new QTextStream(m_progressiveFile);
    m_fileStream->setEncoding(QStringConverter::Utf8);
    
    // Count total lines first
    emit debugMessage("ScintillaEdit: Counting total lines...");
    m_totalLines = 0;
    QTextStream countStream(m_progressiveFile);
    while (!countStream.atEnd()) {
        countStream.readLine();
        m_totalLines++;
        
     //    if (m_totalLines % 500000 == 0) {
      //       emit debugMessage("ScintillaEdit: Counted " + QString::number(m_totalLines) + " lines so far...");
     //    }
    }
    
    emit debugMessage("ScintillaEdit: Total lines in file: " + QString::number(m_totalLines));
    
    // Calculate total chunks needed
    m_totalChunks = 1; // First chunk
    int remainingLines = m_totalLines - m_firstChunkSize;
    if (remainingLines > 0) {
        m_totalChunks += (remainingLines + m_chunkSize - 1) / m_chunkSize; // Ceiling division
    }
    
    m_currentChunk = 0;
    
    emit debugMessage("ScintillaEdit: Will load " + QString::number(m_totalChunks) + " chunks (first: " + QString::number(m_firstChunkSize) + " lines, others: " + QString::number(m_chunkSize) + " lines each)");
    
    // Clear the editor again before loading
    send(SCI_SETREADONLY, 0);
    send(SCI_CLEARALL);
    
    // Reset stream to beginning
    m_fileStream->seek(0);
    
    // Load first chunk immediately
    loadNextChunk();
    
    // Schedule remaining chunks
    if (m_totalChunks > 1) {
        m_loadingTimer->start(10); // Start loading next chunk after 10ms
    }
}

void ScintillaEdit::loadNextChunk()
{
    // Check if loading should be aborted
    if (m_abortLoading) {
        emit debugMessage("ScintillaEdit: Progressive loading aborted");
        return;
    }
    
    if (!m_isProgressiveLoading || !m_fileStream || m_currentChunk >= m_totalChunks) {
        // Loading complete
        if (m_isProgressiveLoading) {
            emit debugMessage("ScintillaEdit: Progressive loading completed - " + QString::number(m_currentChunk) + " chunks loaded");
            
            // Clean up
            if (m_fileStream) {
                delete m_fileStream;
                m_fileStream = nullptr;
            }
            if (m_progressiveFile) {
                m_progressiveFile->close();
                delete m_progressiveFile;
                m_progressiveFile = nullptr;
            }
            
            m_isProgressiveLoading = false;
            
            // Set read-only
            send(SCI_SETREADONLY, 1);
            
            // Update line numbers display
            if (m_showFileLineNumbers) {
                updateLineNumbers();
            }
            
            emit fileLoaded(m_currentFilePath);
        }
        return;
    }
    
    // Load one chunk
    QString chunkContent;
    int linesInChunk = 0;
    int currentChunkSize = (m_currentChunk == 0) ? m_firstChunkSize : m_chunkSize;
    
    // Calculate the line range for this chunk
    int chunkStartLine = 1;
    if (m_currentChunk > 0) {
        chunkStartLine = m_firstChunkSize + (m_currentChunk - 1) * m_chunkSize + 1;
    }
    int chunkEndLine = chunkStartLine + currentChunkSize - 1;
    
    while (!m_fileStream->atEnd() && linesInChunk < currentChunkSize) {
        QString line = m_fileStream->readLine();
        chunkContent += line + "\n";
        linesInChunk++;
    }
    
    if (!chunkContent.isEmpty()) {
        // Convert to UTF-8 and append to Scintilla
        QByteArray utf8Chunk = chunkContent.toUtf8();
        send(SCI_APPENDTEXT, utf8Chunk.size(), reinterpret_cast<sptr_t>(utf8Chunk.data()));
        
        m_currentChunk++;
        
        // Emit progress
        emit loadingProgress(m_currentChunk, m_totalChunks);
        
        // Log progress occasionally
        if (m_currentChunk % 100 == 0 || m_currentChunk == m_totalChunks) {
            emit debugMessage("ScintillaEdit: Loaded chunk " + QString::number(m_currentChunk) + "/" + QString::number(m_totalChunks));
        }
        
        // Check if target line is in this chunk - highlight and scroll immediately
        if (m_targetLine > 0 && m_targetLine >= chunkStartLine && m_targetLine <= chunkEndLine) {
            emit debugMessage("ScintillaEdit: Target line " + QString::number(m_targetLine) + " found in chunk " + QString::number(m_currentChunk) + " (lines " + QString::number(chunkStartLine) + "-" + QString::number(chunkEndLine) + ")");
            scrollToLine(m_targetLine);
            highlightLine(m_targetLine);
            emit debugMessage("ScintillaEdit: AMIR Scrolled to line " + QString::number(m_targetLine));
        }
        
        // Schedule next chunk (non-blocking)
        if (m_currentChunk < m_totalChunks) {
            m_loadingTimer->start(1); // Load next chunk after 1ms (very responsive)
        } else {
            // This was the last chunk, complete loading
            loadNextChunk();
        }
    } else {
        // No more content, complete loading
        loadNextChunk();
    }
} 

void ScintillaEdit::abortProgressiveLoading()
{
    if (m_isProgressiveLoading) {
        emit debugMessage("ScintillaEdit: Aborting progressive loading");
        m_abortLoading = true;
        
        // Stop the timer
        if (m_loadingTimer) {
            m_loadingTimer->stop();
        }
        
        // Clean up resources
        if (m_fileStream) {
            delete m_fileStream;
            m_fileStream = nullptr;
        }
        if (m_progressiveFile) {
            m_progressiveFile->close();
            delete m_progressiveFile;
            m_progressiveFile = nullptr;
        }
        
        m_isProgressiveLoading = false;
        m_abortLoading = false;
    }
} 

// KLOGG-style line offset indexing methods
void ScintillaEdit::buildLineOffsetIndex(const QString &filePath)
{
    QMutexLocker locker(&m_indexMutex);
    if (m_isIndexed) {
        return; // Already indexed
    }

    emit debugMessage("ScintillaEdit: Building KLOGG-style line offset index for: " + filePath);
    
    QFile file(filePath);
    if (!openFileWithWindowsSharing(&file)) {
        qWarning() << "Failed to open file for line offset indexing:" << filePath;
        return;
    }

    m_lineOffsets.clear();
    m_lineOffsets.reserve(1000000); // Reserve space for a large file

    qint64 currentOffset = 0;
    qint64 fileSize = file.size();
    
    emit debugMessage("ScintillaEdit: File size: " + QString::number(fileSize) + " bytes");
    emit debugMessage("ScintillaEdit: Using KLOGG block size: " + QString::number(KLOGG_INDEXING_BLOCK_SIZE) + " bytes");
    
    // KLOGG-style: Read in 5MB blocks
    QByteArray buffer(KLOGG_INDEXING_BLOCK_SIZE, Qt::Uninitialized);
    
    // Start with line 1 (offset 0)
    m_lineOffsets.append(0);
    
    while (!file.atEnd()) {
        qint64 bytesRead = file.read(buffer.data(), KLOGG_INDEXING_BLOCK_SIZE);
        if (bytesRead <= 0) break;
        
        // Process the block to find line endings
        for (qint64 i = 0; i < bytesRead; ++i) {
            if (buffer[i] == '\n') {
                m_lineOffsets.append(currentOffset + i + 1); // +1 for the newline
            }
        }
        
        currentOffset += bytesRead;
        
        // Progress reporting
        if (m_lineOffsets.size() % 100000 == 0) {
            emit debugMessage("ScintillaEdit: Indexed " + QString::number(m_lineOffsets.size()) + " lines so far...");
        }
    }
    
    // If file doesn't end with newline, add the end of file as the last line
    if (fileSize > 0 && (m_lineOffsets.isEmpty() || m_lineOffsets.last() < fileSize)) {
        m_lineOffsets.append(fileSize);
    }

    m_isIndexed = true;
    emit debugMessage("ScintillaEdit: KLOGG-style line offset index built with " + QString::number(m_lineOffsets.size()) + " entries");
}

qint64 ScintillaEdit::getLineOffset(int lineNumber) const
{
    QMutexLocker locker(&m_indexMutex);
    if (!m_isIndexed) {
        qWarning() << "Line offset index not built. Cannot get offset for line" << lineNumber;
        return -1;
    }

    if (lineNumber < 1 || lineNumber > m_lineOffsets.size()) {
        qWarning() << "Line number" << lineNumber << "out of bounds for line offset index (size:" << m_lineOffsets.size() << ").";
        return -1;
    }

    return m_lineOffsets.at(lineNumber - 1); // Convert to 0-based index
}

QString ScintillaEdit::getLineAtOffset(qint64 startOffset, qint64 endOffset) const
{
    QMutexLocker locker(&m_indexMutex);
    if (!m_isIndexed) {
        qWarning() << "Line offset index not built. Cannot get line at offset.";
        return QString();
    }

    if (startOffset < 0 || endOffset < 0 || startOffset >= endOffset) {
        qWarning() << "Invalid offset range for line retrieval.";
        return QString();
    }

    // Read the line from the file
    QFile file(m_currentFilePath);
    if (!openFileWithWindowsSharing(&file)) {
        qWarning() << "Failed to open file for line retrieval at offset range.";
        return QString();
    }

    file.seek(startOffset);
    QByteArray lineData = file.read(endOffset - startOffset);
    file.close();

    return QString::fromUtf8(lineData);
}

void ScintillaEdit::loadLinesFromIndex(int startLine, int endLine)
{
    if (!m_isIndexed) {
        qWarning() << "Cannot load lines from index - index not built";
        return;
    }
    
    if (startLine < 1 || endLine < startLine) {
        qWarning() << "Invalid line range for loading from index:" << startLine << "to" << endLine;
        return;
    }
    
    emit debugMessage("ScintillaEdit: Loading lines " + QString::number(startLine) + " to " + QString::number(endLine) + " using KLOGG index");
    
    // Get the byte offsets for the line range
    qint64 startOffset = getLineOffset(startLine);
    qint64 endOffset;
    
    if (endLine >= m_lineOffsets.size()) {
        // Last line - use the end of file offset
        endOffset = m_lineOffsets.last();
    } else {
        // Get the start of the next line
        endOffset = getLineOffset(endLine + 1);
    }
    
    if (startOffset == -1 || endOffset == -1) {
        qWarning() << "Failed to get offsets for line range" << startLine << "to" << endLine;
        return;
    }
    
    emit debugMessage("ScintillaEdit: Reading bytes " + QString::number(startOffset) + " to " + QString::number(endOffset));
    
    // Read the content from the file using the offsets (KLOGG-style)
    QFile file(m_currentFilePath);
    if (!openFileWithWindowsSharing(&file)) {
        qWarning() << "Failed to open file for line retrieval at offset range.";
        return;
    }
    
    file.seek(startOffset);
    QByteArray content = file.read(endOffset - startOffset);
    file.close();
    
    if (content.isEmpty()) {
        qWarning() << "No content read from file for line range" << startLine << "to" << endLine;
        return;
    }
    
    // Set the content in Scintilla
    send(SCI_SETTEXT, 0, reinterpret_cast<sptr_t>(content.constData()));
    
    // Update line offset for display
    m_lineOffset = startLine - 1;
    
    emit debugMessage("ScintillaEdit: Loaded " + QString::number(endLine - startLine + 1) + " lines using KLOGG index (" + QString::number(content.size()) + " bytes)");
} 

void ScintillaEdit::highlightExtra(const QList<HighlightRule> &rules, bool caseSensitive, bool highlightSentence, bool useScintillaSearch, bool useViewportOnly)
{
    LOG_INFO("ScintillaEdit: Highlighting extra rules - " + QString::number(rules.size()) + " rules");
    LOG_INFO("ScintillaEdit: Case sensitive: " + QString(caseSensitive ? "Yes" : "No") + 
             ", Highlight sentence: " + QString(highlightSentence ? "Yes" : "No") +
             ", Use viewport only: " + QString(useViewportOnly ? "Yes" : "No"));
    
    // Check if we should use viewport-only highlighting (highest priority)
    if (useViewportOnly) {
        LOG_INFO("ScintillaEdit: Viewport-only highlighting mode detected, calling highlightViewportOnly()");
        m_useViewportHighlighting = true;
        highlightViewportOnly(rules, caseSensitive, highlightSentence, useScintillaSearch);
        return;
    } else {
        m_useViewportHighlighting = false;
    }
    
    // Check if we already have highlights for these exact rules
    static QList<HighlightRule> lastAppliedRules;
    static QString lastDocumentHash;
    
    QString currentDocumentHash = QString::number(send(SCI_GETTEXTLENGTH)) + "_" + 
                                 QString::number(send(SCI_GETMODIFY));
    
    // Check if rules and document are the same as last time
    bool rulesChanged = (lastAppliedRules.size() != rules.size());
    if (!rulesChanged) {
        for (int i = 0; i < rules.size(); ++i) {
            if (i >= lastAppliedRules.size() || 
                rules[i].pattern != lastAppliedRules[i].pattern ||
                rules[i].color != lastAppliedRules[i].color ||
                rules[i].enabled != lastAppliedRules[i].enabled) {
                rulesChanged = true;
                break;
            }
        }
    }
    
    bool documentChanged = (lastDocumentHash != currentDocumentHash);
    
    if (!rulesChanged && !documentChanged) {
        LOG_INFO("ScintillaEdit: Rules and document unchanged, skipping re-highlight");
        return;
    }
    
    LOG_INFO("ScintillaEdit: Rules or document changed, re-highlighting");
    LOG_INFO("ScintillaEdit: Rules changed: " + QString(rulesChanged ? "Yes" : "No") + 
             ", Document changed: " + QString(documentChanged ? "Yes" : "No"));
    
    // Clear previous extra highlights
    clearExtraHighlights();
    LOG_INFO("ScintillaEdit: Cleared previous extra highlights");
    
    if (rules.isEmpty()) {
        LOG_INFO("ScintillaEdit: No extra rules to highlight");
        lastAppliedRules.clear();
        lastDocumentHash.clear();
        return;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    // Get document length for debugging
    Scintilla::Position totalLength = send(SCI_GETTEXTLENGTH);
    LOG_INFO("ScintillaEdit: Document length: " + QString::number(totalLength) + " characters");
    
    // Collect all valid rules and their parameters
    QList<HighlightRule> validRules;
    QStringList patterns;
    
    LOG_INFO("ScintillaEdit: Analyzing " + QString::number(rules.size()) + " rules for validity");
    
    for (int ruleIndex = 0; ruleIndex < rules.size(); ++ruleIndex) {
        const HighlightRule &rule = rules[ruleIndex];
        
        LOG_INFO("ScintillaEdit: Rule " + QString::number(ruleIndex + 1) + 
                 " - Pattern: '" + rule.pattern + 
                 "', Enabled: " + QString(rule.enabled ? "Yes" : "No") + 
                 ", Color: " + rule.color.name() + 
                 " (RGB: " + QString::number(rule.color.rgb()) + 
                 " R:" + QString::number(rule.color.red()) + 
                 " G:" + QString::number(rule.color.green()) + 
                 " B:" + QString::number(rule.color.blue()) + ")");
        
        // Skip disabled rules
        if (!rule.enabled) {
            LOG_INFO("ScintillaEdit: Rule " + QString::number(ruleIndex + 1) + " is disabled, skipping");
            continue;
        }
        
        // Skip empty patterns
        if (rule.pattern.isEmpty()) {
            LOG_INFO("ScintillaEdit: Rule " + QString::number(ruleIndex + 1) + " has empty pattern, skipping");
            continue;
        }
        
        // Add to valid rules
        validRules.append(rule);
        patterns.append(rule.pattern);
        
        LOG_INFO("ScintillaEdit: Rule " + QString::number(ruleIndex + 1) + " is valid and added to search");
    }
    
    if (validRules.isEmpty()) {
        LOG_INFO("ScintillaEdit: No valid rules found, exiting");
        return;
    }
    
    // Create one big regex expression with OR logic between all patterns
    // Wrap each alternative with parentheses to be compatible with Scintilla's regex engine
    // Example: (Periph)|(credential)|(Controller)
    QString bigPattern = "(" + patterns.join(")|("
    ) + ")";
    LOG_INFO("ScintillaEdit: Created big regex pattern: '" + bigPattern + "'");
    LOG_INFO("ScintillaEdit: Big pattern length: " + QString::number(bigPattern.length()) + " characters");
    

    
    // Configure indicators for all rules (use indicators 2-11 for extra highlights)
    for (int i = 0; i < validRules.size(); ++i) {
        int indicatorIndex = 2 + i;
        send(SCI_SETINDICATORCURRENT, indicatorIndex);
        send(SCI_INDICSETSTYLE, indicatorIndex, INDIC_FULLBOX);
        
        // Convert QColor to Scintilla RGB format (BGR)
        const QColor &color = validRules[i].color;
        int r = color.red();
        int g = color.green();
        int b = color.blue();
        int scintillaColor = (b << 16) | (g << 8) | r;  // BGR format for Scintilla
        
        send(SCI_INDICSETFORE, indicatorIndex, scintillaColor);
        send(SCI_INDICSETALPHA, indicatorIndex, 100);
        send(SCI_INDICSETUNDER, indicatorIndex, true);
        
        LOG_INFO("ScintillaEdit: Configured indicator " + QString::number(indicatorIndex) + 
                 " for rule " + QString::number(i + 1) + 
                 " with color: " + color.name() + 
                 " (RGB: " + QString::number(r) + "," + QString::number(g) + "," + QString::number(b) + 
                 ") -> Scintilla BGR: " + QString::number(scintillaColor));
    }
    

    
    // Debug: Show a sample of the document content
    QString documentText = getText();
    int sampleLength = qMin(500, documentText.length());
    QString sample = documentText.left(sampleLength);
    LOG_INFO("ScintillaEdit: Document sample (first " + QString::number(sampleLength) + " chars): '" + sample + "'");
    
    // Choose search method based on parameter
    if (useScintillaSearch) {
        LOG_INFO("ScintillaEdit: Using Scintilla native search for highlighting");
        
        // Use Scintilla's SCI_SEARCHNEXT with OR pattern for better compatibility
        int totalMatchCount = 0;
        
        // Set search flags for regex search
        int searchFlags = SCFIND_REGEXP;
#ifdef SCFIND_CXX11REGEX
        searchFlags |= SCFIND_CXX11REGEX; // Enable C++11 regex engine to support alternation '|'
        LOG_INFO("ScintillaEdit: Using C++11 regex engine");
#else
        LOG_INFO("ScintillaEdit: Using legacy regex engine");
#endif
        if (caseSensitive) {
            searchFlags |= SCFIND_MATCHCASE;
        }
        send(SCI_SETSEARCHFLAGS, searchFlags);
        LOG_INFO(QString("ScintillaEdit: Search flags set: 0x") + QString::number(searchFlags, 16));
        
        // Prepare the UTF-8 pattern buffer (persist it outside the loop)
        QByteArray patUtf8 = bigPattern.toUtf8();
        const char* pat = patUtf8.constData();
        int patLen = patUtf8.size();
        
        LOG_INFO("ScintillaEdit: Searching with SCI_SEARCHNEXT for pattern: '" + bigPattern + "'");
        LOG_INFO("ScintillaEdit: Pattern UTF-8 length: " + QString::number(patLen) + " bytes");
        
        // Start from the beginning of the document
        send(SCI_GOTOPOS, 0);
        send(SCI_SEARCHANCHOR); // Set search anchor at current position
        
        int matchCount = 0;
        
        while (true) {
            // Search for next match using SCI_SEARCHNEXT
            Scintilla::Position matchStart = send(SCI_SEARCHNEXT, patLen, reinterpret_cast<sptr_t>(pat));
            
            if (matchStart == -1) {
                LOG_INFO("ScintillaEdit: SCI_SEARCHNEXT returned -1, no more matches");
                break; // No more matches
            }
            
            Scintilla::Position matchEnd = send(SCI_GETTARGETEND);
            
            // Get the matched text to determine which rule it belongs to
            QString matchedText = documentText.mid(matchStart, matchEnd - matchStart);
            
            // Determine which rule/pattern matched this text
            int matchedRuleIndex = -1;
            for (int i = 0; i < validRules.size(); ++i) {
                QRegularExpression ruleRegex(validRules[i].pattern, caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
                QRegularExpressionMatch ruleMatch = ruleRegex.match(matchedText);
                if (ruleMatch.hasMatch()) {
                    matchedRuleIndex = i;
                    break;
                }
            }
            
            if (matchedRuleIndex >= 0) {
                // Highlight with the color of the matched rule
                int indicatorIndex = 2 + matchedRuleIndex;
                send(SCI_SETINDICATORCURRENT, indicatorIndex);
                
                if (highlightSentence) {
                    // Find the start and end of the line containing the match
                    int lineNumber = send(SCI_LINEFROMPOSITION, matchStart);
                    Scintilla::Position lineStart = send(SCI_POSITIONFROMLINE, lineNumber);
                    Scintilla::Position lineEnd = send(SCI_GETLINEENDPOSITION, lineNumber);
                    send(SCI_INDICATORFILLRANGE, lineStart, lineEnd - lineStart);
                } else {
                    // Highlight just the matched phrase
                    send(SCI_INDICATORFILLRANGE, matchStart, matchEnd - matchStart);
                }
                
                matchCount++;
                totalMatchCount++;
                
                // Log first few matches for debugging
                if (matchCount <= 5) {
                    LOG_INFO("ScintillaEdit: Found match " + QString::number(matchCount) + 
                             " with rule " + QString::number(matchedRuleIndex + 1) + 
                             " (pattern: '" + validRules[matchedRuleIndex].pattern + 
                             "', color: " + validRules[matchedRuleIndex].color.name() + 
                             ") at position " + QString::number(matchStart) + 
                             " to " + QString::number(matchEnd) + 
                             ", text: '" + matchedText + "'");
                }
            } else {
                LOG_WARNING("ScintillaEdit: Could not determine which rule matched text: '" + matchedText + "'");
            }
            
            // Move cursor to end of match and set new search anchor
            send(SCI_GOTOPOS, matchEnd);
            send(SCI_SEARCHANCHOR);
        }
        
        // If we got 0 matches, try with legacy BRE (Basic Regular Expression) pattern
        if (totalMatchCount == 0) {
            LOG_INFO("ScintillaEdit: Got 0 matches with modern regex, trying legacy BRE pattern");
            
            // Create BRE pattern by escaping parentheses and pipe
            QString brePattern = "(" + patterns.join(")|("
            ) + ")";
            brePattern.replace("(", "\\(").replace(")", "\\)").replace("|", "\\|");
            
            QByteArray breUtf8 = brePattern.toUtf8();
            const char* brePat = breUtf8.constData();
            int brePatLen = breUtf8.size();
            
            // Use legacy regex flags (no C++11)
            int breFlags = SCFIND_REGEXP;
            if (caseSensitive) {
                breFlags |= SCFIND_MATCHCASE;
            }
            send(SCI_SETSEARCHFLAGS, breFlags);
            
            LOG_INFO("ScintillaEdit: Trying BRE pattern: '" + brePattern + "'");
            LOG_INFO("ScintillaEdit: BRE flags: 0x" + QString::number(breFlags, 16));
            
            // Reset to beginning and search again
            send(SCI_GOTOPOS, 0);
            send(SCI_SEARCHANCHOR);
            
            while (true) {
                Scintilla::Position matchStart = send(SCI_SEARCHNEXT, brePatLen, reinterpret_cast<sptr_t>(brePat));
                
                if (matchStart == -1) {
                    break;
                }
                
                Scintilla::Position matchEnd = send(SCI_GETTARGETEND);
                QString matchedText = documentText.mid(matchStart, matchEnd - matchStart);
                
                // Find matching rule
                int matchedRuleIndex = -1;
                for (int i = 0; i < validRules.size(); ++i) {
                    QRegularExpression ruleRegex(validRules[i].pattern, caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
                    if (ruleRegex.match(matchedText).hasMatch()) {
                        matchedRuleIndex = i;
                        break;
                    }
                }
                
                if (matchedRuleIndex >= 0) {
                    int indicatorIndex = 2 + matchedRuleIndex;
                    send(SCI_SETINDICATORCURRENT, indicatorIndex);
                    
                    if (highlightSentence) {
                        int lineNumber = send(SCI_LINEFROMPOSITION, matchStart);
                        Scintilla::Position lineStart = send(SCI_POSITIONFROMLINE, lineNumber);
                        Scintilla::Position lineEnd = send(SCI_GETLINEENDPOSITION, lineNumber);
                        send(SCI_INDICATORFILLRANGE, lineStart, lineEnd - lineStart);
                    } else {
                        send(SCI_INDICATORFILLRANGE, matchStart, matchEnd - matchStart);
                    }
                    
                    totalMatchCount++;
                }
                
                send(SCI_GOTOPOS, matchEnd);
                send(SCI_SEARCHANCHOR);
            }
            
            LOG_INFO("ScintillaEdit: BRE pattern found " + QString::number(totalMatchCount) + " matches");
        }
        
        // If still 0 matches, fall back to individual rule searches
        if (totalMatchCount == 0) {
            LOG_INFO("ScintillaEdit: Combined patterns failed, falling back to individual rule searches");
            
            for (int ruleIndex = 0; ruleIndex < validRules.size(); ++ruleIndex) {
                const HighlightRule &rule = validRules[ruleIndex];
                QByteArray ruleUtf8 = rule.pattern.toUtf8();
                const char* rulePat = ruleUtf8.constData();
                int rulePatLen = ruleUtf8.size();
                
                int indicatorIndex = 2 + ruleIndex;
                send(SCI_SETINDICATORCURRENT, indicatorIndex);
                
                send(SCI_GOTOPOS, 0);
                send(SCI_SEARCHANCHOR);
                
                int ruleMatches = 0;
                while (true) {
                    Scintilla::Position matchStart = send(SCI_SEARCHNEXT, rulePatLen, reinterpret_cast<sptr_t>(rulePat));
                    
                    if (matchStart == -1) break;
                    
                    Scintilla::Position matchEnd = send(SCI_GETTARGETEND);
                    
                    if (highlightSentence) {
                        int lineNumber = send(SCI_LINEFROMPOSITION, matchStart);
                        Scintilla::Position lineStart = send(SCI_POSITIONFROMLINE, lineNumber);
                        Scintilla::Position lineEnd = send(SCI_GETLINEENDPOSITION, lineNumber);
                        send(SCI_INDICATORFILLRANGE, lineStart, lineEnd - lineStart);
                    } else {
                        send(SCI_INDICATORFILLRANGE, matchStart, matchEnd - matchStart);
                    }
                    
                    ruleMatches++;
                    totalMatchCount++;
                    
                    send(SCI_GOTOPOS, matchEnd);
                    send(SCI_SEARCHANCHOR);
                }
                
                LOG_INFO("ScintillaEdit: Rule " + QString::number(ruleIndex + 1) + 
                         " ('" + rule.pattern + "') found " + QString::number(ruleMatches) + " matches");
            }
        }
        
        qint64 highlightTime = timer.elapsed();
        LOG_INFO("ScintillaEdit: Scintilla native search found and highlighted " + QString::number(totalMatchCount) + " total matches");
        LOG_INFO("ScintillaEdit: Extra highlighting completed in " + QString::number(highlightTime) + "ms");
        qDebug() << "ScintillaEdit: Extra highlighting completed in" << highlightTime << "ms";
        
        // Save current state for next comparison
        lastAppliedRules = rules;
        lastDocumentHash = currentDocumentHash;
        LOG_INFO("ScintillaEdit: Saved current rules and document state for next comparison");
        return;
    }
    
    // Use Qt regex to find all matches and highlight them manually (fallback)
    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
    if (!caseSensitive) {
        options |= QRegularExpression::CaseInsensitiveOption;
    }
    
    QRegularExpression qtRegex(bigPattern, options);
    if (!qtRegex.isValid()) {
        LOG_WARNING("ScintillaEdit: Regex pattern is INVALID: " + qtRegex.errorString());
        return;
    }
    
    LOG_INFO("ScintillaEdit: Using Qt regex to find matches and highlight manually");
    QRegularExpressionMatchIterator it = qtRegex.globalMatch(documentText);
    int totalMatchCount = 0;
    int searchIterations = 0;
    
    while (it.hasNext() ) {
            searchIterations++;
        QRegularExpressionMatch match = it.next();
        
        int startPos = match.capturedStart();
        int endPos = match.capturedEnd();
        QString matchedText = match.captured();
        
        // Determine which rule/pattern matched this text
        int matchedRuleIndex = -1;
        for (int i = 0; i < validRules.size(); ++i) {
            QRegularExpression ruleRegex(validRules[i].pattern);
            QRegularExpressionMatch ruleMatch = ruleRegex.match(matchedText);
            if (ruleMatch.hasMatch()) {
                matchedRuleIndex = i;
                break;
            }
        }
        
        if (matchedRuleIndex >= 0) {
            // Highlight with the color of the matched rule
            int indicatorIndex = 2 + matchedRuleIndex;
            send(SCI_SETINDICATORCURRENT, indicatorIndex);
            
            if (highlightSentence) {
                // Find the start and end of the line containing the match
                int lineStart = send(SCI_POSITIONFROMLINE, send(SCI_LINEFROMPOSITION, startPos));
                int lineEnd = send(SCI_GETLINEENDPOSITION, send(SCI_LINEFROMPOSITION, startPos));
                send(SCI_INDICATORFILLRANGE, lineStart, lineEnd - lineStart);
                
                // Log first 10 matches for debugging
                if (totalMatchCount <= 10) {
                    LOG_INFO("ScintillaEdit: Highlighted sentence " + QString::number(totalMatchCount) + 
                             " with rule " + QString::number(matchedRuleIndex + 1) + 
                             " (pattern: '" + validRules[matchedRuleIndex].pattern + 
                             "', color: " + validRules[matchedRuleIndex].color.name() + 
                             ") at line positions " + QString::number(lineStart) + 
                             " to " + QString::number(lineEnd) + 
                             ", match text: '" + matchedText + "'");
                }
            } else {
                // Highlight just the matched phrase
                send(SCI_INDICATORFILLRANGE, startPos, endPos - startPos);
                
                // Log first 10 matches for debugging
                if (totalMatchCount <= 10) {
                    LOG_INFO("ScintillaEdit: Highlighted phrase " + QString::number(totalMatchCount) + 
                             " with rule " + QString::number(matchedRuleIndex + 1) + 
                             " (pattern: '" + validRules[matchedRuleIndex].pattern + 
                             "', color: " + validRules[matchedRuleIndex].color.name() + 
                             ") at position " + QString::number(startPos) + 
                             " to " + QString::number(endPos) + 
                             ", text: '" + matchedText + "'");
                }
            }
            
            totalMatchCount++;
        } else {
            LOG_WARNING("ScintillaEdit: Could not determine which rule matched text: '" + matchedText + "'");
        }
    }
        

    
    qint64 highlightTime = timer.elapsed();
    LOG_INFO("ScintillaEdit: Qt regex found and highlighted " + QString::number(totalMatchCount) + " total matches");
    LOG_INFO("ScintillaEdit: Search iterations: " + QString::number(searchIterations));
    LOG_INFO("ScintillaEdit: Extra highlighting completed in " + QString::number(highlightTime) + "ms");
    qDebug() << "ScintillaEdit: Extra highlighting completed in" << highlightTime << "ms";
    
    // Save current state for next comparison
    lastAppliedRules = rules;
    lastDocumentHash = currentDocumentHash;
    LOG_INFO("ScintillaEdit: Saved current rules and document state for next comparison");
}

void ScintillaEdit::clearExtraHighlights()
{
    qDebug() << "ScintillaEdit: Clearing extra highlights";
    LOG_INFO("ScintillaEdit: Clearing extra highlights");

    // Clear all extra highlight indicators (2-11)
    for (int i = 2; i <= 11; ++i) {
        send(SCI_SETINDICATORCURRENT, i);
        send(SCI_INDICATORCLEARRANGE, 0, send(SCI_GETTEXTLENGTH));
    }
    
    // Clear tracked highlighted ranges
    m_highlightedRanges.clear();
    LOG_INFO("ScintillaEdit: Cleared tracked highlighted ranges");
    
    // Reset background highlighting state when clearing highlights
    m_fullyHighlightedFile = false;
    m_backgroundHighlightLine = 0;
}

void ScintillaEdit::highlightViewportOnly(const QList<HighlightRule> &rules, bool caseSensitive, bool highlightSentence, bool useScintillaSearch)
{
    qDebug() << "ScintillaEdit: Viewport-only highlighting - " << rules.size() << " rules";
    LOG_INFO("ScintillaEdit: Starting viewport-only highlighting with " + QString::number(rules.size()) + " rules");
    LOG_INFO("ScintillaEdit: Settings - Case sensitive: " + QString(caseSensitive ? "Yes" : "No") + 
             ", Highlight sentence: " + QString(highlightSentence ? "Yes" : "No") + 
             ", Use Scintilla search: " + QString(useScintillaSearch ? "Yes" : "No"));
    
    // Cache the rules and settings for scroll updates
    m_lastAppliedViewportRules = rules;
    m_cachedCaseSensitive = caseSensitive;
    m_cachedHighlightSentence = highlightSentence;
    m_cachedUseScintillaSearch = useScintillaSearch;
    
    // DON'T clear previous highlights - keep them persistent!
    // Only clear when ENABLED rules actually change (ignore disabled rule changes)
    static QList<HighlightRule> lastEnabledRules;
    static bool lastCaseSensitive = false;
    static bool lastHighlightSentence = false;
    
    // Extract only enabled rules for comparison
    QList<HighlightRule> currentEnabledRules;
    for (const auto &rule : rules) {
        if (rule.enabled) {
            currentEnabledRules.append(rule);
        }
    }
    
    bool enabledRulesChanged = (lastEnabledRules.size() != currentEnabledRules.size()) || 
                              (lastCaseSensitive != caseSensitive) || 
                              (lastHighlightSentence != highlightSentence);
    
    if (!enabledRulesChanged) {
        for (int i = 0; i < currentEnabledRules.size(); ++i) {
            if (i >= lastEnabledRules.size() || 
                currentEnabledRules[i].pattern != lastEnabledRules[i].pattern ||
                currentEnabledRules[i].color != lastEnabledRules[i].color) {
                enabledRulesChanged = true;
                break;
            }
        }
    }
    
    if (enabledRulesChanged || m_lastFirstVisibleLine == -1) {
        LOG_INFO("ScintillaEdit: Enabled rules changed or first time, clearing existing highlights");
        LOG_INFO("ScintillaEdit: Enabled rules count changed from " + QString::number(lastEnabledRules.size()) + 
                 " to " + QString::number(currentEnabledRules.size()));
        clearExtraHighlights();
        lastEnabledRules = currentEnabledRules;
        lastCaseSensitive = caseSensitive;
        lastHighlightSentence = highlightSentence;
    } else {
        LOG_INFO("ScintillaEdit: Enabled rules unchanged, keeping existing highlights and adding new ones");
    }
    
    if (rules.isEmpty()) {
        LOG_INFO("ScintillaEdit: No rules to highlight in viewport");
        return;
    }
    
    QElapsedTimer timer;
    timer.start();
    
    // Get visible viewport range
    int firstVisibleLine = send(SCI_GETFIRSTVISIBLELINE);
    int visibleLineCount = send(SCI_LINESONSCREEN);
    int lastVisibleLine = firstVisibleLine + visibleLineCount;
    
    // Expand range with buffer for pre-caching (500 lines up and down)
    int bufferLines = 500;
    int totalLines = send(SCI_GETLINECOUNT);
    int expandedFirstLine = qMax(0, firstVisibleLine - bufferLines);
    int expandedLastLine = qMin(totalLines - 1, lastVisibleLine + bufferLines);
    
    LOG_INFO("ScintillaEdit: Visible viewport: lines " + QString::number(firstVisibleLine) + 
             "-" + QString::number(lastVisibleLine) + " (" + QString::number(visibleLineCount) + " lines)");
    LOG_INFO("ScintillaEdit: Expanded range with buffer: lines " + QString::number(expandedFirstLine) + 
             "-" + QString::number(expandedLastLine) + " (" + QString::number(expandedLastLine - expandedFirstLine + 1) + " lines)");
    
    // Check if the visible viewport is already highlighted (not the full expanded range)
    bool visibleRangeHighlighted = false;
    for (const auto &range : m_highlightedRanges) {
        if (firstVisibleLine >= range.first && lastVisibleLine <= range.second) {
            visibleRangeHighlighted = true;
            LOG_INFO("ScintillaEdit: Visible range " + QString::number(firstVisibleLine) + 
                     "-" + QString::number(lastVisibleLine) + " already highlighted in range " + 
                     QString::number(range.first) + "-" + QString::number(range.second));
            break;
        }
    }
    
    // Check if we can expand an existing range instead of creating a new one
    bool canExpandExisting = false;
    if (visibleRangeHighlighted && !enabledRulesChanged && m_lastFirstVisibleLine != -1) {
        // Check if we can expand the buffer around the visible area
        for (auto &range : m_highlightedRanges) {
            if (firstVisibleLine >= range.first && lastVisibleLine <= range.second) {
                // Expand the existing range to include the new buffer
                int newStart = qMin(range.first, expandedFirstLine);
                int newEnd = qMax(range.second, expandedLastLine);
                if (newStart != range.first || newEnd != range.second) {
                    LOG_INFO("ScintillaEdit: Expanding existing range from " + QString::number(range.first) + 
                             "-" + QString::number(range.second) + " to " + QString::number(newStart) + 
                             "-" + QString::number(newEnd));
                    range.first = newStart;
                    range.second = newEnd;
                    canExpandExisting = true;
                } else {
                    LOG_INFO("ScintillaEdit: Visible range already fully buffered, skipping");
                    return;
                }
                break;
            }
        }
        
        if (!canExpandExisting) {
            LOG_INFO("ScintillaEdit: Visible range already highlighted, skipping");
            return;
        }
    }
    
    // Update cached viewport info
    m_lastFirstVisibleLine = firstVisibleLine;
    m_lastVisibleLineCount = visibleLineCount;
    
    // Convert expanded range to character positions
    Scintilla::Position startPos = send(SCI_POSITIONFROMLINE, expandedFirstLine);
    Scintilla::Position endPos = send(SCI_POSITIONFROMLINE, expandedLastLine + 1);
    if (endPos == -1) endPos = send(SCI_GETTEXTLENGTH); // Handle end of document
    
    LOG_INFO("ScintillaEdit: Expanded range - lines " + QString::number(expandedFirstLine) + 
             " to " + QString::number(expandedLastLine) + 
             " (positions " + QString::number(startPos) + " to " + QString::number(endPos) + ")");
    
    // Get expanded text (visible + buffer)
    QString fullText = getText();
    QString expandedText = fullText.mid(startPos, endPos - startPos);
    
    LOG_INFO("ScintillaEdit: Expanded text length: " + QString::number(expandedText.length()) + " characters");
    
    // Collect valid rules
    QList<HighlightRule> validRules;
    QStringList patterns;
    
    for (int ruleIndex = 0; ruleIndex < rules.size(); ++ruleIndex) {
        const HighlightRule &rule = rules[ruleIndex];
        
        if (rule.enabled && !rule.pattern.isEmpty()) {
            validRules.append(rule);
            patterns.append(rule.pattern);
        }
    }
    
    if (validRules.isEmpty()) {
        LOG_INFO("ScintillaEdit: No valid rules found for viewport highlighting");
        return;
    }
    
    // Configure indicators for all rules (use indicators 2-11 for extra highlights)
    for (int i = 0; i < validRules.size(); ++i) {
        int indicatorIndex = 2 + i;
        send(SCI_SETINDICATORCURRENT, indicatorIndex);
        send(SCI_INDICSETSTYLE, indicatorIndex, INDIC_FULLBOX);
        
        // Convert QColor to Scintilla RGB format (BGR)
        const QColor &color = validRules[i].color;
        int r = color.red();
        int g = color.green();
        int b = color.blue();
        int scintillaColor = (b << 16) | (g << 8) | r;  // BGR format for Scintilla
        
        send(SCI_INDICSETFORE, indicatorIndex, scintillaColor);
        send(SCI_INDICSETALPHA, indicatorIndex, 100);
        send(SCI_INDICSETUNDER, indicatorIndex, true);
    }
    
    // Create combined OR pattern
    QString bigPattern = "(" + patterns.join(")|(") + ")";
    LOG_INFO("ScintillaEdit: Viewport search pattern: '" + bigPattern + "'");
    
    int totalMatchCount = 0;
    
    // Use Qt regex search on visible text only (fastest for small viewport)
    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
    if (!caseSensitive) {
        options |= QRegularExpression::CaseInsensitiveOption;
    }
    
    QRegularExpression qtRegex(bigPattern, options);
    if (!qtRegex.isValid()) {
        LOG_WARNING("ScintillaEdit: Viewport regex pattern is INVALID: " + qtRegex.errorString());
        return;
    }
    
    QRegularExpressionMatchIterator it = qtRegex.globalMatch(expandedText);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        
        // Convert relative positions back to document positions
        int relativeStartPos = match.capturedStart();
        int relativeEndPos = match.capturedEnd();
        int docStartPos = startPos + relativeStartPos;
        int docEndPos = startPos + relativeEndPos;
        
        QString matchedText = match.captured();
        
        // Determine which rule/pattern matched this text
        int matchedRuleIndex = -1;
        for (int i = 0; i < validRules.size(); ++i) {
            QRegularExpression ruleRegex(validRules[i].pattern, options);
            QRegularExpressionMatch ruleMatch = ruleRegex.match(matchedText);
            if (ruleMatch.hasMatch()) {
                matchedRuleIndex = i;
                break;
            }
        }
        
        if (matchedRuleIndex >= 0) {
            // Highlight with the color of the matched rule
            int indicatorIndex = 2 + matchedRuleIndex;
            send(SCI_SETINDICATORCURRENT, indicatorIndex);
            
            if (highlightSentence) {
                // Find the start and end of the line containing the match
                int lineStart = send(SCI_POSITIONFROMLINE, send(SCI_LINEFROMPOSITION, docStartPos));
                int lineEnd = send(SCI_GETLINEENDPOSITION, send(SCI_LINEFROMPOSITION, docStartPos));
                send(SCI_INDICATORFILLRANGE, lineStart, lineEnd - lineStart);
            } else {
                // Highlight just the matched phrase
                send(SCI_INDICATORFILLRANGE, docStartPos, docEndPos - docStartPos);
            }
            
            totalMatchCount++;
        }
    }
    
    qint64 highlightTime = timer.elapsed();
    LOG_INFO("ScintillaEdit: Viewport highlighting completed in " + QString::number(highlightTime) + "ms");
    LOG_INFO("ScintillaEdit: Found and highlighted " + QString::number(totalMatchCount) + " matches in viewport");
    qDebug() << "ScintillaEdit: Viewport highlighting completed in" << highlightTime << "ms with" << totalMatchCount << "matches";
    
    // Track the expanded range as highlighted
    int rangeStart = expandedFirstLine;
    int rangeEnd = expandedLastLine;
    
    // Merge with existing ranges if they overlap
    bool merged = false;
    int mergeBuffer = 10; // Small buffer for merging adjacent ranges
    for (auto &range : m_highlightedRanges) {
        if (rangeStart <= range.second + mergeBuffer && rangeEnd >= range.first - mergeBuffer) {
            // Merge ranges
            range.first = qMin(range.first, rangeStart);
            range.second = qMax(range.second, rangeEnd);
            merged = true;
            LOG_INFO("ScintillaEdit: Merged highlighted range to " + QString::number(range.first) + 
                     "-" + QString::number(range.second));
            break;
        }
    }
    
    if (!merged) {
        // Add new range
        m_highlightedRanges.append(qMakePair(rangeStart, rangeEnd));
        
        // Safety check: prevent m_highlightedRanges from growing too large
        if (m_highlightedRanges.size() > 10000) {
            LOG_WARNING("ScintillaEdit: m_highlightedRanges size limit reached (" + QString::number(m_highlightedRanges.size()) + "), clearing to prevent memory issues");
            m_highlightedRanges.clear();
        }
        LOG_INFO("ScintillaEdit: Added new highlighted range " + QString::number(rangeStart) + 
                 "-" + QString::number(rangeEnd));
    }
    
    LOG_INFO("ScintillaEdit: Total highlighted ranges: " + QString::number(m_highlightedRanges.size()));
}

// ===== BACKGROUND HIGHLIGHTING SYSTEM =====
//
// WHY: Background highlighting allows highlighting large files without blocking the UI.
//      It processes the file in chunks during idle time, providing complete coverage.
//
// WHAT: 1. Stores highlighting rules and configuration
//       2. Sets up timers for chunk processing and user activity detection
//       3. Manages two processing modes: Lines (fixed number) and Duration (time-based)
//       4. Handles performance tuning and idle detection
//
// FLOW: MainWindow calls this → Configuration stored → Idle timer started → 
//       User becomes idle → Chunk processing begins → Continues until file complete

void ScintillaEdit::startBackgroundHighlighting(const QList<HighlightRule> &rules, bool caseSensitive, bool highlightSentence, int chunkSize, int idleDelay, const QString &performanceMode, const QString &chunkMode, int durationMs)
{
    LOG_INFO("ScintillaEdit: Starting background highlighting with " + QString::number(rules.size()) + " rules");
    LOG_INFO("ScintillaEdit: Configuration - Chunk Mode: " + chunkMode + 
             ", Chunk Size: " + QString::number(chunkSize) + " lines" +
             ", Duration: " + QString::number(durationMs) + "ms" +
             ", Idle Delay: " + QString::number(idleDelay) + "ms, Performance Mode: " + performanceMode);
    
    // Stop any existing background highlighting
    stopBackgroundHighlighting();
    
    // Store the rules and settings for background highlighting
    m_backgroundRules = rules;
    m_backgroundCaseSensitive = caseSensitive;
    m_backgroundHighlightSentence = highlightSentence;
    m_backgroundChunkSize = chunkSize;
    m_backgroundChunkMode = chunkMode;
    m_backgroundDurationMs = durationMs;
    
    // Configure timer interval based on performance mode
    int timerInterval;
    if (performanceMode == "Fast") {
        timerInterval = 5; // 5ms - very responsive
    } else if (performanceMode == "Thorough") {
        timerInterval = 20; // 20ms - more thorough processing
    } else { // Balanced (default)
        timerInterval = 10; // 10ms - balanced
    }
    
    m_backgroundHighlightTimer->setInterval(timerInterval);
    LOG_INFO("ScintillaEdit: Performance mode '" + performanceMode + "' set timer interval to " + QString::number(timerInterval) + "ms");
    
    // Configure idle timer with user-specified delay
    m_idleTimer->setInterval(idleDelay);
    
    // Reset progress
    m_backgroundHighlightLine = 0;
    m_fullyHighlightedFile = false;
    m_backgroundHighlightingActive = true; // Mark background highlighting as active
    
    LOG_INFO("ScintillaEdit: Background highlighting initialized - Starting line: " + QString::number(m_backgroundHighlightLine) + 
             ", Chunk mode: " + chunkMode + 
             (chunkMode == "Lines" ? ", Chunk size: " + QString::number(chunkSize) + " lines" : ", Duration: " + QString::number(durationMs) + "ms"));
    
    // Start idle timer - background highlighting begins when user becomes idle
    m_userActive = true; // Assume user is active when starting
    m_idleTimer->start();
    
    LOG_INFO("ScintillaEdit: Background highlighting scheduled to start after " + QString::number(idleDelay) + "ms idle period");
}

void ScintillaEdit::stopBackgroundHighlighting()
{
    try {
        LOG_INFO("ScintillaEdit: Stopping background highlighting");
        
        // Stop all timers with safety checks
        if (m_backgroundHighlightTimer) {
            m_backgroundHighlightTimer->stop();
        }
        if (m_idleTimer) {
            m_idleTimer->stop();
        }
        if (m_continuousTimer) {
            m_continuousTimer->stop();
        }
        
        // Clear background rules
        m_backgroundRules.clear();
        m_backgroundHighlightingActive = false; // Mark background highlighting as inactive
    } catch (const std::exception& e) {
        LOG_WARNING("ScintillaEdit: Exception in stopBackgroundHighlighting: " + QString(e.what()));
    }
}

int ScintillaEdit::getBackgroundProgress() const
{
    int totalLines = send(SCI_GETLINECOUNT);
    if (totalLines == 0) return 100;
    
    // Ensure we don't have negative values
    int currentLine = qMax(0, m_backgroundHighlightLine);
    
    int progress = (currentLine * 100) / totalLines;
    return qMax(0, qMin(100, progress));
}

// ===== BACKGROUND HIGHLIGHTING SLOTS =====

// ===== CHUNK PROCESSING: THE HEART OF BACKGROUND HIGHLIGHTING =====
//
// WHY: This method processes one chunk of the file at a time, allowing for non-blocking
//      highlighting of large files. It implements two distinct processing modes.
//
// WHAT: 1. LINES MODE: Processes a fixed number of lines per chunk (e.g., 1000 lines)
//       2. DURATION MODE: Processes as many lines as possible within a time limit (e.g., 50ms)
//       3. Tracks progress and position between chunks
//       4. Handles completion detection and progress reporting
//
// FLOW: Timer triggers this → Check user activity → Process chunk → Update position → 
//       Schedule next chunk → Repeat until file complete
//
void ScintillaEdit::onBackgroundHighlightChunk()
{
    try {
        if (m_userActive || m_backgroundRules.isEmpty() || m_fullyHighlightedFile) {
            return; // Don't highlight if user is active or no rules or already fully highlighted
        }
        
        int totalLines = send(SCI_GETLINECOUNT);
        if (m_backgroundHighlightLine >= totalLines) {
            // Highlighting complete!
            m_fullyHighlightedFile = true;
            LOG_INFO("ScintillaEdit: Background highlighting completed! File is fully highlighted.");
            emit backgroundHighlightCompleted();
            return;
        }
    
    QElapsedTimer chunkTimer;
    chunkTimer.start();
    
    // Ensure m_backgroundHighlightLine is valid
    if (m_backgroundHighlightLine < 0) {
        LOG_WARNING("ScintillaEdit: Background highlight line was negative (" + QString::number(m_backgroundHighlightLine) + "), resetting to 0");
        m_backgroundHighlightLine = 0;
    }
    
    // ===== DURATION MODE: TIME-BASED CHUNK PROCESSING =====
    //
    // WHY: Duration mode processes as many lines as possible within a specific time limit.
    //      This provides consistent timing regardless of file content complexity.
    //
    // WHAT: 1. Starts a timer for the specified duration (e.g., 50ms)
    //       2. Processes lines in a while loop until timer expires
    //       3. Performs actual highlighting work during the loop
    //       4. Remembers exact position for next chunk
    //
    // FLOW: Timer starts → While loop processes lines → Timer expires → Position saved → 
    //       Next chunk scheduled
    //
    if (m_backgroundChunkMode == "Duration") {
        // Duration-based chunking: process lines for the specified duration
        LOG_INFO("ScintillaEdit: Duration-based chunking - processing for " + QString::number(m_backgroundDurationMs) + "ms starting from line " + QString::number(m_backgroundHighlightLine));
        
        // Start timer for duration-based processing
        QElapsedTimer durationTimer;
        durationTimer.start();
        
        int currentLine = m_backgroundHighlightLine;
        int linesProcessed = 0;
        
        // Build combined pattern for all enabled rules (once, outside the loop)
        QStringList patterns;
        for (const auto &rule : m_backgroundRules) {
            if (rule.enabled) {
                patterns.append("(" + rule.pattern + ")");
            }
        }
        
        if (patterns.isEmpty()) {
            LOG_INFO("ScintillaEdit: No enabled rules, skipping duration processing");
            m_backgroundHighlightLine = totalLines; // Mark as complete
            return;
        }
        
        QString combinedPattern = patterns.join("|");
        QRegularExpression regex(combinedPattern);
        if (!m_backgroundCaseSensitive) {
            regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        }
        
        // Process lines within the time duration
        while (currentLine < totalLines && durationTimer.elapsed() < m_backgroundDurationMs) {
            // Check if this line is already highlighted
            bool lineAlreadyHighlighted = false;
            for (const auto &range : m_highlightedRanges) {
                if (currentLine >= range.first && currentLine < range.second) {
                    lineAlreadyHighlighted = true;
                    break;
                }
            }
            
            if (!lineAlreadyHighlighted) {
                // Get text for this line
                int lineStart = send(SCI_POSITIONFROMLINE, currentLine);
                int lineEnd = send(SCI_GETLINEENDPOSITION, currentLine);
                int textLength = lineEnd - lineStart;
                
                if (textLength > 0) {
                    QByteArray textData(textLength + 1, '\0');
                    struct Sci_TextRange tr = {{lineStart, lineEnd}, textData.data()};
                    send(SCI_GETTEXTRANGE, 0, reinterpret_cast<sptr_t>(&tr));
                    QString lineText = QString::fromUtf8(textData.constData());
                    
                    // Apply highlighting to this line
                    auto matchIterator = regex.globalMatch(lineText);
                    while (matchIterator.hasNext()) {
                        QRegularExpressionMatch match = matchIterator.next();
                        
                        // Find which rule matched
                        for (int i = 0; i < m_backgroundRules.size(); ++i) {
                            if (!m_backgroundRules[i].enabled) continue;
                            
                            int captureIndex = i + 1;
                            if (captureIndex < match.capturedTexts().size() && !match.captured(captureIndex).isEmpty()) {
                                int matchStart = lineStart + match.capturedStart();
                                int matchLength = match.capturedLength();
                                
                                if (m_backgroundHighlightSentence) {
                                    // Highlight entire line
                                    send(SCI_SETINDICATORCURRENT, 2 + (i % 10));
                                    send(SCI_INDICSETSTYLE, 2 + (i % 10), INDIC_ROUNDBOX);
                                    send(SCI_INDICSETFORE, 2 + (i % 10), 
                                         (m_backgroundRules[i].color.blue() << 16) | 
                                         (m_backgroundRules[i].color.green() << 8) | 
                                         m_backgroundRules[i].color.red());
                                    send(SCI_INDICSETALPHA, 2 + (i % 10), 100);
                                    send(SCI_INDICATORFILLRANGE, lineStart, lineEnd - lineStart);
                                } else {
                                    // Highlight just the matched text
                                    send(SCI_SETINDICATORCURRENT, 2 + (i % 10));
                                    send(SCI_INDICSETSTYLE, 2 + (i % 10), INDIC_ROUNDBOX);
                                    send(SCI_INDICSETFORE, 2 + (i % 10), 
                                         (m_backgroundRules[i].color.blue() << 16) | 
                                         (m_backgroundRules[i].color.green() << 8) | 
                                         m_backgroundRules[i].color.red());
                                    send(SCI_INDICSETALPHA, 2 + (i % 10), 100);
                                    send(SCI_INDICATORFILLRANGE, matchStart, matchLength);
                                }
                                break;
                            }
                        }
                    }
                }
            }
            
            currentLine++;
            linesProcessed++;
        }
        
        // Update position and add to highlighted ranges
        m_backgroundHighlightLine = currentLine;
        if (linesProcessed > 0) {
            m_highlightedRanges.append(qMakePair(currentLine - linesProcessed, currentLine));
            
            // Safety check: prevent m_highlightedRanges from growing too large
            if (m_highlightedRanges.size() > 10000) {
                LOG_WARNING("ScintillaEdit: m_highlightedRanges size limit reached (" + QString::number(m_highlightedRanges.size()) + "), clearing to prevent memory issues");
                m_highlightedRanges.clear();
            }
        }
        
        qint64 actualDuration = durationTimer.elapsed();
//        LOG_INFO("ScintillaEdit: Duration-based chunk completed - processed " + QString::number(linesProcessed) + 
 //                " lines in " + QString::number(actualDuration) + "ms (target: " + QString::number(m_backgroundDurationMs) + "ms)");
        
    } else {
        // Lines-based chunking: process fixed number of lines  
        int chunkEndLine = qMin(m_backgroundHighlightLine + m_backgroundChunkSize, totalLines);
    //    LOG_INFO("ScintillaEdit: Lines-based chunking - processing " + QString::number(m_backgroundChunkSize) + " lines");
        
 //       LOG_INFO("ScintillaEdit: Background highlighting chunk lines " + 
 //                QString::number(m_backgroundHighlightLine) + "-" + QString::number(chunkEndLine) + 
 //                " (" + QString::number(getBackgroundProgress()) + "%)");
        
        // Check if this range is already highlighted (skip if it is)
        bool alreadyHighlighted = false;
        for (const auto &range : m_highlightedRanges) {
            if (m_backgroundHighlightLine >= range.first && chunkEndLine <= range.second) {
                alreadyHighlighted = true;
                break;
            }
        }
        
        if (alreadyHighlighted) {
            LOG_INFO("ScintillaEdit: Background chunk already highlighted, skipping");
        } else {
        // Highlight this chunk using the same logic as viewport highlighting
        // Get text for this range
        int startPos = send(SCI_POSITIONFROMLINE, m_backgroundHighlightLine);
        int endPos = send(SCI_POSITIONFROMLINE, chunkEndLine);
        if (chunkEndLine >= totalLines) {
            endPos = send(SCI_GETTEXTLENGTH);
        }
        
        int textLength = endPos - startPos;
        if (textLength > 0) {
            QByteArray textData(textLength + 1, '\0');
            struct Sci_TextRange tr = {{startPos, endPos}, textData.data()};
            send(SCI_GETTEXTRANGE, 0, reinterpret_cast<sptr_t>(&tr));
            QString chunkText = QString::fromUtf8(textData.constData());
            
            // Build combined pattern for all enabled rules
            QStringList patterns;
            for (const auto &rule : m_backgroundRules) {
                if (rule.enabled) {
                    patterns.append("(" + rule.pattern + ")");
                }
            }
            
            if (!patterns.isEmpty()) {
                QString combinedPattern = patterns.join("|");
                QRegularExpression regex(combinedPattern);
                if (!m_backgroundCaseSensitive) {
                    regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
                }
                
                auto matchIterator = regex.globalMatch(chunkText);
                int matchCount = 0;
                
                while (matchIterator.hasNext()) {
                    QRegularExpressionMatch match = matchIterator.next();
                    
                    // Find which rule matched
                    for (int i = 0; i < m_backgroundRules.size(); ++i) {
                        if (!m_backgroundRules[i].enabled) continue;
                        
                        int captureIndex = i + 1; // Capture groups start at 1
                        if (captureIndex < match.capturedTexts().size() && !match.captured(captureIndex).isEmpty()) {
                            // This rule matched
                            int matchStart = startPos + match.capturedStart();
                            int matchLength = match.capturedLength();
                            
                            if (m_backgroundHighlightSentence) {
                                // Highlight entire line
                                int lineNum = send(SCI_LINEFROMPOSITION, matchStart);
                                int lineStart = send(SCI_POSITIONFROMLINE, lineNum);
                                int lineEnd = send(SCI_GETLINEENDPOSITION, lineNum);
                                
                                send(SCI_SETINDICATORCURRENT, 2 + (i % 10));
                                send(SCI_INDICSETSTYLE, 2 + (i % 10), INDIC_ROUNDBOX);
                                send(SCI_INDICSETFORE, 2 + (i % 10), 
                                     (m_backgroundRules[i].color.blue() << 16) | 
                                     (m_backgroundRules[i].color.green() << 8) | 
                                     m_backgroundRules[i].color.red());
                                send(SCI_INDICSETALPHA, 2 + (i % 10), 100);
                                send(SCI_INDICATORFILLRANGE, lineStart, lineEnd - lineStart);
                            } else {
                                // Highlight just the matched text
                                send(SCI_SETINDICATORCURRENT, 2 + (i % 10));
                                send(SCI_INDICSETSTYLE, 2 + (i % 10), INDIC_ROUNDBOX);
                                send(SCI_INDICSETFORE, 2 + (i % 10), 
                                     (m_backgroundRules[i].color.blue() << 16) | 
                                     (m_backgroundRules[i].color.green() << 8) | 
                                     m_backgroundRules[i].color.red());
                                send(SCI_INDICSETALPHA, 2 + (i % 10), 100);
                                send(SCI_INDICATORFILLRANGE, matchStart, matchLength);
                            }
                            matchCount++;
                            break;
                        }
                    }
                }
                
                if (matchCount > 0) {
   //                 LOG_INFO("ScintillaEdit: Background highlighted " + QString::number(matchCount) + " matches in chunk");
                }
            }
        }
        
        // Add this range to highlighted ranges
        m_highlightedRanges.append(qMakePair(m_backgroundHighlightLine, chunkEndLine));
        
        // Safety check: prevent m_highlightedRanges from growing too large
        if (m_highlightedRanges.size() > 10000) {
            LOG_WARNING("ScintillaEdit: m_highlightedRanges size limit reached (" + QString::number(m_highlightedRanges.size()) + "), clearing to prevent memory issues");
            m_highlightedRanges.clear();
        }
        }
        
        // Update progress for lines mode
        m_backgroundHighlightLine = chunkEndLine;
        
        qint64 chunkTime = chunkTimer.elapsed();
        int progress = getBackgroundProgress();
        LOG_INFO("ScintillaEdit: Lines-based chunk (lines ) completed in " + QString::number(chunkTime) + "ms (" + QString::number(progress) + "%)");
    }
    
    // Check if highlighting is complete
    if (m_backgroundHighlightLine >= totalLines) {
        m_fullyHighlightedFile = true;
        LOG_INFO("ScintillaEdit: Background highlighting completed! File is fully highlighted.");
        emit backgroundHighlightCompleted();
        return;
    }
    
    // Emit progress signal
    int progress = getBackgroundProgress();
    emit backgroundHighlightProgress(progress);
    
    // Schedule next chunk immediately if not complete and user still idle
    if (m_backgroundHighlightLine < totalLines && !m_userActive) {
        // Continue with the continuous timer for fast processing when user is idle
        if (m_continuousTimer) {
            m_continuousTimer->start();
        }
    }
    } catch (const std::exception& e) {
        LOG_WARNING("ScintillaEdit: Exception in onBackgroundHighlightChunk: " + QString(e.what()));
    }
    
}

// ===== USER ACTIVITY DETECTION: INTELLIGENT IDLE MANAGEMENT =====
//
// WHY: Background highlighting should only run when the user is not actively working.
//      This prevents performance impact during active editing/scrolling.
//
// WHAT: 1. Detects user activity (scrolling, editing, clicking)
//       2. Immediately pauses background highlighting
//       3. Starts idle timer to detect when user becomes inactive
//
// FLOW: User activity detected → Background highlighting paused → Idle timer started →
//       Timer expires → Background highlighting resumes
//
void ScintillaEdit::onUserActivity()
{

   
    try {
        // Only pause background highlighting if it's not caused by background highlighting itself
        if (m_backgroundHighlightingActive) {
     //       LOG_INFO("ScintillaEdit: Ignoring background highlighting operations as user activity");
            // Don't stop timers or restart idle timer - just return
            return;
        }
        
        // Mark user as active only for real user activity
        if (!m_userActive) {
            LOG_INFO("ScintillaEdit: User became active");
            m_userActive = true;
        }
        
        // Pause background highlighting for real user activity
        
        // Safety checks for timers
        if (m_backgroundHighlightTimer) {
            m_backgroundHighlightTimer->stop();
        }
        if (m_continuousTimer) {
            m_continuousTimer->stop();
        }
        
        // Restart idle timer with the configured idle delay
        // Get the current idle delay setting from QSettings
        int idleDelay = 1000; // default 1 second
        try {
            QSettings settings("app.ini", QSettings::IniFormat);
            settings.beginGroup("Configuration");
            QString idleDelayText = settings.value("IdleDelay", "1 second").toString();
            settings.endGroup();
            
            // Parse idle delay from text
            if (idleDelayText.contains("1 second")) {
                idleDelay = 1000;
            } else if (idleDelayText.contains("2 seconds")) {
                idleDelay = 2000;
            } else if (idleDelayText.contains("5 seconds")) {
                idleDelay = 5000;
            } else if (idleDelayText.contains("10 seconds")) {
                idleDelay = 10000;
            }
        } catch (const std::exception& e) {
            LOG_WARNING("ScintillaEdit: Error reading idle delay settings: " + QString(e.what()) + ", using default 1000ms");
        }
        
        if (m_idleTimer) {
            m_idleTimer->setInterval(idleDelay);
            m_idleTimer->start();
 //           LOG_INFO("ScintillaEdit: Idle timer restarted with " + QString::number(idleDelay) + "ms delay");
        } else {
            LOG_WARNING("ScintillaEdit: m_idleTimer is null, cannot restart");
        }
    } catch (const std::exception& e) {
        LOG_WARNING("ScintillaEdit: Exception in onUserActivity: " + QString(e.what()));
    }

   
}

// ===== IDLE DETECTION: RESUMING BACKGROUND PROCESSING =====
//
// WHY: When the user becomes idle, it's safe to resume background highlighting.
//      This ensures highlighting continues without interfering with user work.
//
// WHAT: 1. Detects when idle timer expires (user has been inactive)
//       2. Resumes background highlighting if conditions are met
//       3. Continues from the exact position where highlighting was paused
//
// FLOW: Idle timer expires → User marked as idle → Background highlighting timer restarted →
//       Chunk processing resumes
//
void ScintillaEdit::onUserIdle()
{

    
    try {
        if (m_userActive) {
            LOG_INFO("ScintillaEdit: User became idle, starting continuous background highlighting");
            m_userActive = false;
            
            // Start continuous background highlighting if we have rules and not fully highlighted
            if (!m_backgroundRules.isEmpty() && !m_fullyHighlightedFile) {
                if (m_continuousTimer) {
                    m_continuousTimer->start();
                } else {
                    LOG_WARNING("ScintillaEdit: m_continuousTimer is null, cannot start background highlighting");
                }
            }
        }
    } catch (const std::exception& e) {
        LOG_WARNING("ScintillaEdit: Exception in onUserIdle: " + QString(e.what()));
    }
    
} 