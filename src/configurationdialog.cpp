#include "configurationdialog.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFontDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QFrame>
#include <QGroupBox>
#include <QRadioButton>
#include <QApplication>
#include <QDebug>
#include <QPainter>

ConfigurationDialog::ConfigurationDialog(QWidget *parent)
    : QDialog(parent), m_mainWindow(nullptr)
{
    setupUI();
    loadSettings();
}

ConfigurationDialog::~ConfigurationDialog()
{
}

void ConfigurationDialog::setupUI()
{
    setWindowTitle("Configuration");
    setModal(true);
    resize(800, 600);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    tabWidget = new QTabWidget(this);
    
    // Setup all tabs
    setupTab0_General();
    setupTab1_Fonts();
    setupTab2_Layout();
    setupTab3_BackgroundHighlighting();
    setupTab4_HighlightEngines();
    setupTab5_CustomColors();
    
    mainLayout->addWidget(tabWidget);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    doneButton = new QPushButton("Apply", this);
    // Create check mark icon using Unicode character
    QPixmap checkPixmap(16, 16);
    checkPixmap.fill(Qt::transparent);
    QPainter painter(&checkPixmap);
    painter.setPen(QPen(Qt::white, 2));
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(checkPixmap.rect(), Qt::AlignCenter, "✓");
    doneButton->setIcon(QIcon(checkPixmap));
    cancelButton = new QPushButton("Cancel", this);
    cancelButton->setIcon(QIcon(":/icons/close_window.png"));
    
    doneButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #28a745, stop:1 #20c997);"
        "    border: none;"
        "    color: white;"
        "    padding: 8px 16px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    border-radius: 6px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #20c997, stop:1 #17a2b8);"
        "    transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #17a2b8, stop:1 #138496);"
        "    transform: translateY(1px);"
        "}"
    );
    
    cancelButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6c757d, stop:1 #495057);"
        "    border: none;"
        "    color: white;"
        "    padding: 8px 16px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    border-radius: 6px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #868e96, stop:1 #6c757d);"
        "    transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #495057, stop:1 #343a40);"
        "    transform: translateY(1px);"
        "}"
    );
    
    buttonLayout->addWidget(doneButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(doneButton, &QPushButton::clicked, this, &ConfigurationDialog::onDoneClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ConfigurationDialog::onCancelClicked);
}

void ConfigurationDialog::setupTab0_General()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    
    // History Management Group
    QGroupBox *historyGroup = new QGroupBox("Search History Management", tab);
    QVBoxLayout *historyLayout = new QVBoxLayout(historyGroup);
    
    QLabel *historyInfoLabel = new QLabel("Clear all saved search patterns and paths from history.\nThis action cannot be undone.", historyGroup);
    historyInfoLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");
    
    clearHistoryButton = new QPushButton("🗑️ Clear All History", historyGroup);
    clearHistoryButton->setToolTip("Delete all pattern and path history entries");
    clearHistoryButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #dc3545, stop:1 #c82333);"
        "    border: none;"
        "    color: white;"
        "    padding: 10px 20px;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "    border-radius: 6px;"
        "    min-width: 150px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #c82333, stop:1 #bd2130);"
        "    transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #bd2130, stop:1 #a71e2a);"
        "    transform: translateY(1px);"
        "}"
    );
    
    historyLayout->addWidget(historyInfoLabel);
    historyLayout->addWidget(clearHistoryButton);
    historyLayout->addStretch();
    
    layout->addWidget(historyGroup);
    layout->addStretch();
    
    tabWidget->addTab(tab, "⚙️ General");
    
    // Connect clear history button
    connect(clearHistoryButton, &QPushButton::clicked, this, &ConfigurationDialog::onClearHistoryClicked);
}

