#include "rgsearchdialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QDateTime>
#include <QSettings>
#include <QStandardPaths>
#include <QPainter>
#include "logger.h"

RGSearchDialog::RGSearchDialog(QWidget *parent)
    : QDialog(parent)
    , kSearchBun(new KSearchBun(this))
    , highlightColor(QColor(130, 130, 130))  // Initialize with default grey color
{
    setupUI();
    setupToolTips();
    loadSettings();
    loadHistory();
    
    // Connect signals
    // Browse button connection removed - browse functionality now in main window
    connect(searchButton, &QPushButton::clicked, this, &RGSearchDialog::onSearchClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(colorButton, &QPushButton::clicked, this, &RGSearchDialog::onColorButtonClicked);
    connect(defaultColorButton, &QPushButton::clicked, this, &RGSearchDialog::onDefaultColorButtonClicked);
    connect(saveParamsButton, &QPushButton::clicked, this, &RGSearchDialog::onSaveParamsClicked);
    connect(loadParamsButton, &QPushButton::clicked, this, &RGSearchDialog::onLoadParamsClicked);
    
    // Connect case sensitivity checkboxes for mutual exclusivity
    connect(caseSensitiveCheckBox, &QCheckBox::toggled, this, &RGSearchDialog::onCaseSensitiveToggled);
    connect(ignoreCaseCheckBox, &QCheckBox::toggled, this, &RGSearchDialog::onIgnoreCaseToggled);
    connect(smartCaseCheckBox, &QCheckBox::toggled, this, &RGSearchDialog::onSmartCaseToggled);
}

void RGSearchDialog::setupUI()
{
    setWindowTitle("RG Search");
    setModal(true);
    resize(500, 400);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Pattern and Path fields removed - now in main search controls
    
    // Fixed string checkbox
    QHBoxLayout *fixedStringLayout = new QHBoxLayout();
    fixedStringCheckBox = new QCheckBox("Fixed String");
    fixedStringCheckBox->setToolTip("Treat pattern as literal text instead of regex\n\nUseful when searching for:\n- Special characters like . * + ? [ ] ( ) { } | \\\n- Exact strings without regex interpretation\n- Code snippets with punctuation");
    fixedStringLayout->addWidget(fixedStringCheckBox);
    fixedStringLayout->addStretch();
    mainLayout->addLayout(fixedStringLayout);
    
    // Additional pattern
    QHBoxLayout *addPatternLayout = new QHBoxLayout();
    QLabel *addPatternLabel = new QLabel("Additional Pattern:");
    addPatternEdit = new QLineEdit();
    addPatternEdit->setToolTip("Additional regex patterns separated by | (OR operator)\n\nExamples:\n- error|warning|fail\n- TODO|FIXME|HACK\n- \\berror\\b|\\bfail\\b\n\nCombines with main pattern using AND logic");
    addPatternLayout->addWidget(addPatternLabel);
    addPatternLayout->addWidget(addPatternEdit);
    mainLayout->addLayout(addPatternLayout);
    
    // Case sensitivity options
    QGridLayout *caseLayout = new QGridLayout();
    caseSensitiveCheckBox = new QCheckBox("Case Sensitive");
    caseSensitiveCheckBox->setToolTip("Match exact case\n\nExample:\n- 'Error' will match 'Error' but not 'error' or 'ERROR'");
    ignoreCaseCheckBox = new QCheckBox("Ignore Case");
    ignoreCaseCheckBox->setToolTip("Ignore case differences\n\nExample:\n- 'error' will match 'Error', 'ERROR', 'error'");
    smartCaseCheckBox = new QCheckBox("Smart Case");
    smartCaseCheckBox->setToolTip("Case sensitive if pattern contains uppercase, otherwise case insensitive\n\nExamples:\n- 'error' → case insensitive\n- 'Error' → case sensitive");
    caseLayout->addWidget(caseSensitiveCheckBox, 0, 0);
    caseLayout->addWidget(ignoreCaseCheckBox, 0, 1);
    caseLayout->addWidget(smartCaseCheckBox, 0, 2);
    mainLayout->addLayout(caseLayout);
    
    // Include/Exclude patterns
    QHBoxLayout *inclExcludeLayout = new QHBoxLayout();
    QLabel *inclExcludeLabel = new QLabel("Include/Exclude:");
    inclExcludeEdit = new QLineEdit();
    inclExcludeEdit->setToolTip("File patterns to include/exclude (comma-separated)\n\nExamples:\n- *.cpp,*.h (include only C++ files)\n- *.txt,!*.tmp (include .txt but exclude .tmp)\n- !*.log,!build/* (exclude log files and build directory)\n\nUse ! prefix to exclude patterns");
    inclExcludeLayout->addWidget(inclExcludeLabel);
    inclExcludeLayout->addWidget(inclExcludeEdit);
    mainLayout->addLayout(inclExcludeLayout);
    
    // Keep files in cache checkbox
    QHBoxLayout *cacheLayout = new QHBoxLayout();
    keepFilesInCacheCheckBox = new QCheckBox("Keep searched files in cache");
    keepFilesInCacheCheckBox->setToolTip("Keep found files loaded in memory for faster access\n\nBenefits:\n- Faster file opening and navigation\n- Reduced disk I/O for subsequent operations\n- Better performance for large search results\n\nNote: Uses more memory");
    cacheLayout->addWidget(keepFilesInCacheCheckBox);
    cacheLayout->addStretch();
    mainLayout->addLayout(cacheLayout);
    
    // Highlight color picker
    QHBoxLayout *colorLayout = new QHBoxLayout();
    QLabel *colorLabel = new QLabel("Highlight Color:");
    colorButton = new QPushButton("Choose Color", this);
    colorButton->setToolTip("Choose color for highlighting search results in files\n\nTip: Use contrasting colors for better visibility");
    defaultColorButton = new QPushButton("Default Color", this);
    defaultColorButton->setToolTip("Reset to default grey color (RGB: 130, 130, 130)");
    colorPreviewLabel = new QLabel(this);
    colorPreviewLabel->setFixedSize(40, 25);
    colorPreviewLabel->setStyleSheet(QString("background-color: %1; border: 2px solid black; border-radius: 4px;").arg(highlightColor.name()));
    colorPreviewLabel->setToolTip("Preview of current highlight color");
    
    colorLayout->addWidget(colorLabel);
    colorLayout->addWidget(colorButton);
    colorLayout->addWidget(defaultColorButton);
    colorLayout->addWidget(colorPreviewLabel);
    colorLayout->addStretch();
    
    mainLayout->addLayout(colorLayout);
    
    // Save/Load Parameters buttons (left side)
    QHBoxLayout *saveLoadLayout = new QHBoxLayout();
    
    saveParamsButton = new QPushButton("💾 Save Parameters");
    saveParamsButton->setToolTip("Save all search parameters to a .params file\n\nSaves:\n- All checkbox states\n- Text field values\n- Highlight color\n- Case sensitivity settings\n- Include/exclude patterns");
    saveParamsButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #17a2b8, stop:1 #138496);"
        "    border: none;"
        "    color: white;"
        "    padding: 8px 16px;"
        "    font-size: 12px;"
        "    font-weight: 600;"
        "    border-radius: 6px;"
        "    min-width: 120px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #20c997, stop:1 #17a2b8);"
        "    transform: translateY(-1px);"
        "    box-shadow: 0 2px 4px rgba(0,0,0,0.2);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #138496, stop:1 #0f6674);"
        "    transform: translateY(1px);"
        "    box-shadow: 0 1px 2px rgba(0,0,0,0.2);"
        "}"
    );
    
    loadParamsButton = new QPushButton("📂 Load Parameters");
    loadParamsButton->setToolTip("Load search parameters from a .params file\n\nLoads:\n- All checkbox states\n- Text field values\n- Highlight color\n- Case sensitivity settings\n- Include/exclude patterns");
    loadParamsButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6f42c1, stop:1 #5a32a3);"
        "    border: none;"
        "    color: white;"
        "    padding: 8px 16px;"
        "    font-size: 12px;"
        "    font-weight: 600;"
        "    border-radius: 6px;"
        "    min-width: 120px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #8250df, stop:1 #6f42c1);"
        "    transform: translateY(-1px);"
        "    box-shadow: 0 2px 4px rgba(0,0,0,0.2);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5a32a3, stop:1 #4a2b8a);"
        "    transform: translateY(1px);"
        "    box-shadow: 0 1px 2px rgba(0,0,0,0.2);"
        "}"
    );
    
    saveLoadLayout->addWidget(saveParamsButton);
    saveLoadLayout->addWidget(loadParamsButton);
    saveLoadLayout->addStretch();
    mainLayout->addLayout(saveLoadLayout);
    
    // Apply/Cancel buttons (right side)
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    searchButton = new QPushButton("Apply");
    searchButton->setToolTip("Apply settings and close dialog\n\nShortcut: Enter key in pattern field");
    // Create check mark icon using Unicode character
    QPixmap checkPixmap(16, 16);
    checkPixmap.fill(Qt::transparent);
    QPainter painter(&checkPixmap);
    painter.setPen(QPen(Qt::white, 2));
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(checkPixmap.rect(), Qt::AlignCenter, "✓");
    searchButton->setIcon(QIcon(checkPixmap));
    searchButton->setStyleSheet(
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
    
    cancelButton = new QPushButton("Cancel");
    cancelButton->setToolTip("Close dialog without searching");
    cancelButton->setIcon(QIcon(":/icons/close_window.png"));
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
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(searchButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);
    
    // Set default values
    // Pattern edit placeholder removed - pattern field now in main window
    addPatternEdit->setPlaceholderText("Additional regex patterns separated by |");
    inclExcludeEdit->setPlaceholderText("*.txt,*.log,!*.tmp");
}

