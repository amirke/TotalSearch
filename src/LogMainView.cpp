#include "LogMainView.h"
#include "LogDataWorker.h"
#include "logger.h"
#include <QPainter>
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTextLayout>
#include <QFontMetrics>
#include <QDebug>

LogMainView::LogMainView(QWidget *parent)
    : QWidget(parent)
    , logDataWorker_(nullptr)
    , showLineNumbers_(true)
    , font_(QFont("Consolas", 10))
    , tabWidth_(4)
    , totalLines_(0)
    , viewTop_(0)
    , viewBottom_(0)
    , scrollOffset_(0)
    , horizontalOffset_(0)
    , lineHeight_(0)
    , lineNumberWidth_(0)
    , contentWidth_(0)
    , viewportHeight_(0)
    , viewportWidth_(0)
    , highlightedLine_(-1)
{
    setFocusPolicy(Qt::StrongFocus);
    
    // Create scroll bars
    verticalScrollBar_ = new QScrollBar(Qt::Vertical, this);
    horizontalScrollBar_ = new QScrollBar(Qt::Horizontal, this);
    
    // Connect scroll bar signals
    connect(verticalScrollBar_, &QScrollBar::valueChanged, 
            this, &LogMainView::onVerticalScrollBarValueChanged);
    connect(horizontalScrollBar_, &QScrollBar::valueChanged,
            [this](int value) {
                horizontalOffset_ = value;
                update();
            });
    
    // Set up text layout
    textLayout_.setFont(font_);
    
    // Calculate initial layout
    updateScrollBars();
}

LogMainView::~LogMainView()
{
}

void LogMainView::setLogDataWorker(LogDataWorker* worker)
{
    LOG_DEBUG("LogMainView::setLogDataWorker - Worker connected");
    logDataWorker_ = worker;
    if (worker) {
        connect(worker, &LogDataWorker::indexingFinished, 
                this, &LogMainView::updateViewport);
        connect(worker, &LogDataWorker::viewportUpdateRequested,
                this, &LogMainView::updateViewport);
        LOG_DEBUG("LogMainView::setLogDataWorker - Signals connected");
    }
}

void LogMainView::setLineNumbersVisible(bool visible)
{
    if (showLineNumbers_ != visible) {
        showLineNumbers_ = visible;
        updateScrollBars();
        update();
    }
}

void LogMainView::setFont(const QFont& font)
{
    font_ = font;
    textLayout_.setFont(font);
    updateScrollBars();
    update();
}

void LogMainView::setTabWidth(int width)
{
    if (tabWidth_ != width) {
        tabWidth_ = width;
        updateScrollBars();
        update();
    }
}

void LogMainView::clear()
{
    totalLines_ = 0;
    viewTop_ = 0;
    viewBottom_ = 0;
    visibleLines_.clear();
    visibleLineIndices_.clear();
    updateScrollBars();
    update();
}

void LogMainView::updateViewport()
{
    LOG_DEBUG("LogMainView::updateViewport - Starting update");
    if (!logDataWorker_) {
        LOG_DEBUG("LogMainView::updateViewport - No LogDataWorker available");
        return;
    }
    
    // Get total lines from LogDataWorker
    int totalLines = logDataWorker_->getTotalLines();
    LOG_DEBUG("LogMainView::updateViewport - Total lines: " + QString::number(totalLines));
    
    setTotalLines(totalLines);
    
    // Calculate visible lines
    calculateVisibleLines();
    
    // Update scroll bars
    updateScrollBars();
    
    update(); // Trigger repaint
    LOG_DEBUG("LogMainView::updateViewport - Update completed");
}

int LogMainView::getTotalLines() const
{
    return totalLines_;
}

void LogMainView::setTotalLines(int lines)
{
    totalLines_ = lines;
    updateScrollBars();
}

void LogMainView::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(font_);
    
    // Fill background
    painter.fillRect(rect(), Qt::white);
    
    if (!logDataWorker_ || !logDataWorker_->isFileLoaded() || visibleLineIndices_.isEmpty()) {
        return;
    }
    
    // Draw line numbers area if enabled
    if (showLineNumbers_) {
        painter.fillRect(0, 0, lineNumberWidth_, height(), QColor(240, 240, 240));
        painter.setPen(Qt::darkGray);
        painter.drawLine(lineNumberWidth_ - 1, 0, lineNumberWidth_ - 1, height());
    }
    
    // Draw visible lines
    int y = 0;
    for (int i = 0; i < visibleLineIndices_.size(); ++i) {
        int lineIndex = visibleLineIndices_[i];
        QString lineText = visibleLines_[i];
        
        drawLine(painter, lineIndex, y, lineText);
        y += lineHeight_;
    }
}

void LogMainView::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateScrollBars();
    calculateVisibleLines();
}

