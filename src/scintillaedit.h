#ifndef SCINTILLAEDIT_H
#define SCINTILLAEDIT_H

#include <QWidget>
#include <QString>
#include <QTextStream>
#include <QFile>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDebug>
#include <QColor>
#include <QTimer> // Added for QTimer
#include <QVector> // Added for QVector
#include <QMutex> // Added for QMutex

// Include Scintilla headers
#include "ScintillaEditBase.h"
#include "Scintilla.h"
#include "ScintillaTypes.h"
#include "ScintillaMessages.h"

// Forward declaration
struct HighlightRule;

class ScintillaEdit : public ScintillaEditBase
{
    Q_OBJECT

public:
    explicit ScintillaEdit(QWidget *parent = nullptr);
    ~ScintillaEdit();

    // File operations
    bool loadFile(const QString &filePath, int targetLine = 0, int contextLines = 0);
    bool loadFileKloggStyle(const QString &filePath, int targetLine = 0, int contextLines = 0);
    void scrollToLine(int lineNumber);
    void highlightLine(int lineNumber);
    void clearHighlight();
    int getFileLineNumber(int displayLine) const;
    
    // Text operations
    QString getText() const;
    void setText(const QString &text);
    void appendText(const QString &text);
    // Fast path: set UTF-8 bytes directly without converting from QString
    void setUtf8Bytes(const char* data, int length);
    void clearText();
    
    // Line count operations
    int lineCount() const;
    
    // Scintilla specific operations
    void setReadOnly(bool readOnly);
    void setLineNumbers(bool show);
    void setWrapMode(bool wrap);
    void setFont(const QString &fontFamily, int fontSize);
    void setTheme(const QString &theme);
    void setTabWidth(int width, bool useSpaces = true);
    void setHighlightColor(const QColor &color);
    void setShowFileLineNumbers(bool show);
    void setChunkSize(int chunkSize);
    void setFirstChunkSize(int firstChunkSize);
    
    // Progressive loading
    void startProgressiveLoading(const QString &filePath, int targetLine, int contextLines);
    void loadNextChunk();
    void abortProgressiveLoading(); // Add abort method
    
    // Search and replace
    void findText(const QString &text, bool caseSensitive = false, bool wholeWord = false);
    void replaceText(const QString &findText, const QString &replaceText, bool caseSensitive = false);
    void clearSearchIndicators();
    
    // Fast in-file search with highlighting
    void fastSearchAndHighlight(const QString &pattern, bool caseSensitive = false, bool wholeWord = false, bool useRegex = false);
    void highlightSearchResults(const QList<QPair<int, int>> &matches); // start, length pairs
    
    // Extra highlighting system
    void highlightExtra(const QList<HighlightRule> &rules, bool caseSensitive = false, bool highlightSentence = false, bool useScintillaSearch = false, bool useViewportOnly = false);
    void highlightViewportOnly(const QList<HighlightRule> &rules, bool caseSensitive = false, bool highlightSentence = false, bool useScintillaSearch = false);
    void clearExtraHighlights();
    
    // Background highlighting system
    void startBackgroundHighlighting(const QList<HighlightRule> &rules, bool caseSensitive = false, bool highlightSentence = false, int chunkSize = 1000, int idleDelay = 2000, const QString &performanceMode = "Balanced", const QString &chunkMode = "Lines", int durationMs = 100);
    void stopBackgroundHighlighting();
    bool isFullyHighlighted() const { return m_fullyHighlightedFile; }
    int getBackgroundProgress() const; // Returns percentage (0-100)

signals:
    void fileLoaded(const QString &filePath);
    void fileLoadError(const QString &error);
    void debugMessage(const QString &message);
    void lineClicked(int lineNumber);
    void fileLineNumberChanged(int fileLineNumber);
    void verticalScrolled();
    void textChanged();
    void loadingProgress(int chunksLoaded, int totalChunks);
    void backgroundHighlightProgress(int percentage);
    void backgroundHighlightCompleted();

private slots:
    void onDoubleClick(Scintilla::Position position, Scintilla::Position line);
    void onModified(Scintilla::ModificationFlags type, Scintilla::Position position, 
                   Scintilla::Position length, Scintilla::Position linesAdded,
                   const QByteArray &text, Scintilla::Position line, 
                   Scintilla::FoldLevel foldNow, Scintilla::FoldLevel foldPrev);
    void onScrolled();
    void onScrollStopped();
    