void ConfigurationDialog::setupTab1_Fonts()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    
    // Search Results Font
    QGroupBox *searchResultsGroup = new QGroupBox("Search Results Font", tab);
    QVBoxLayout *searchResultsLayout = new QVBoxLayout(searchResultsGroup);
    
    QHBoxLayout *fontLayout = new QHBoxLayout();
    QLabel *fontLabel = new QLabel("Font:", searchResultsGroup);
    fontButton = new QPushButton("Choose Font", searchResultsGroup);
    fontButton->setIcon(QIcon(":/icons/file.png"));
    fontSizeEdit = new QLineEdit("12", searchResultsGroup);
    fontSizeEdit->setMaximumWidth(60);
    fontPreviewLabel = new QLabel("Sample Text - Search Results", searchResultsGroup);
    fontPreviewLabel->setStyleSheet("QLabel { border: 1px solid #ccc; padding: 8px; background: white; }");
    
    fontLayout->addWidget(fontLabel);
    fontLayout->addWidget(fontButton);
    fontLayout->addWidget(new QLabel("Size:", searchResultsGroup));
    fontLayout->addWidget(fontSizeEdit);
    fontLayout->addStretch();
    
    searchResultsLayout->addLayout(fontLayout);
    searchResultsLayout->addWidget(fontPreviewLabel);
    
    // File Content Font
    QGroupBox *fileContentGroup = new QGroupBox("File Content Font", tab);
    QVBoxLayout *fileContentLayout = new QVBoxLayout(fileContentGroup);
    
    QHBoxLayout *previewFontLayout = new QHBoxLayout();
    QLabel *previewFontLabel = new QLabel("Font:", fileContentGroup);
    previewFontButton = new QPushButton("Choose Font", fileContentGroup);
    previewFontButton->setIcon(QIcon(":/icons/file.png"));
    previewFontSizeEdit = new QLineEdit("10", fileContentGroup);
    previewFontSizeEdit->setMaximumWidth(60);
    previewFontPreviewLabel = new QLabel("Sample Text - File Content", fileContentGroup);
    previewFontPreviewLabel->setStyleSheet("QLabel { border: 1px solid #ccc; padding: 8px; background: white; }");
    
    previewFontLayout->addWidget(previewFontLabel);
    previewFontLayout->addWidget(previewFontButton);
    previewFontLayout->addWidget(new QLabel("Size:", fileContentGroup));
    previewFontLayout->addWidget(previewFontSizeEdit);
    previewFontLayout->addStretch();
    
    fileContentLayout->addLayout(previewFontLayout);
    fileContentLayout->addWidget(previewFontPreviewLabel);
    
    layout->addWidget(searchResultsGroup);
    layout->addWidget(fileContentGroup);
    layout->addStretch();
    
    tabWidget->addTab(tab, "📝 Fonts");
    
    // Connect font buttons
    connect(fontButton, &QPushButton::clicked, this, &ConfigurationDialog::onFontButtonClicked);
    connect(previewFontButton, &QPushButton::clicked, this, &ConfigurationDialog::onPreviewFontButtonClicked);
}

void ConfigurationDialog::setupTab2_Layout()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    
    // Window Components Group
    QGroupBox *componentsGroup = new QGroupBox("Window Components", tab);
    QVBoxLayout *componentsLayout = new QVBoxLayout(componentsGroup);
    
    enableLogWindowCheck = new QCheckBox("Enable Log Window", componentsGroup);
    enableLogWindowCheck->setToolTip("Show/hide the log window at the bottom of the application");
    
    componentsLayout->addWidget(enableLogWindowCheck);
    
    // Layout Orientation Group
    QGroupBox *orientationGroup = new QGroupBox("Pane Layout", tab);
    QVBoxLayout *orientationLayout = new QVBoxLayout(orientationGroup);
    
    upDownLayoutRadio = new QRadioButton("Up/Down Layout", orientationGroup);
    upDownLayoutRadio->setToolTip("Search Results pane above File Viewer pane (vertical stacking)");
    upDownLayoutRadio->setChecked(true); // Default to current layout
    
    sideBySideLayoutRadio = new QRadioButton("Side by Side Layout", orientationGroup);
    sideBySideLayoutRadio->setToolTip("Search Results pane on left, File Viewer pane on right (horizontal arrangement)");
    
    orientationLayout->addWidget(upDownLayoutRadio);
    orientationLayout->addWidget(sideBySideLayoutRadio);
    
    // Layout Preview
    QGroupBox *previewGroup = new QGroupBox("Layout Preview", tab);
    QVBoxLayout *previewLayout = new QVBoxLayout(previewGroup);
    
    QLabel *previewLabel = new QLabel(previewGroup);
    previewLabel->setStyleSheet("QLabel { "
                               "font-family: monospace; "
                               "font-size: 10px; "
                               "background: #f8f9fa; "
                               "border: 1px solid #dee2e6; "
                               "padding: 12px; "
                               "}");
    previewLabel->setAlignment(Qt::AlignCenter);
    
    previewLayout->addWidget(previewLabel);
    
    // Add all groups to main layout
    layout->addWidget(componentsGroup);
    layout->addWidget(orientationGroup);
    layout->addWidget(previewGroup);
    layout->addStretch();
    
    tabWidget->addTab(tab, "📐 Layout");
    
    // Store preview label for updates
    m_layoutPreviewLabel = previewLabel;
    
    // Connect signals
    connect(enableLogWindowCheck, &QCheckBox::toggled, this, &ConfigurationDialog::onLayoutSettingsChanged);
    connect(upDownLayoutRadio, &QRadioButton::toggled, this, &ConfigurationDialog::onLayoutSettingsChanged);
    connect(sideBySideLayoutRadio, &QRadioButton::toggled, this, &ConfigurationDialog::onLayoutSettingsChanged);
    
    // Set initial preview after everything is set up
    updateLayoutPreview();
}

