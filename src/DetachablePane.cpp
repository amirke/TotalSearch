#include "DetachablePane.h"
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QDebug>

DetachablePane::DetachablePane(const QString& title, QWidget* parent)
    : QWidget(parent)
    , m_contentWidget(nullptr)
    , m_isDetached(false)
    , m_paneTitle(title)
    , m_defaultSize(800, 600)
    , m_detachedMaximized(false)
    , m_originalParent(nullptr)
    , m_originalLayout(nullptr)
{
    setupUI();
    restoreWindowState();
}

DetachablePane::~DetachablePane()
{
    saveWindowState();
}

void DetachablePane::setupUI()
{
    // Main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // Title bar layout
    m_titleBarLayout = new QHBoxLayout();
    m_titleBarLayout->setContentsMargins(8, 4, 8, 4);
    m_titleBarLayout->setSpacing(4);

    // Title area with icon and text
    QWidget* titleArea = new QWidget();
    QHBoxLayout* titleLayout = new QHBoxLayout(titleArea);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(4);
    
    // Icon label
    QLabel* iconLabel = new QLabel();
    iconLabel->setFixedSize(16, 16);
    
    // Set icon based on pane title
    if (m_paneTitle.contains("Search Results", Qt::CaseInsensitive)) {
        iconLabel->setPixmap(QIcon(":/icons/search_results.png").pixmap(16, 16));
    } else if (m_paneTitle.contains("File Viewer", Qt::CaseInsensitive) || 
               m_paneTitle.contains("File Content", Qt::CaseInsensitive)) {
        iconLabel->setPixmap(QIcon(":/icons/file_viewer.png").pixmap(16, 16));
    } else {
        iconLabel->hide(); // Hide icon if no match
    }
    
    // Title label
    m_titleLabel = new QLabel(m_paneTitle);
    m_titleLabel->setStyleSheet("QLabel { font-weight: bold; color: #2c3e50; }");
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    // Add icon and text to title area
    titleLayout->addWidget(iconLabel);
    titleLayout->addWidget(m_titleLabel);

    // Control buttons
    m_detachButton = new QPushButton();
    m_detachButton->setIcon(QIcon(":/icons/detach_pane.png"));
    m_detachButton->setToolTip("Detach to separate window");
    m_detachButton->setFixedSize(24, 24);
    m_detachButton->setStyleSheet(
        "QPushButton { border: none; background: transparent; }"
        "QPushButton:hover { background-color: #e0e0e0; border-radius: 3px; }"
        "QPushButton:pressed { background-color: #d0d0d0; }"
    );

    m_attachButton = new QPushButton();
    m_attachButton->setIcon(QIcon(":/icons/attach_pane.png"));
    m_attachButton->setToolTip("Attach to main window");
    m_attachButton->setFixedSize(24, 24);
    m_attachButton->setStyleSheet(
        "QPushButton { border: none; background: transparent; }"
        "QPushButton:hover { background-color: #e0e0e0; border-radius: 3px; }"
        "QPushButton:pressed { background-color: #d0d0d0; }"
    );
    m_attachButton->hide(); // Initially hidden

    m_maximizeButton = new QPushButton();
    m_maximizeButton->setIcon(QIcon(":/icons/maximize_window.png"));
    m_maximizeButton->setToolTip("Maximize window");
    m_maximizeButton->setFixedSize(24, 24);
    m_maximizeButton->setStyleSheet(
        "QPushButton { border: none; background: transparent; }"
        "QPushButton:hover { background-color: #e0e0e0; border-radius: 3px; }"
        "QPushButton:pressed { background-color: #d0d0d0; }"
    );
    m_maximizeButton->hide(); // Initially hidden

    m_closeButton = new QPushButton();
    m_closeButton->setIcon(QIcon(":/icons/close_window.png"));
    m_closeButton->setToolTip("Close window");
    m_closeButton->setFixedSize(24, 24);
    m_closeButton->setStyleSheet(
        "QPushButton { border: none; background: transparent; }"
        "QPushButton:hover { background-color: #ff6b6b; border-radius: 3px; }"
        "QPushButton:pressed { background-color: #ff5252; }"
    );
    m_closeButton->hide(); // Initially hidden

    // Add widgets to title bar
    m_titleBarLayout->addWidget(titleArea);
    m_titleBarLayout->addWidget(m_detachButton);
    m_titleBarLayout->addWidget(m_attachButton);
    m_titleBarLayout->addWidget(m_maximizeButton);
    m_titleBarLayout->addWidget(m_closeButton);

    // Create title bar widget
    QWidget* titleBarWidget = new QWidget();
    titleBarWidget->setLayout(m_titleBarLayout);
    titleBarWidget->setStyleSheet(
        "QWidget { background-color: #f8f9fa; border-bottom: 1px solid #dee2e6; }"
    );
    titleBarWidget->setFixedHeight(32);

    // Add title bar to main layout
    m_mainLayout->addWidget(titleBarWidget);

    // Connect signals
    connect(m_detachButton, &QPushButton::clicked, this, &DetachablePane::onDetachButtonClicked);
    connect(m_attachButton, &QPushButton::clicked, this, &DetachablePane::onAttachButtonClicked);
    connect(m_maximizeButton, &QPushButton::clicked, this, &DetachablePane::onMaximizeButtonClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &DetachablePane::onCloseButtonClicked);

    // Set initial style
    setStyleSheet(
        "DetachablePane { background-color: white; border: 1px solid #dee2e6; border-radius: 4px; }"
    );
}