void RGSearchDialog::onColorButtonClicked()
{
    QColor newColor = QColorDialog::getColor(highlightColor, this, "Choose Highlight Color");
    if (newColor.isValid()) {
        highlightColor = newColor;
        colorPreviewLabel->setStyleSheet(QString("background-color: %1; border: 2px solid black; border-radius: 4px;").arg(highlightColor.name()));
        LOG_INFO("RGSearchDialog: Color changed to: " + highlightColor.name() + 
                 " (RGB: " + QString::number(highlightColor.rgb()) + 
                 " R:" + QString::number(highlightColor.red()) + 
                 " G:" + QString::number(highlightColor.green()) + 
                 " B:" + QString::number(highlightColor.blue()) + ")");
    }
}

void RGSearchDialog::onDefaultColorButtonClicked()
{
    highlightColor = QColor(130, 130, 130);  // Default grey color
    colorPreviewLabel->setStyleSheet(QString("background-color: %1; border: 2px solid black; border-radius: 4px;").arg(highlightColor.name()));
    LOG_INFO("RGSearchDialog: Color reset to default grey: " + highlightColor.name() + 
             " (RGB: " + QString::number(highlightColor.rgb()) + 
             " R:" + QString::number(highlightColor.red()) + 
             " G:" + QString::number(highlightColor.green()) + 
             " B:" + QString::number(highlightColor.blue()) + ")");
}