void ConfigurationDialog::setupTab3_BackgroundHighlighting()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    
    // Background Highlighting Group
    QGroupBox *backgroundGroup = new QGroupBox("Background Highlighting", tab);
    QVBoxLayout *backgroundLayout = new QVBoxLayout(backgroundGroup);
    
    backgroundHighlightingCheck = new QCheckBox("Enable Background Highlighting", backgroundGroup);
    backgroundHighlightingCheck->setToolTip("Highlight the entire file in the background when user is idle");
    
    // Chunk Mode
    QHBoxLayout *chunkModeLayout = new QHBoxLayout();
    QLabel *chunkModeLabel = new QLabel("Chunk Mode:", backgroundGroup);
    chunkModeCombo = new QComboBox(backgroundGroup);
    chunkModeCombo->addItems({"Lines", "Duration"});
    chunkModeCombo->setToolTip("Process by number of lines or time duration");
    
    chunkModeLayout->addWidget(chunkModeLabel);
    chunkModeLayout->addWidget(chunkModeCombo);
    chunkModeLayout->addStretch();
    
    // Chunk Size (Lines) - Improved with better descriptions
    QHBoxLayout *chunkSizeLayout = new QHBoxLayout();
    QLabel *chunkSizeLabel = new QLabel("Chunk Size:", backgroundGroup);
    chunkSizeCombo = new QComboBox(backgroundGroup);
    chunkSizeCombo->addItems({
        "100 lines (Very Fast)",
        "500 lines (Fast)", 
        "1000 lines (Balanced)",
        "2000 lines (Thorough)",
        "5000 lines (Comprehensive)",
        "10000 lines (Extensive)",
        "20000 lines (Deep)",
        "50000 lines (Very Deep)",
        "100000 lines (Maximum)",
        "Custom lines"
    });
    chunkSizeCombo->setToolTip("Number of lines to process in each background chunk\n\n"
                               "Smaller chunks = Faster response, more frequent updates\n"
                               "Larger chunks = Better performance, less UI updates\n"
                               "Recommended: 1000-5000 lines for most files");
    customLinesEdit = new QLineEdit("1000", backgroundGroup);
    customLinesEdit->setMaximumWidth(100);
    customLinesEdit->setPlaceholderText("Enter lines (100-1000000)");
    customLinesEdit->setToolTip("Custom number of lines per chunk\n\n"
                                "Enter a value between 100 and 1,000,000\n"
                                "Smaller values = More responsive UI\n"
                                "Larger values = Better performance");
    customLinesEdit->hide();
    
    chunkSizeLayout->addWidget(chunkSizeLabel);
    chunkSizeLayout->addWidget(chunkSizeCombo);
    chunkSizeLayout->addWidget(customLinesEdit);
    chunkSizeLayout->addStretch();
    
    // Duration
    QHBoxLayout *durationLayout = new QHBoxLayout();
    QLabel *durationLabel = new QLabel("Duration:", backgroundGroup);
    durationCombo = new QComboBox(backgroundGroup);
    durationCombo->addItems({"10ms", "50ms", "100ms", "500ms", "Custom ms"});
    customDurationEdit = new QLineEdit("50", backgroundGroup);
    customDurationEdit->setMaximumWidth(100);
    customDurationEdit->setPlaceholderText("Custom ms");
    customDurationEdit->hide();
    
    durationLayout->addWidget(durationLabel);
    durationLayout->addWidget(durationCombo);
    durationLayout->addWidget(customDurationEdit);
    durationLayout->addStretch();
    
    // Idle Delay
    QHBoxLayout *idleDelayLayout = new QHBoxLayout();
    QLabel *idleDelayLabel = new QLabel("Idle Delay:", backgroundGroup);
    idleDelayCombo = new QComboBox(backgroundGroup);
    idleDelayCombo->addItems({"0.05 seconds", "0.1 seconds", "0.5 seconds", "1 second", "2 seconds", "5 seconds"});
    idleDelayCombo->setToolTip("Delay before starting background highlighting after user becomes idle");
    
    idleDelayLayout->addWidget(idleDelayLabel);
    idleDelayLayout->addWidget(idleDelayCombo);
    idleDelayLayout->addStretch();
    
    // Performance Mode
    QHBoxLayout *performanceLayout = new QHBoxLayout();
    QLabel *performanceLabel = new QLabel("Performance Mode:", backgroundGroup);
    performanceModeCombo = new QComboBox(backgroundGroup);
    performanceModeCombo->addItems({"Fast", "Balanced", "Thorough"});
    performanceModeCombo->setToolTip("Fast: 5ms intervals, Balanced: 10ms intervals, Thorough: 20ms intervals");
    
    performanceLayout->addWidget(performanceLabel);
    performanceLayout->addWidget(performanceModeCombo);
    performanceLayout->addStretch();
    
    // Progress Indicator
    progressIndicatorCheck = new QCheckBox("Show Progress Indicator", backgroundGroup);
    progressIndicatorCheck->setToolTip("Display progress in status bar lamp");
    
    // Timing Diagram
    timingDiagram = new QLabel(backgroundGroup);
    timingDiagram->setStyleSheet("QLabel { font-family: monospace; font-size: 10px; background: #f8f9fa; border: 1px solid #dee2e6; padding: 8px; }");
    timingDiagram->setWordWrap(true);
    
    backgroundLayout->addWidget(backgroundHighlightingCheck);
    backgroundLayout->addLayout(chunkModeLayout);
    backgroundLayout->addLayout(chunkSizeLayout);
    backgroundLayout->addLayout(durationLayout);
    backgroundLayout->addLayout(idleDelayLayout);
    backgroundLayout->addLayout(performanceLayout);
    backgroundLayout->addWidget(progressIndicatorCheck);
    backgroundLayout->addWidget(timingDiagram);
    
    layout->addWidget(backgroundGroup);
    layout->addStretch();
    
    tabWidget->addTab(tab, "🔄 Background Highlighting");
    
    // Connect signals
    connect(backgroundHighlightingCheck, &QCheckBox::toggled, this, &ConfigurationDialog::onBackgroundHighlightingChanged);
    connect(chunkModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConfigurationDialog::onChunkModeChanged);
    connect(chunkSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConfigurationDialog::onChunkSizeChanged);
    connect(durationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConfigurationDialog::onDurationChanged);
    connect(chunkSizeCombo, &QComboBox::currentTextChanged, this, &ConfigurationDialog::updateTimingDiagram);
    connect(customLinesEdit, &QLineEdit::textChanged, this, &ConfigurationDialog::updateTimingDiagram);
    connect(durationCombo, &QComboBox::currentTextChanged, this, &ConfigurationDialog::updateTimingDiagram);
    connect(customDurationEdit, &QLineEdit::textChanged, this, &ConfigurationDialog::updateTimingDiagram);
    connect(idleDelayCombo, &QComboBox::currentTextChanged, this, &ConfigurationDialog::updateTimingDiagram);
    connect(performanceModeCombo, &QComboBox::currentTextChanged, this, &ConfigurationDialog::updateTimingDiagram);
}