void DetachablePane::setContentWidget(QWidget* widget)
{
    if (m_contentWidget) {
        m_mainLayout->removeWidget(m_contentWidget);
        m_contentWidget->setParent(nullptr);
    }

    m_contentWidget = widget;
    if (m_contentWidget) {
        m_contentWidget->setParent(this);
        m_mainLayout->addWidget(m_contentWidget);
    }
}

void DetachablePane::setOriginalParent(QWidget* parent, QLayout* layout)
{
    m_originalParent = parent;
    m_originalLayout = layout;
}

void DetachablePane::detachFromMain()
{
    if (m_isDetached) return;

    qDebug() << "DetachablePane: Detaching pane" << m_paneTitle << "from main window";
    
    m_isDetached = true;
    
    // Store original parent and layout if not already set
    if (!m_originalParent && parentWidget()) {
        m_originalParent = parentWidget();
        qDebug() << "DetachablePane: Stored original parent:" << m_originalParent->objectName();
        
        // Find the layout that contains this widget
        if (m_originalParent) {
            QLayout* parentLayout = m_originalParent->layout();
            if (parentLayout) {
                // Find which layout item contains this widget
                for (int i = 0; i < parentLayout->count(); ++i) {
                    QLayoutItem* item = parentLayout->itemAt(i);
                    if (item && item->widget() == this) {
                        m_originalLayout = parentLayout;
                        qDebug() << "DetachablePane: Found original layout at index" << i;
                        break;
                    }
                }
            }
        }
    }
    
    // Save current position relative to parent
    if (parentWidget()) {
        m_detachedPosition = mapToGlobal(pos());
    }

    // Remove from current parent
    if (parentWidget()) {
        qDebug() << "DetachablePane: Removing from current parent";
        setParent(nullptr);
    }

    // Setup as detached window
    setupDetachedWindow();
    
    // Show detached window
    show();
    raise();
    activateWindow();
    
    emit paneDetached(this);
}

void DetachablePane::attachToMain()
{
    if (!m_isDetached) return;

    qDebug() << "DetachablePane: Attaching pane" << m_paneTitle << "back to main window";
    
    m_isDetached = false;
    
    // Save window state before attaching
    saveWindowState();
    
    // Hide the detached window
    hide();
    
    // Setup as attached widget
    setupAttachedWidget();
    
    // Reparent back to original parent and layout
    if (m_originalParent && m_originalLayout) {
        qDebug() << "DetachablePane: Reparenting to original parent and layout";
        setParent(m_originalParent);
        m_originalLayout->addWidget(this);
        show();
    } else {
        qDebug() << "DetachablePane: ERROR - No original parent or layout found!";
    }
    
    emit paneAttached(this);
}