void RGSearchDialog::setupToolTips()
{
    // Pattern edit tooltip removed - pattern field now in main window
    fixedStringCheckBox->setToolTip("The patterns are treated as literal text");
    addPatternEdit->setToolTip("Additional pattern to find, separated by logical or \"|\"\nThis is a regex line with several expressions delimited by | (logic or)");
    caseSensitiveCheckBox->setToolTip("Case sensitive search");
    ignoreCaseCheckBox->setToolTip("Ignore case in search");
    smartCaseCheckBox->setToolTip("Smart case search");
    inclExcludeEdit->setToolTip("Will include all files from type ext if add flag: -g \"*.ext\"\n"
                                "Will exclude all files from type ext if add flag: -g \"!*.ext\"\n"
                                "If the text has comma between such as text=\"*.ext,*.drv\" then it will create 2 flags -g \"*.ext\" -g \"*.drv\"");
    keepFilesInCacheCheckBox->setToolTip("When checked, files will be kept in memory cache for faster navigation.\n"
                                         "When unchecked, only one file is kept in memory at a time.");
    colorButton->setToolTip("Choose the color to highlight matching text in the file\n\nSelected color will be used to highlight all matching patterns in the file viewport");
}

// browsePath method removed - browse functionality moved to main window

RGSearchParams RGSearchDialog::getSearchParams() const
{
    RGSearchParams params;
    params.path = m_currentPath;  // Use stored values from main window
    params.pattern = m_currentPattern;
    params.fixed_string = fixedStringCheckBox->isChecked();
    params.add_pattern = addPatternEdit->text();
    params.case_sensitive = caseSensitiveCheckBox->isChecked();
    params.ignore_case = ignoreCaseCheckBox->isChecked();
    params.smart_case = smartCaseCheckBox->isChecked();
    params.incl_exclude = inclExcludeEdit->text();
    params.keep_files_in_cache = keepFilesInCacheCheckBox->isChecked();
    params.highlight_color = highlightColor;
    LOG_INFO("RGSearchDialog: getSearchParams returning highlight color: " + params.highlight_color.name());
    return params;
}