void ConfigurationDialog::setupTab4_HighlightEngines()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    
    // Highlight Engines Group
    QGroupBox *enginesGroup = new QGroupBox("Highlight Engines", tab);
    QVBoxLayout *enginesLayout = new QVBoxLayout(enginesGroup);
    
    useRGHighlightCheck = new QCheckBox("Use RG Highlight", enginesGroup);
    useRGHighlightCheck->setToolTip("Use ripgrep for highlighting (faster for large files)");
    
    useQtHighlightCheck = new QCheckBox("Use Qt Highlight", enginesGroup);
    useQtHighlightCheck->setToolTip("Use Qt's regex engine for highlighting");
    
    useViewportHighlightCheck = new QCheckBox("Use Viewport Highlight", enginesGroup);
    useViewportHighlightCheck->setToolTip("Highlight only visible portion of file");
    
    enginesLayout->addWidget(useRGHighlightCheck);
    enginesLayout->addWidget(useQtHighlightCheck);
    enginesLayout->addWidget(useViewportHighlightCheck);
    
    layout->addWidget(enginesGroup);
    layout->addStretch();
    
    tabWidget->addTab(tab, "🎨 Highlight Engines");
}

void ConfigurationDialog::setupTab5_CustomColors()
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    
    QGroupBox *colorsGroup = new QGroupBox("GUI Component Colors", tab);
    QVBoxLayout *colorsLayout = new QVBoxLayout(colorsGroup);
    
    // Search Results Color
    QVBoxLayout *searchResultsLayout = new QVBoxLayout();
    QLabel *searchResultsLabel = new QLabel("Search Results:", colorsGroup);
    searchResultsColorButton = new QPushButton("Choose Color", colorsGroup);
    searchResultsColorPreview = new QLabel(colorsGroup);
    searchResultsColorPreview->setFixedSize(24, 24);
    searchResultsColorPreview->setStyleSheet("QLabel { border: 1px solid black; background-color: white; }");
    
    searchResultsLayout->addWidget(searchResultsLabel);
    searchResultsLayout->addWidget(searchResultsColorButton);
    searchResultsLayout->addWidget(searchResultsColorPreview);
    
    // File Content Color
    QVBoxLayout *fileContentLayout = new QVBoxLayout();
    QLabel *fileContentLabel = new QLabel("File Content:", colorsGroup);
    fileContentColorButton = new QPushButton("Choose Color", colorsGroup);
    fileContentColorPreview = new QLabel(colorsGroup);
    fileContentColorPreview->setFixedSize(24, 24);
    fileContentColorPreview->setStyleSheet("QLabel { border: 1px solid black; background-color: white; }");
    
    fileContentLayout->addWidget(fileContentLabel);
    fileContentLayout->addWidget(fileContentColorButton);
    fileContentLayout->addWidget(fileContentColorPreview);
    
    // Log Window Color
    QVBoxLayout *logWindowLayout = new QVBoxLayout();
    QLabel *logWindowLabel = new QLabel("Log Window:", colorsGroup);
    logWindowColorButton = new QPushButton("Choose Color", colorsGroup);
    logWindowColorPreview = new QLabel(colorsGroup);
    logWindowColorPreview->setFixedSize(24, 24);
    logWindowColorPreview->setStyleSheet("QLabel { border: 1px solid black; background-color: white; }");
    
    logWindowLayout->addWidget(logWindowLabel);
    logWindowLayout->addWidget(logWindowColorButton);
    logWindowLayout->addWidget(logWindowColorPreview);
    
    // Status Bar Color
    QVBoxLayout *statusBarLayout = new QVBoxLayout();
    QLabel *statusBarLabel = new QLabel("Status Bar:", colorsGroup);
    statusBarColorButton = new QPushButton("Choose Color", colorsGroup);
    statusBarColorPreview = new QLabel(colorsGroup);
    statusBarColorPreview->setFixedSize(24, 24);
    statusBarColorPreview->setStyleSheet("QLabel { border: 1px solid black; background-color: white; }");
    
    statusBarLayout->addWidget(statusBarLabel);
    statusBarLayout->addWidget(statusBarColorButton);
    statusBarLayout->addWidget(statusBarColorPreview);
    
    // Create horizontal layout for color sections
    QHBoxLayout *colorsHorizontalLayout = new QHBoxLayout();
    colorsHorizontalLayout->addLayout(searchResultsLayout);
    colorsHorizontalLayout->addLayout(fileContentLayout);
    colorsHorizontalLayout->addLayout(logWindowLayout);
    colorsHorizontalLayout->addLayout(statusBarLayout);
    
    colorsLayout->addLayout(colorsHorizontalLayout);
    
    layout->addWidget(colorsGroup);
    layout->addStretch();
    
    tabWidget->addTab(tab, "🎨 Custom Colors");
    
    // Connect color buttons
    connect(searchResultsColorButton, &QPushButton::clicked, this, &ConfigurationDialog::onColorButtonClicked);
    connect(fileContentColorButton, &QPushButton::clicked, this, &ConfigurationDialog::onColorButtonClicked);
    connect(logWindowColorButton, &QPushButton::clicked, this, &ConfigurationDialog::onColorButtonClicked);
    connect(statusBarColorButton, &QPushButton::clicked, this, &ConfigurationDialog::onColorButtonClicked);
}