void LogMainView::wheelEvent(QWheelEvent* event)
{
    if (event->angleDelta().y() != 0) {
        int delta = event->angleDelta().y() / 120; // Standard wheel delta
        int newValue = verticalScrollBar_->value() - delta * 3; // 3 lines per wheel step
        verticalScrollBar_->setValue(newValue);
        event->accept();
    } else if (event->angleDelta().x() != 0) {
        int delta = event->angleDelta().x() / 120;
        int newValue = horizontalScrollBar_->value() - delta * 20; // 20 pixels per wheel step
        horizontalScrollBar_->setValue(newValue);
        event->accept();
    }
}

void LogMainView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        setFocus();
    }
    QWidget::mousePressEvent(event);
}

void LogMainView::mouseMoveEvent(QMouseEvent* event)
{
    // Handle mouse movement if needed
    QWidget::mouseMoveEvent(event);
}

void LogMainView::calculateVisibleLines()
{
    LOG_DEBUG("LogMainView::calculateVisibleLines - Starting calculation");
    if (!logDataWorker_ || !logDataWorker_->isFileLoaded()) {
        LOG_DEBUG("LogMainView::calculateVisibleLines - LogDataWorker not available or file not loaded");
        return;
    }
    
    // Calculate visible line range
    int firstVisibleLine = scrollOffset_ / lineHeight_;
    int visibleLineCount = viewportHeight_ / lineHeight_ + 2; // +2 for partial lines
    
    viewTop_ = qMax(0, firstVisibleLine);
    viewBottom_ = qMin(totalLines_ - 1, viewTop_ + visibleLineCount);
    LOG_DEBUG("LogMainView::AAA1 firstVisibleLine: " + QString::number(firstVisibleLine));
    LOG_DEBUG("LogMainView::AAA2 visibleLineCount: " + QString::number(visibleLineCount));
    LOG_DEBUG("LogMainView::AAA3 totalLines: " + QString::number(totalLines_));
    LOG_DEBUG("LogMainView::AAA4 viewTop: " + QString::number(viewTop_));
    LOG_DEBUG("LogMainView::AAA5 viewBottom: " + QString::number(viewBottom_));

    
 // Amir     viewBottom_ = qMin(totalLines_, viewTop_ + visibleLineCount); // Allow scrolling beyond last line

    
    // Get lines from LogDataWorker
    int lineCount = viewBottom_ - viewTop_ + 1;
    
    if (lineCount > 0) {
        visibleLines_ = logDataWorker_->getLines(viewTop_, lineCount);
        
        visibleLineIndices_.clear();
        for (int i = viewTop_; i <= viewBottom_; ++i) {
            visibleLineIndices_.append(i);
        }
        
        LOG_DEBUG("LogMainView::calculateVisibleLines - Loaded " + QString::number(visibleLines_.size()) + " lines");
    } else {
        visibleLines_.clear();
        visibleLineIndices_.clear();
        LOG_DEBUG("LogMainView::calculateVisibleLines - No visible lines to display");
    }
}

void LogMainView::drawLine(QPainter& painter, int lineIndex, int y, const QString& lineText)
{
    int x = 0;
    
    // Check if this line should be highlighted (jump to line highlighting)
    bool isHighlighted = (lineIndex == highlightedLine_);
    
    // Check if this line is a search result
    bool isSearchResult = hasSearchResults_ && searchResults_.contains(lineIndex); // Already 0-based
    
    // Draw highlight background if needed
    if (isSearchResult) {
        // Search result highlighting takes precedence
        painter.fillRect(0, y, viewportWidth_, lineHeight_, searchHighlightColor_);
    } else if (isHighlighted) {
        // Jump to line highlighting (yellow)
        painter.fillRect(0, y, viewportWidth_, lineHeight_, QColor(255, 255, 0));
    }
    
    // Draw line number if enabled
    if (showLineNumbers_) {
        QString lineNumberText = getLineNumberText(lineIndex + 1);
        painter.setPen(Qt::darkGray);
        painter.drawText(x, y, lineNumberWidth_ - 5, lineHeight_, 
                        Qt::AlignRight | Qt::AlignVCenter, lineNumberText);
        x = lineNumberWidth_;
    }
    
    // Draw line content
    painter.setPen(Qt::black);
    
    // Use QTextLayout for proper text rendering
    textLayout_.setText(lineText);
    textLayout_.beginLayout();
    
    QTextLine line = textLayout_.createLine();
    if (line.isValid()) {
        line.setLineWidth(viewportWidth_ - x - horizontalOffset_);
        line.setPosition(QPointF(x - horizontalOffset_, y));
        textLayout_.endLayout();
        
        // Draw the text line
        textLayout_.draw(&painter, QPointF(0, 0));
    } else {
        textLayout_.endLayout();
        // Fallback: draw text directly
        painter.drawText(x - horizontalOffset_, y, viewportWidth_ - x, lineHeight_,
                        Qt::AlignLeft | Qt::AlignVCenter, lineText);
    }
}



QString LogMainView::getLineNumberText(int lineNumber) const
{
    return QString::number(lineNumber);
}