void RGSearchDialog::onSearchClicked()
{
    QElapsedTimer timer;
    timer.start();
    
    LOG_INFO("RGSearchDialog: Starting search at " + QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
    
    // Save settings and history before search
    saveSettings();
    saveHistory();
    
    // Build search parameters
    RGSearchParams params;
    params.path = m_currentPath;
    params.pattern = m_currentPattern;
    params.fixed_string = fixedStringCheckBox->isChecked();
    params.add_pattern = addPatternEdit->text();
    params.case_sensitive = caseSensitiveCheckBox->isChecked();
    params.ignore_case = ignoreCaseCheckBox->isChecked();
    params.smart_case = smartCaseCheckBox->isChecked();
    params.incl_exclude = inclExcludeEdit->text();
    params.keep_files_in_cache = keepFilesInCacheCheckBox->isChecked();
    params.highlight_color = highlightColor;
    
    // Log search parameters
    LOG_INFO("RGSearchDialog: Search parameters - Path: " + params.path + ", Pattern: " + params.pattern);
    
   // Accept the dialog to return QDialog::Accepted
    LOG_INFO("RGSearchDialog: Accepting dialog after search started");
    this->accept();
}

void RGSearchDialog::setLogWidget(QTextEdit *logWidget)
{
    m_logWidget = logWidget;
}

void RGSearchDialog::loadSettings()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = appDir + "/App.ini";
    QSettings settings(configFile, QSettings::IniFormat);
    settings.beginGroup("RGSearch");
    
    // Path and pattern loading removed - these are now managed by main window
    fixedStringCheckBox->setChecked(settings.value("LastFixedString", false).toBool());
    addPatternEdit->setText(settings.value("LastAddPattern", "").toString());
    caseSensitiveCheckBox->setChecked(settings.value("LastCaseSensitive", false).toBool());
    ignoreCaseCheckBox->setChecked(settings.value("LastIgnoreCase", false).toBool());
    smartCaseCheckBox->setChecked(settings.value("LastSmartCase", false).toBool());
    inclExcludeEdit->setText(settings.value("LastInclExclude", "").toString());
    keepFilesInCacheCheckBox->setChecked(settings.value("LastKeepFilesInCache", false).toBool());
    highlightColor = settings.value("LastHighlightColor", QColor(130, 130, 130)).value<QColor>();
    
    // Load the last save/load path preference
    m_lastSaveLoadPath = settings.value("LastSaveLoadPath", "").toString();
    
    settings.endGroup();
    
    // Update the color preview label to reflect the loaded color
    if (colorPreviewLabel) {
        colorPreviewLabel->setStyleSheet(QString("background-color: %1; border: 2px solid black; border-radius: 4px;").arg(highlightColor.name()));
        LOG_INFO("RGSearchDialog: Loaded highlight color: " + highlightColor.name() + " and updated preview label");
    }
}