void ConfigurationDialog::loadSettings()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = appDir + "/App.ini";
    QSettings settings(configFile, QSettings::IniFormat);
    settings.beginGroup("Configuration");
    
    // Font settings
    fontSizeEdit->setText(settings.value("SearchResultsFontSize", "12").toString());
    previewFontSizeEdit->setText(settings.value("FileContentFontSize", "10").toString());
    
    // Background highlighting settings
    backgroundHighlightingCheck->setChecked(settings.value("BackgroundHighlighting", false).toBool());
    chunkModeCombo->setCurrentText(settings.value("ChunkMode", "Lines").toString());
    chunkSizeCombo->setCurrentText(settings.value("ChunkSize", "1000 lines").toString());
    customLinesEdit->setText(settings.value("CustomLines", "1000").toString());
    durationCombo->setCurrentText(settings.value("Duration", "50ms").toString());
    customDurationEdit->setText(settings.value("CustomDuration", "50").toString());
    idleDelayCombo->setCurrentText(settings.value("IdleDelay", "1 second").toString());
    performanceModeCombo->setCurrentText(settings.value("PerformanceMode", "Balanced").toString());
    progressIndicatorCheck->setChecked(settings.value("ProgressIndicator", true).toBool());
    
    // Highlight engines settings
    useRGHighlightCheck->setChecked(settings.value("UseRGHighlight", true).toBool());
    useQtHighlightCheck->setChecked(settings.value("UseQtHighlight", false).toBool());
    useViewportHighlightCheck->setChecked(settings.value("UseViewportHighlight", true).toBool());
    
    // Layout settings
    enableLogWindowCheck->setChecked(settings.value("EnableLogWindow", true).toBool());
    bool isUpDownLayout = settings.value("UpDownLayout", true).toBool();
    upDownLayoutRadio->setChecked(isUpDownLayout);
    sideBySideLayoutRadio->setChecked(!isUpDownLayout);
    
    // Custom colors
    QColor searchResultsColor = settings.value("SearchResultsColor", QColor(255, 255, 255)).value<QColor>();
    QColor fileContentColor = settings.value("FileContentColor", QColor(255, 255, 255)).value<QColor>();
    QColor logWindowColor = settings.value("LogWindowColor", QColor(248, 249, 250)).value<QColor>();
    QColor statusBarColor = settings.value("StatusBarColor", QColor(255, 255, 255)).value<QColor>();
    
    searchResultsColorPreview->setStyleSheet(QString("QLabel { border: 1px solid black; background-color: %1; }").arg(searchResultsColor.name()));
    fileContentColorPreview->setStyleSheet(QString("QLabel { border: 1px solid black; background-color: %1; }").arg(fileContentColor.name()));
    logWindowColorPreview->setStyleSheet(QString("QLabel { border: 1px solid black; background-color: %1; }").arg(logWindowColor.name()));
    statusBarColorPreview->setStyleSheet(QString("QLabel { border: 1px solid black; background-color: %1; }").arg(statusBarColor.name()));
    
    settings.endGroup();
    
    // Update UI based on loaded settings
    onBackgroundHighlightingChanged();
    onChunkModeChanged();
    onChunkSizeChanged();
    onDurationChanged();
    updateTimingDiagram();
    updateLayoutPreview();
}