    // Background highlighting slots
    void onBackgroundHighlightChunk(); // Process next chunk
    void onUserActivity();              // User became active
    void onUserIdle();                  // User became idle

private:
    QString m_currentFilePath;
    int m_highlightedLine;
    QColor m_highlightColor;
    int m_lineOffset; // Track the offset for correct line numbering
    bool m_showFileLineNumbers; // Whether to show actual file line numbers
    int m_loadedStartLine;   // Track the currently loaded range
    int m_loadedEndLine;   // Track the currently loaded range
    
    // Progressive loading members
    int m_chunkSize;
    int m_firstChunkSize;
    QTimer *m_loadingTimer;
    QTextStream *m_fileStream;
    QFile *m_progressiveFile;
    int m_totalLines;
    int m_currentChunk;
    int m_totalChunks;
    int m_targetLine;
    int m_contextLines;
    bool m_isProgressiveLoading;
    bool m_abortLoading;
    
    // KLOGG-style line offset indexing
    QVector<qint64> m_lineOffsets;  // Byte offsets for each line
    bool m_isIndexed;
    mutable QMutex m_indexMutex;
    
    // Viewport highlighting members
    QTimer *m_scrollTimer;          // Detect scroll end
    int m_lastFirstVisibleLine;     // Track scroll position
    int m_lastVisibleLineCount;     // Track visible area
    QList<HighlightRule> m_lastAppliedViewportRules; // Cache for viewport rules
    bool m_useViewportHighlighting; // Current viewport mode state
    bool m_scrollingInProgress;     // Track if scrolling is in progress
    
    // Cached highlighting settings for scroll updates
    bool m_cachedCaseSensitive;     // Cached case sensitivity setting
    bool m_cachedHighlightSentence; // Cached highlight sentence setting
    bool m_cachedUseScintillaSearch; // Cached Scintilla search setting
    
    // Track highlighted ranges to avoid duplicate highlighting
    QList<QPair<int, int>> m_highlightedRanges; // List of (startLine, endLine) pairs that are already highlighted
    
    // Background highlighting infrastructure
    QTimer *m_backgroundHighlightTimer;     // Timer for chunked background highlighting
    QTimer *m_idleTimer;                    // Timer to detect when user becomes idle
    QTimer *m_continuousTimer;              // Timer for continuous processing when user is idle
    int m_backgroundHighlightLine;          // Current line being processed in background
    int m_backgroundChunkSize;              // Lines to process per chunk (default 1000)
    QString m_backgroundChunkMode;          // "Lines" or "Duration" - controls chunking method
    int m_backgroundDurationMs;             // Duration in ms for time-based chunking
    bool m_userActive;                      // Track if user is currently active
    bool m_fullyHighlightedFile;            // Flag: true when entire file is highlighted
    bool m_backgroundHighlightingActive;    // Flag: true when background highlighting is running
    QList<HighlightRule> m_backgroundRules; // Rules to use for background highlighting
    bool m_backgroundCaseSensitive;         // Settings for background highlighting
    bool m_backgroundHighlightSentence;     // Settings for background highlighting
    
    // KLOGG constants
    static constexpr int KLOGG_INDEXING_BLOCK_SIZE = 5 * 1024 * 1024; // 5MB like KLOGG
    static constexpr int KLOGG_PREFETCH_BUFFER_SIZE_MB = 16; // 16MB like KLOGG
    static constexpr int KLOGG_SEARCH_BUFFER_SIZE_LINES = 10000; // 10K lines like KLOGG
    
    void setupScintilla();
    void setupDefaultStyle();
    QString convertToUtf8(const QByteArray &data);
    void updateLineNumbers();
    bool isLineInLoadedRange(int lineNumber) const;
    void expandLoadedRange(int newStartLine, int newEndLine);
    
    // KLOGG-style indexing methods
    void buildLineOffsetIndex(const QString &filePath);
    qint64 getLineOffset(int lineNumber) const;
    QString getLineAtOffset(qint64 startOffset, qint64 endOffset) const;
    void loadLinesFromIndex(int startLine, int endLine);
};

#endif // SCINTILLAEDIT_H 