void RGSearchDialog::saveSettings()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = appDir + "/App.ini";
    QSettings settings(configFile, QSettings::IniFormat);
    settings.beginGroup("RGSearch");
    
    // Path and pattern saving removed - these are now managed by main window
    settings.setValue("LastFixedString", fixedStringCheckBox->isChecked());
    settings.setValue("LastAddPattern", addPatternEdit->text());
    settings.setValue("LastCaseSensitive", caseSensitiveCheckBox->isChecked());
    settings.setValue("LastIgnoreCase", ignoreCaseCheckBox->isChecked());
    settings.setValue("LastSmartCase", smartCaseCheckBox->isChecked());
    settings.setValue("LastInclExclude", inclExcludeEdit->text());
    settings.setValue("LastKeepFilesInCache", keepFilesInCacheCheckBox->isChecked());
    settings.setValue("LastHighlightColor", highlightColor);
    
    // Save the last save/load path preference
    settings.setValue("LastSaveLoadPath", m_lastSaveLoadPath);
    
    settings.endGroup();
}

void RGSearchDialog::loadHistory()
{
    QSettings settings("app.ini", QSettings::IniFormat);
    settings.beginGroup("RGSearch");
    
    // Load history for combo boxes (if we had them)
    // For now, we'll just load the last values as defaults
    
    settings.endGroup();
}

void RGSearchDialog::saveHistory()
{
    QSettings settings("app.ini", QSettings::IniFormat);
    settings.beginGroup("RGSearch");
    
    // Save current values to history
    QStringList pathHistory = settings.value("PathHistory", QStringList()).toStringList();
    QStringList patternHistory = settings.value("PatternHistory", QStringList()).toStringList();
    QStringList addPatternHistory = settings.value("AddPatternHistory", QStringList()).toStringList();
    QStringList inclExcludeHistory = settings.value("InclExcludeHistory", QStringList()).toStringList();
    
    // Path and pattern history saving removed - these are now managed by main window
    
    if (!addPatternEdit->text().isEmpty() && !addPatternHistory.contains(addPatternEdit->text())) {
        addPatternHistory.prepend(addPatternEdit->text());
        if (addPatternHistory.size() > 10) addPatternHistory.removeLast();
    }
    
    if (!inclExcludeEdit->text().isEmpty() && !inclExcludeHistory.contains(inclExcludeEdit->text())) {
        inclExcludeHistory.prepend(inclExcludeEdit->text());
        if (inclExcludeHistory.size() > 10) inclExcludeHistory.removeLast();
    }
    
    // Path and pattern history settings removed - managed by main window
    settings.setValue("AddPatternHistory", addPatternHistory);
    settings.setValue("InclExcludeHistory", inclExcludeHistory);
    
    settings.endGroup();
}