void ConfigurationDialog::saveSettings()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = appDir + "/App.ini";
    QSettings settings(configFile, QSettings::IniFormat);
    settings.beginGroup("Configuration");
    
    // Font settings
    settings.setValue("SearchResultsFontSize", fontSizeEdit->text());
    settings.setValue("FileContentFontSize", previewFontSizeEdit->text());
    
    // Background highlighting settings
    settings.setValue("BackgroundHighlighting", backgroundHighlightingCheck->isChecked());
    settings.setValue("ChunkMode", chunkModeCombo->currentText());
    settings.setValue("ChunkSize", chunkSizeCombo->currentText());
    settings.setValue("CustomLines", customLinesEdit->text());
    settings.setValue("Duration", durationCombo->currentText());
    settings.setValue("CustomDuration", customDurationEdit->text());
    settings.setValue("IdleDelay", idleDelayCombo->currentText());
    settings.setValue("PerformanceMode", performanceModeCombo->currentText());
    settings.setValue("ProgressIndicator", progressIndicatorCheck->isChecked());
    
    // Highlight engines settings
    settings.setValue("UseRGHighlight", useRGHighlightCheck->isChecked());
    settings.setValue("UseQtHighlight", useQtHighlightCheck->isChecked());
    settings.setValue("UseViewportHighlight", useViewportHighlightCheck->isChecked());
    
    // Layout settings
    settings.setValue("EnableLogWindow", enableLogWindowCheck->isChecked());
    settings.setValue("UpDownLayout", upDownLayoutRadio->isChecked());
    
    // Custom colors
    QColor searchResultsColor = searchResultsColorPreview->palette().color(QPalette::Window);
    QColor fileContentColor = fileContentColorPreview->palette().color(QPalette::Window);
    QColor logWindowColor = logWindowColorPreview->palette().color(QPalette::Window);
    QColor statusBarColor = statusBarColorPreview->palette().color(QPalette::Window);
    
    settings.setValue("SearchResultsColor", searchResultsColor);
    settings.setValue("FileContentColor", fileContentColor);
    settings.setValue("LogWindowColor", logWindowColor);
    settings.setValue("StatusBarColor", statusBarColor);
    
    settings.endGroup();
}