void DetachablePane::setupDetachedWindow()
{
    // Set window flags for detached mode - remove close button
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    
    // Set window title and create programmatic icon
    setWindowTitle(m_paneTitle);
    
    // Use app_icon.ico for all detached panes
    setWindowIcon(QIcon(":/icons/app_icon.ico"));
    
    // Show control buttons for detached mode - hide close button
    m_detachButton->hide();
    m_attachButton->show();
    m_maximizeButton->show();
    m_closeButton->hide(); // Hide close button in detached mode
    
    // Restore window state
    restoreWindowState();
    
    // Set minimum size
    setMinimumSize(400, 300);
}

void DetachablePane::setupAttachedWidget()
{
    // Reset window flags for attached mode
    setWindowFlags(Qt::Widget);
    
    // Hide control buttons for attached mode
    m_detachButton->show();
    m_attachButton->hide();
    m_maximizeButton->hide();
    m_closeButton->hide();
    
    // Reset window title
    setWindowTitle("");
}

void DetachablePane::saveWindowState()
{
    if (!m_isDetached) return;

    QSettings settings("app.ini", QSettings::IniFormat);
    settings.beginGroup("DetachedPanes");
    settings.beginGroup(m_paneTitle);
    
    settings.setValue("position", pos());
    settings.setValue("size", size());
    settings.setValue("maximized", isMaximized());
    
    settings.endGroup();
    settings.endGroup();
}

void DetachablePane::restoreWindowState()
{
    QSettings settings("app.ini", QSettings::IniFormat);
    settings.beginGroup("DetachedPanes");
    settings.beginGroup(m_paneTitle);
    
    QPoint savedPos = settings.value("position", QPoint()).toPoint();
    QSize savedSize = settings.value("size", m_defaultSize).toSize();
    bool savedMaximized = settings.value("maximized", false).toBool();
    
    settings.endGroup();
    settings.endGroup();
    
    if (m_isDetached) {
        // Check if position is valid (on screen)
        bool validPosition = false;
        for (QScreen* screen : QApplication::screens()) {
            if (screen->geometry().contains(savedPos)) {
                validPosition = true;
                break;
            }
        }
        
        if (validPosition) {
            move(savedPos);
        } else {
            // Center on primary screen
            QScreen* primaryScreen = QApplication::primaryScreen();
            if (primaryScreen) {
                QRect screenGeometry = primaryScreen->geometry();
                move(screenGeometry.center() - rect().center());
            }
        }
        
        if (savedMaximized) {
            showMaximized();
        } else {
            resize(savedSize);
        }
    }
}

void DetachablePane::setDefaultSize(const QSize& size)
{
    m_defaultSize = size;
}

void DetachablePane::closeEvent(QCloseEvent* event)
{
    if (m_isDetached) {
        saveWindowState();
        emit paneClosed(this);
    }
    event->accept();
}

void DetachablePane::moveEvent(QMoveEvent* event)
{
    if (m_isDetached) {
        // Save position periodically
        QTimer::singleShot(100, this, &DetachablePane::saveWindowState);
    }
    QWidget::moveEvent(event);
}

void DetachablePane::resizeEvent(QResizeEvent* event)
{
    if (m_isDetached) {
        // Save size periodically
        QTimer::singleShot(100, this, &DetachablePane::saveWindowState);
    }
    QWidget::resizeEvent(event);
}

void DetachablePane::onDetachButtonClicked()
{
    detachFromMain();
}

void DetachablePane::onAttachButtonClicked()
{
    attachToMain();
}

void DetachablePane::onMaximizeButtonClicked()
{
    if (isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
}

void DetachablePane::onCloseButtonClicked()
{
    close();
}

void DetachablePane::setPaneTitle(const QString& title)
{
    m_paneTitle = title;
    
    // Update the title label in the title bar
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
    
    // Update window title if detached
    if (m_isDetached) {
        QWidget::setWindowTitle(title);
    }
    
    qDebug() << "DetachablePane: Updated pane title to:" << title;
}

void DetachablePane::setWindowTitle(const QString& title)
{
    // Update window title (works for both attached and detached modes)
    QWidget::setWindowTitle(title);
    
    // If this is a custom title (different from pane title), 
    // we can optionally update the pane title too
    if (title != m_paneTitle) {
        qDebug() << "DetachablePane: Updated window title to:" << title;
    }
}