void RGSearchDialog::onSaveParamsClicked()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString defaultConfigPath = appDir + "/data/config";
    
    // Ensure the config directory exists
    QDir().mkpath(defaultConfigPath);
    
    QString defaultPath = m_lastSaveLoadPath.isEmpty() ? 
        defaultConfigPath + "/search1.params" :
        m_lastSaveLoadPath + "/search1.params";
    
    QString fileName = QFileDialog::getSaveFileName(this, 
        "Save Search Parameters", 
        defaultPath,
        "Parameter Files (*.params);;All Files (*.*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Ensure .params extension
    if (!fileName.endsWith(".params")) {
        fileName += ".params";
    }
    
    QSettings paramsFile(fileName, QSettings::IniFormat);
    paramsFile.beginGroup("SearchParameters");
    
    // Save all current parameter values
    paramsFile.setValue("FixedString", fixedStringCheckBox->isChecked());
    paramsFile.setValue("AddPattern", addPatternEdit->text());
    paramsFile.setValue("CaseSensitive", caseSensitiveCheckBox->isChecked());
    paramsFile.setValue("IgnoreCase", ignoreCaseCheckBox->isChecked());
    paramsFile.setValue("SmartCase", smartCaseCheckBox->isChecked());
    paramsFile.setValue("InclExclude", inclExcludeEdit->text());
    paramsFile.setValue("KeepFilesInCache", keepFilesInCacheCheckBox->isChecked());
    paramsFile.setValue("HighlightColor", highlightColor);
    
    paramsFile.endGroup();
    
    // Remember the directory for next time
    QFileInfo fileInfo(fileName);
    m_lastSaveLoadPath = fileInfo.absolutePath();
    
    LOG_INFO("RGSearchDialog: Parameters saved to: " + fileName);
    QMessageBox::information(this, "Parameters Saved", 
        "Search parameters have been saved to:\n" + fileName);
}

void RGSearchDialog::onLoadParamsClicked()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString defaultConfigPath = appDir + "/data/config";
    
    QString defaultPath = m_lastSaveLoadPath.isEmpty() ? 
        defaultConfigPath :
        m_lastSaveLoadPath;
    
    QString fileName = QFileDialog::getOpenFileName(this, 
        "Load Search Parameters", 
        defaultPath,
        "Parameter Files (*.params);;All Files (*.*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QSettings paramsFile(fileName, QSettings::IniFormat);
    paramsFile.beginGroup("SearchParameters");
    
    // Load all parameter values
    fixedStringCheckBox->setChecked(paramsFile.value("FixedString", false).toBool());
    addPatternEdit->setText(paramsFile.value("AddPattern", "").toString());
    caseSensitiveCheckBox->setChecked(paramsFile.value("CaseSensitive", false).toBool());
    ignoreCaseCheckBox->setChecked(paramsFile.value("IgnoreCase", false).toBool());
    smartCaseCheckBox->setChecked(paramsFile.value("SmartCase", false).toBool());
    inclExcludeEdit->setText(paramsFile.value("InclExclude", "").toString());
    keepFilesInCacheCheckBox->setChecked(paramsFile.value("KeepFilesInCache", false).toBool());
    highlightColor = paramsFile.value("HighlightColor", QColor(130, 130, 130)).value<QColor>();
    
    paramsFile.endGroup();
    
    // Update the color preview label
    colorPreviewLabel->setStyleSheet(QString("background-color: %1; border: 2px solid black; border-radius: 4px;").arg(highlightColor.name()));
    
    // Remember the directory for next time
    QFileInfo fileInfo(fileName);
    m_lastSaveLoadPath = fileInfo.absolutePath();
    
    LOG_INFO("RGSearchDialog: Parameters loaded from: " + fileName);
}

// Case sensitivity mutual exclusivity slots
void RGSearchDialog::onCaseSensitiveToggled(bool checked)
{
    if (checked) {
        // Uncheck other case sensitivity options
        ignoreCaseCheckBox->setChecked(false);
        smartCaseCheckBox->setChecked(false);
    }
}

void RGSearchDialog::onIgnoreCaseToggled(bool checked)
{
    if (checked) {
        // Uncheck other case sensitivity options
        caseSensitiveCheckBox->setChecked(false);
        smartCaseCheckBox->setChecked(false);
    }
}

void RGSearchDialog::onSmartCaseToggled(bool checked)
{
    if (checked) {
        // Uncheck other case sensitivity options
        caseSensitiveCheckBox->setChecked(false);
        ignoreCaseCheckBox->setChecked(false);
    }
} 