void ConfigurationDialog::updateTimingDiagram()
{
    QString diagram;
    
    if (chunkModeCombo->currentText() == "Lines") {
        QString chunkSize = chunkSizeCombo->currentText();
        if (chunkSize == "Custom lines") {
            chunkSize = customLinesEdit->text() + " lines";
        }
        
        // Calculate chunks based on chunk size
        int chunkSizeValue = 1000; // default
        if (chunkSize.contains("100 lines")) chunkSizeValue = 100;
        else if (chunkSize.contains("500 lines")) chunkSizeValue = 500;
        else if (chunkSize.contains("1000 lines")) chunkSizeValue = 1000;
        else if (chunkSize.contains("2000 lines")) chunkSizeValue = 2000;
        else if (chunkSize.contains("5000 lines")) chunkSizeValue = 5000;
        else if (chunkSize.contains("10000 lines")) chunkSizeValue = 10000;
        else if (chunkSize.contains("20000 lines")) chunkSizeValue = 20000;
        else if (chunkSize.contains("50000 lines")) chunkSizeValue = 50000;
        else if (chunkSize.contains("100000 lines")) chunkSizeValue = 100000;
        else if (chunkSize.contains("Custom lines")) {
            bool ok;
            chunkSizeValue = customLinesEdit->text().toInt(&ok);
            if (!ok || chunkSizeValue < 100) chunkSizeValue = 1000;
        }
        
        int totalLines = 100000;
        int totalChunks = (totalLines + chunkSizeValue - 1) / chunkSizeValue; // Ceiling division
        
        diagram = QString(
            "Lines Mode:\n"
            "┌─────────────────────────────────────────┐\n"
            "│ File: %1 lines                          │\n"
            "│ Chunk: %2                               │\n"
            "│ Idle Delay: %3                          │\n"
            "│ Performance: %4                         │\n"
            "│                                         │\n"
            "│ Processing: [████████████████████] 100%% │\n"
            "│ Chunks: %5 chunks processed             │\n"
            "└─────────────────────────────────────────┘"
        ).arg(QString::number(totalLines), chunkSize, idleDelayCombo->currentText(), 
              performanceModeCombo->currentText(), QString::number(totalChunks));
    } else {
        QString duration = durationCombo->currentText();
        if (duration == "Custom ms") {
            duration = customDurationEdit->text() + "ms";
        }
        
        // Calculate estimated time based on duration
        int durationMs = 50; // default
        if (duration.contains("10ms")) durationMs = 10;
        else if (duration.contains("50ms")) durationMs = 50;
        else if (duration.contains("100ms")) durationMs = 100;
        else if (duration.contains("500ms")) durationMs = 500;
        else if (duration.contains("Custom ms")) {
            bool ok;
            durationMs = customDurationEdit->text().toInt(&ok);
            if (!ok || durationMs < 1) durationMs = 50;
        }
        
        // Estimate total time (assuming 1000 chunks for 100k lines)
        int estimatedChunks = 1000;
        double totalTimeSeconds = (durationMs * estimatedChunks) / 1000.0;
        
        diagram = QString(
            "Duration Mode:\n"
            "┌─────────────────────────────────────────┐\n"
            "│ File: 100,000 lines                     │\n"
            "│ Duration: %1 per chunk                  │\n"
            "│ Idle Delay: %2                          │\n"
            "│ Performance: %3                         │\n"
            "│                                         │\n"
            "│ Processing: [████████████████████] 100%% │\n"
            "│ Time: ~%4 seconds total                 │\n"
            "└─────────────────────────────────────────┘"
        ).arg(duration, idleDelayCombo->currentText(), performanceModeCombo->currentText(), 
              QString::number(totalTimeSeconds, 'f', 1));
    }
    
    timingDiagram->setText(diagram);
}

void ConfigurationDialog::updateLayoutPreview()
{
    if (!m_layoutPreviewLabel) return;
    
    QString preview;
    
    if (upDownLayoutRadio->isChecked()) {
        // Up/Down Layout (Vertical)
        preview = QString(
            "Up/Down Layout (Current):\n\n"
            "┌─────────────────────────────────┐\n"
            "│     🔍 Search Controls          │\n"
            "├─────────────────────────────────┤\n"
            "│                                 │\n"
            "│    📋 Search Results Pane       │\n"
            "│                                 │\n"
            "├─────────────────────────────────┤\n"
            "│                                 │\n"
            "│    📄 File Viewer Pane          │\n"
            "│                                 │\n"
            "├─────────────────────────────────┤%1"
            "└─────────────────────────────────┘"
        ).arg(enableLogWindowCheck->isChecked() ? 
              "\n│    📝 Log Window                │\n" : "\n");
    } else {
        // Side by Side Layout (Horizontal)
        preview = QString(
            "Side by Side Layout:\n\n"
            "┌─────────────────────────────────┐\n"
            "│     🔍 Search Controls          │\n"
            "├─────────────┬───────────────────┤\n"
            "│             │                   │\n"
            "│ 📋 Search   │  📄 File Viewer   │\n"
            "│  Results    │     Pane          │\n"
            "│   Pane      │                   │\n"
            "│             │                   │\n"
            "├─────────────┴───────────────────┤%1"
            "└─────────────────────────────────┘"
        ).arg(enableLogWindowCheck->isChecked() ? 
              "\n│        📝 Log Window            │\n" : "\n");
    }
    
    m_layoutPreviewLabel->setText(preview);
}

// Slot implementations
void ConfigurationDialog::onDoneClicked()
{
    saveSettings();
    
    // Apply background highlighting settings immediately
    applyBackgroundHighlightingSettings();
    
    // Apply layout settings immediately
    applyLayoutSettings();
    
    accept();
}

void ConfigurationDialog::onCancelClicked()
{
    reject();
}

void ConfigurationDialog::onClearHistoryClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "Clear History", 
        "Are you sure you want to clear all search history?\n\nThis will delete all saved patterns and paths.\nThis action cannot be undone.",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        if (m_mainWindow) {
            // Clear pattern and path history
            m_mainWindow->clearAllSearchHistory();
            
            QMessageBox::information(
                this,
                "History Cleared",
                "All search history has been cleared successfully."
            );
        }
    }
}