int LogMainView::getLineNumberWidth() const
{
    if (!showLineNumbers_) {
        return 0;
    }
    
    QFontMetrics fm(font_);
    QString maxLineNumber = QString::number(totalLines_);
    return fm.horizontalAdvance(maxLineNumber) + 20; // 20 pixels padding
}

int LogMainView::getContentWidth() const
{
    // This would need to be calculated based on actual content
    // For now, use a reasonable default
    return 1000;
}

int LogMainView::getLineHeight() const
{
    QFontMetrics fm(font_);
    return fm.height();
}

int LogMainView::viewportToLine(int y) const
{
    return (y + scrollOffset_) / lineHeight_;
}

int LogMainView::lineToViewport(int lineIndex) const
{
    return lineIndex * lineHeight_ - scrollOffset_;
}

void LogMainView::updateScrollBars()
{
    QFontMetrics fm(font_);
    lineHeight_ = fm.height();
    lineNumberWidth_ = getLineNumberWidth();
    contentWidth_ = getContentWidth();
    viewportHeight_ = height() - horizontalScrollBar_->sizeHint().height();
    viewportWidth_ = width() - verticalScrollBar_->sizeHint().width();
    
    // Update vertical scroll bar
    int maxValue = qMax(0, totalLines_ * lineHeight_ - viewportHeight_);
    verticalScrollBar_->setRange(0, maxValue);
    verticalScrollBar_->setPageStep(viewportHeight_);
    verticalScrollBar_->setSingleStep(lineHeight_);
    
    // Update horizontal scroll bar
    int maxHValue = qMax(0, contentWidth_ - viewportWidth_);
    horizontalScrollBar_->setRange(0, maxHValue);
    horizontalScrollBar_->setPageStep(viewportWidth_);
    horizontalScrollBar_->setSingleStep(20);
    
    // Position scroll bars
    verticalScrollBar_->setGeometry(width() - verticalScrollBar_->sizeHint().width(), 0,
                                   verticalScrollBar_->sizeHint().width(), height() - horizontalScrollBar_->sizeHint().height());
    horizontalScrollBar_->setGeometry(0, height() - horizontalScrollBar_->sizeHint().height(),
                                     width() - verticalScrollBar_->sizeHint().width(), horizontalScrollBar_->sizeHint().height());
}

void LogMainView::onVerticalScrollBarValueChanged(int value)
{
    scrollOffset_ = value;
    calculateVisibleLines();
    update();
}

void LogMainView::scrollToLine(int lineNumber)
{
    LOG_DEBUG("LogMainView::scrollToLine - Scrolling to line " + QString::number(lineNumber));
    
    if (lineNumber < 0 || lineNumber >= totalLines_) {
        LOG_DEBUG("LogMainView::scrollToLine - Invalid line number");
        return;
    }
    
    // Calculate the scroll position to center the line in the viewport
    int targetScrollOffset = lineNumber * lineHeight_ - (viewportHeight_ / 2);
    
    // Clamp to valid range
    int maxScrollOffset = qMax(0, totalLines_ * lineHeight_ - viewportHeight_);
    targetScrollOffset = qBound(0, targetScrollOffset, maxScrollOffset);
    
    // Set the scroll bar value
    verticalScrollBar_->setValue(targetScrollOffset);
    
    LOG_DEBUG("LogMainView::scrollToLine - Scroll offset set to " + QString::number(targetScrollOffset));
}

void LogMainView::setHighlightedLine(int lineNumber)
{
    LOG_DEBUG("LogMainView::setHighlightedLine - Setting highlighted line to " + QString::number(lineNumber));
    
    if (highlightedLine_ != lineNumber) {
        highlightedLine_ = lineNumber;
        update(); // Trigger repaint to show highlighting
        LOG_DEBUG("LogMainView::setHighlightedLine - Highlight updated");
    }
}

void LogMainView::setSearchResults(const QList<int>& searchResults, const QColor& highlightColor)
{
    LOG_DEBUG("LogMainView::setSearchResults - Setting " + QString::number(searchResults.size()) + " search results");
    
    // Convert 1-based line numbers to 0-based indices for internal storage
    searchResults_.clear();
    for (int lineNumber : searchResults) {
        searchResults_.append(lineNumber - 1); // Convert to 0-based
    }
    
    searchHighlightColor_ = highlightColor;
    hasSearchResults_ = !searchResults.isEmpty();
    
    update(); // Trigger repaint to show search result highlighting
    LOG_DEBUG("LogMainView::setSearchResults - Search results updated with " + QString::number(searchResults_.size()) + " converted results");
}

void LogMainView::clearSearchResults()
{
    LOG_DEBUG("LogMainView::clearSearchResults - Clearing search results");
    
    searchResults_.clear();
    hasSearchResults_ = false;
    
    update(); // Trigger repaint to remove search result highlighting
    LOG_DEBUG("LogMainView::clearSearchResults - Search results cleared");
}

bool LogMainView::hasSearchResults() const
{
    return hasSearchResults_;
} 