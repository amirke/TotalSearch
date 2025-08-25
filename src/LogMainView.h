#ifndef LOGMAINVIEW_H
#define LOGMAINVIEW_H

#include <QWidget>
#include <QPainter>
#include <QTextLayout>
#include <QFontMetrics>
#include <QScrollBar>
#include <QVector>

// Forward declarations
class LogDataWorker;

class LogMainView : public QWidget {
    Q_OBJECT
    
public:
    explicit LogMainView(QWidget *parent = nullptr);
    ~LogMainView();
    
    // Set the LogDataWorker for line access
    void setLogDataWorker(LogDataWorker* worker);
    
    // Viewport settings
    void setLineNumbersVisible(bool visible);
    void setFont(const QFont& font);
    void setTabWidth(int width);
    
    // Clear the view
    void clear();
    
    // Update viewport (called when scroll position changes)
    void updateViewport();
    
    // Get total lines count
    int getTotalLines() const;
    
    // Set total lines count
    void setTotalLines(int lines);
    
    // Scroll to specific line
    void scrollToLine(int lineNumber);
    
    // Set highlighted line
    void setHighlightedLine(int lineNumber);
    
    // Search result highlighting
    void setSearchResults(const QList<int>& searchResults, const QColor& highlightColor);
    void clearSearchResults();
    bool hasSearchResults() const;
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    
private slots:
    void onVerticalScrollBarValueChanged(int value);
    
private:
    // Calculate visible line range based on viewport
    void calculateVisibleLines();
    
    // Draw a single line with line number
    void drawLine(QPainter& painter, int lineIndex, int y, const QString& lineText);
    
    // Update scroll bars
    void updateScrollBars();
    
    // Get line number text
    QString getLineNumberText(int lineNumber) const;
    
    // Calculate line number width
    int getLineNumberWidth() const;
    
    // Calculate content width
    int getContentWidth() const;
    
    // Calculate line height
    int getLineHeight() const;
    
    // Convert viewport coordinates to line index
    int viewportToLine(int y) const;
    
    // Convert line index to viewport y coordinate
    int lineToViewport(int lineIndex) const;
    
private:
    LogDataWorker* logDataWorker_;
    
    // Viewport settings
    bool showLineNumbers_;
    QFont font_;
    int tabWidth_;
    int totalLines_;
    
    // Viewport state
    int viewTop_;           // First visible line index
    int viewBottom_;        // Last visible line index
    int scrollOffset_;      // Vertical scroll offset in pixels
    int horizontalOffset_;  // Horizontal scroll offset in pixels
    
    // Layout calculations
    int lineHeight_;
    int lineNumberWidth_;
    int contentWidth_;
    int viewportHeight_;
    int viewportWidth_;
    
    // Highlighting
    int highlightedLine_;   // Currently highlighted line (-1 for none)
    
    // Search result highlighting
    QList<int> searchResults_;
    QColor searchHighlightColor_;
    bool hasSearchResults_;
    
    // Scroll bars
    QScrollBar* verticalScrollBar_;
    QScrollBar* horizontalScrollBar_;
    
    // Cached visible lines
    QVector<QString> visibleLines_;
    QVector<int> visibleLineIndices_;
    
    // Text layout for line rendering
    QTextLayout textLayout_;
};

#endif // LOGMAINVIEW_H 