void ConfigurationDialog::onFontButtonClicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, this);
    if (ok) {
        fontPreviewLabel->setFont(font);
        fontPreviewLabel->setText("Sample Text - Search Results (" + font.family() + ")");
    }
}

void ConfigurationDialog::onPreviewFontButtonClicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, this);
    if (ok) {
        previewFontPreviewLabel->setFont(font);
        previewFontPreviewLabel->setText("Sample Text - File Content (" + font.family() + ")");
    }
}

void ConfigurationDialog::onColorButtonClicked()
{
    QPushButton *senderButton = qobject_cast<QPushButton*>(sender());
    if (!senderButton) return;
    
    QColor color = QColorDialog::getColor(Qt::white, this, "Choose Color");
    if (color.isValid()) {
        if (senderButton == searchResultsColorButton) {
            searchResultsColorPreview->setStyleSheet(QString("QLabel { border: 1px solid black; background-color: %1; }").arg(color.name()));
        } else if (senderButton == fileContentColorButton) {
            fileContentColorPreview->setStyleSheet(QString("QLabel { border: 1px solid black; background-color: %1; }").arg(color.name()));
        } else if (senderButton == logWindowColorButton) {
            logWindowColorPreview->setStyleSheet(QString("QLabel { border: 1px solid black; background-color: %1; }").arg(color.name()));
        } else if (senderButton == statusBarColorButton) {
            statusBarColorPreview->setStyleSheet(QString("QLabel { border: 1px solid black; background-color: %1; }").arg(color.name()));
        }
    }
}

void ConfigurationDialog::onBackgroundHighlightingChanged()
{
    bool enabled = backgroundHighlightingCheck->isChecked();
    chunkModeCombo->setEnabled(enabled);
    chunkSizeCombo->setEnabled(enabled);
    customLinesEdit->setEnabled(enabled);
    durationCombo->setEnabled(enabled);
    customDurationEdit->setEnabled(enabled);
    idleDelayCombo->setEnabled(enabled);
    performanceModeCombo->setEnabled(enabled);
    progressIndicatorCheck->setEnabled(enabled);
}

void ConfigurationDialog::onChunkModeChanged()
{
    bool isLinesMode = chunkModeCombo->currentText() == "Lines";
    chunkSizeCombo->setVisible(isLinesMode);
    customLinesEdit->setVisible(isLinesMode && chunkSizeCombo->currentText() == "Custom lines");
    durationCombo->setVisible(!isLinesMode);
    customDurationEdit->setVisible(!isLinesMode && durationCombo->currentText() == "Custom ms");
}

void ConfigurationDialog::onChunkSizeChanged()
{
    customLinesEdit->setVisible(chunkSizeCombo->currentText() == "Custom lines");
}

void ConfigurationDialog::onDurationChanged()
{
    customDurationEdit->setVisible(durationCombo->currentText() == "Custom ms");
}

void ConfigurationDialog::onLayoutSettingsChanged()
{
    updateLayoutPreview();
}

// Getter methods for highlight engines
bool ConfigurationDialog::isUseRGHighlight() const
{
    return useRGHighlightCheck->isChecked();
}

bool ConfigurationDialog::isUseQtHighlight() const
{
    return useQtHighlightCheck->isChecked();
}

bool ConfigurationDialog::isUseViewportHighlight() const
{
    return useViewportHighlightCheck->isChecked();
}

void ConfigurationDialog::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

void ConfigurationDialog::applyBackgroundHighlightingSettings()
{
    if (!m_mainWindow) {
        qDebug() << "ConfigurationDialog: No MainWindow reference available";
        return;
    }
    
    // Get current background highlighting settings
    bool backgroundEnabled = backgroundHighlightingCheck->isChecked();
    bool progressEnabled = progressIndicatorCheck->isChecked();
    
    qDebug() << "ConfigurationDialog: Applying background highlighting settings - Enabled:" << backgroundEnabled << "Progress:" << progressEnabled;
    
    // Call MainWindow method to apply settings immediately
    // We'll need to add this method to MainWindow
    m_mainWindow->applyBackgroundHighlightingSettingsFromConfig(backgroundEnabled, progressEnabled);
}

void ConfigurationDialog::applyLayoutSettings()
{
    if (!m_mainWindow) {
        qDebug() << "ConfigurationDialog: No MainWindow reference available for layout settings";
        return;
    }
    
    // Get current layout settings
    bool enableLogWindow = enableLogWindowCheck->isChecked();
    bool isUpDownLayout = upDownLayoutRadio->isChecked();
    
    qDebug() << "ConfigurationDialog: Applying layout settings - Log Window:" << enableLogWindow << "Up/Down Layout:" << isUpDownLayout;
    
    // Call MainWindow method to apply layout settings immediately
    m_mainWindow->applyLayoutSettingsFromConfig(enableLogWindow, isUpDownLayout);
}
