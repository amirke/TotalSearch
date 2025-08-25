#include "searchdialog.h"
#include <QApplication>
#include <QStyle>
#include <QMessageBox>

SearchDialog::SearchDialog(QWidget *parent)
    : QDialog(parent)
    , highlightColor(Qt::yellow)
{
    setWindowTitle("Find in File");
    setModal(true);
    setFixedSize(400, 500);
    
    setupUI();
    setupToolTips();
}

void SearchDialog::setupUI()
{
    auto mainLayout = new QVBoxLayout(this);
    
    // Pattern input
    auto patternGroup = new QGroupBox("Search Pattern");
    auto patternLayout = new QFormLayout(patternGroup);
    
    patternEdit = new QLineEdit(this);
    patternEdit->setPlaceholderText("Enter search pattern...");
    patternEdit->setToolTip("Search Pattern Examples:\n\n"
                           "Simple Text:\n"
                           "- 'error' - Find lines containing 'error'\n"
                           "- 'warning' - Find lines containing 'warning'\n"
                           "- 'fail' - Find lines containing 'fail'\n\n"
                           "Regular Expressions:\n"
                           "- 'error|warning|fail' - Find lines with any of these words\n"
                           "- '\\d{4}-\\d{2}-\\d{2}' - Find date patterns (YYYY-MM-DD)\n"
                           "- '\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}\\b' - Find email addresses\n"
                           "- '^ERROR' - Find lines starting with 'ERROR'\n"
                           "- 'error$' - Find lines ending with 'error'\n"
                           "- '\\b\\w+@\\w+\\.\\w+\\b' - Find simple email patterns\n\n"
                           "Wildcard Patterns:\n"
                           "- '*.log' - Find lines containing '*.log' literally\n"
                           "- 'file*' - Find lines containing 'file' followed by anything\n"
                           "- 'test??' - Find lines containing 'test' followed by 2 characters\n\n"
                           "Boolean Expressions:\n"
                           "- 'error AND warning' - Lines containing both 'error' and 'warning'\n"
                           "- 'error OR fail' - Lines containing either 'error' or 'fail'\n"
                           "- 'error AND NOT debug' - Lines with 'error' but not 'debug'\n\n"
                           "Special Characters:\n"
                           "- '\\[' - Find literal '[' character\n"
                           "- '\\(' - Find literal '(' character\n"
                           "- '\\$' - Find literal '$' character");
    patternLayout->addRow("Pattern:", patternEdit);
    
    mainLayout->addWidget(patternGroup);
    
    // Search options
    auto optionsGroup = new QGroupBox("Search Options");
    auto optionsLayout = new QFormLayout(optionsGroup);
    
    // Search Engine
    searchEngineCombo = new QComboBox(this);
    searchEngineCombo->addItem("Hyperscan (High Performance)", "Hyperscan");
    searchEngineCombo->addItem("QRegularExpression (Compatible)", "QRegularExpression");
    searchEngineCombo->setCurrentIndex(0); // Default to Hyperscan
    optionsLayout->addRow("Search Engine:", searchEngineCombo);
    
    // Search Type
    searchTypeCombo = new QComboBox(this);
    searchTypeCombo->addItem("Extended Regex", "ExtendedRegex");
    searchTypeCombo->addItem("Wildcard", "Wildcard");
    searchTypeCombo->addItem("Fixed String", "FixedString");
    searchTypeCombo->setCurrentIndex(0); // Default to Extended Regex
    optionsLayout->addRow("Search Type:", searchTypeCombo);
    
    caseSensitiveCheck = new QCheckBox("Case Sensitive", this);
    caseSensitiveCheck->setChecked(true);
    optionsLayout->addRow(caseSensitiveCheck);
    
    inverseCheck = new QCheckBox("Inverse Match (Exclude)", this);
    optionsLayout->addRow(inverseCheck);
    
    booleanCheck = new QCheckBox("Boolean Combining", this);
    optionsLayout->addRow(booleanCheck);
    
    plainTextCheck = new QCheckBox("Plain Text Mode", this);
    optionsLayout->addRow(plainTextCheck);
    
    autoRefreshCheck = new QCheckBox("Auto Refresh", this);
    autoRefreshCheck->setChecked(true);
    optionsLayout->addRow(autoRefreshCheck);
    
    mainLayout->addWidget(optionsGroup);
    
    // Search range
    auto rangeGroup = new QGroupBox("Search Range");
    auto rangeLayout = new QFormLayout(rangeGroup);
    
    startLineSpin = new QSpinBox(this);
    startLineSpin->setMinimum(1);
    startLineSpin->setMaximum(999999999);
    startLineSpin->setValue(1);
    rangeLayout->addRow("Start Line:", startLineSpin);
    
    endLineSpin = new QSpinBox(this);
    endLineSpin->setMinimum(1);
    endLineSpin->setMaximum(999999999);
    endLineSpin->setValue(999999999);
    rangeLayout->addRow("End Line:", endLineSpin);
    
    mainLayout->addWidget(rangeGroup);
    
    // Highlight color
    auto colorGroup = new QGroupBox("Highlight Color");
    auto colorLayout = new QHBoxLayout(colorGroup);
    
    colorButton = new QPushButton("Choose Color", this);
    colorPreviewLabel = new QLabel(this);
    colorPreviewLabel->setFixedSize(40, 25);
    colorPreviewLabel->setStyleSheet(QString("background-color: %1; border: 2px solid black; border-radius: 4px;").arg(highlightColor.name()));
    colorPreviewLabel->setToolTip("Current highlight color");
    
    colorLayout->addWidget(new QLabel("Highlight Color:"));
    colorLayout->addWidget(colorButton);
    colorLayout->addWidget(colorPreviewLabel);
    colorLayout->addStretch();
    
    mainLayout->addWidget(colorGroup);
    
    // Buttons
    auto buttonLayout = new QHBoxLayout();
    
    searchButton = new QPushButton("Search", this);
    searchButton->setDefault(true);
    cancelButton = new QPushButton("Cancel", this);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(searchButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(searchButton, &QPushButton::clicked, this, &SearchDialog::onSearchClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(colorButton, &QPushButton::clicked, this, &SearchDialog::onColorButtonClicked);
    
    // Set focus to pattern edit
    patternEdit->setFocus();
}

void SearchDialog::setupToolTips()
{
    patternEdit->setToolTip("Enter the text or pattern to search for in the file\n\nExamples:\n- Simple text: 'error'\n- Regex: 'error|warning|fail'\n- Wildcard: '*.log'\n- Boolean: 'error AND not debug'");
    
    caseSensitiveCheck->setToolTip("When checked, the search will be case sensitive (e.g., 'Error' != 'error')\n\nExample: 'ERROR' will not match 'error' or 'Error'");
    
    inverseCheck->setToolTip("When checked, the search will exclude matching lines instead of including them\n\nExample: Searching for 'error' with inverse will show all lines EXCEPT those containing 'error'");
    
    booleanCheck->setToolTip("Enable logical combining for complex search expressions (AND, OR, NOT operations)\n\nExamples:\n- 'error AND warning': Lines containing both\n- 'error OR fail': Lines containing either\n- 'error AND NOT debug': Lines with 'error' but not 'debug'");
    
    plainTextCheck->setToolTip("When checked, the pattern is treated as plain text instead of a regular expression\n\nExample: 'error.*' will search for literal text 'error.*' instead of regex pattern");
    
    autoRefreshCheck->setToolTip("Automatically refresh search results when the file changes\n\nUseful for log files that are being written to in real-time");
    
    startLineSpin->setToolTip("The line number to start searching from (1-based)\n\nExample: Set to 1000 to search only from line 1000 onwards");
    
    endLineSpin->setToolTip("The line number to stop searching at (inclusive)\n\nExample: Set to 5000 to search only up to line 5000");
    
    colorButton->setToolTip("Choose the color to highlight matching text in the file\n\nSelected color will be used to highlight all matching patterns in the file viewport");
    
    searchEngineCombo->setToolTip("Select the search engine to use\n\nHyperscan: High-performance regex engine (default)\nQRegularExpression: Qt's regex engine (more compatible)");
    
    searchTypeCombo->setToolTip("Select the type of pattern matching\n\nExtended Regex: Full regex support (default)\nWildcard: Simple wildcard patterns (*, ?)\nFixed String: Literal text matching");
}

void SearchDialog::updateToolTips()
{
    // This function is called when tooltips need to be updated
    // Currently just calls setupToolTips() to ensure all tooltips are set
    setupToolTips();
}

void SearchDialog::onSearchClicked()
{
    if (patternEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Invalid Pattern", "Please enter a search pattern.");
        patternEdit->setFocus();
        return;
    }
    
    if (startLineSpin->value() > endLineSpin->value()) {
        QMessageBox::warning(this, "Invalid Range", "Start line cannot be greater than end line.");
        return;
    }
    
    accept();
}

void SearchDialog::onColorButtonClicked()
{
    QColor newColor = QColorDialog::getColor(highlightColor, this, "Choose Highlight Color");
    if (newColor.isValid()) {
        highlightColor = newColor;
        colorPreviewLabel->setStyleSheet(QString("background-color: %1; border: 1px solid black;").arg(highlightColor.name()));
    }
}

QString SearchDialog::getPattern() const
{
    return patternEdit->text().trimmed();
}

bool SearchDialog::isCaseSensitive() const
{
    return caseSensitiveCheck->isChecked();
}

bool SearchDialog::isInverse() const
{
    return inverseCheck->isChecked();
}

bool SearchDialog::isBoolean() const
{
    return booleanCheck->isChecked();
}

bool SearchDialog::isPlainText() const
{
    return plainTextCheck->isChecked();
}

bool SearchDialog::isAutoRefresh() const
{
    return autoRefreshCheck->isChecked();
}

int SearchDialog::getStartLine() const
{
    return startLineSpin->value();
}

int SearchDialog::getEndLine() const
{
    return endLineSpin->value();
}

QColor SearchDialog::getHighlightColor() const
{
    return highlightColor;
}

QString SearchDialog::getSearchEngine() const
{
    return searchEngineCombo->currentData().toString();
}

QString SearchDialog::getSearchType() const
{
    return searchTypeCombo->currentData().toString();
}

void SearchDialog::setPattern(const QString& pattern)
{
    if (patternEdit) {
        patternEdit->setText(pattern);
    }
}

void SearchDialog::setSearchEngine(const QString& engine)
{
    if (searchEngineCombo) {
        int index = searchEngineCombo->findData(engine);
        if (index >= 0) {
            searchEngineCombo->setCurrentIndex(index);
        }
    }
}

void SearchDialog::setSearchType(const QString& type)
{
    if (searchTypeCombo) {
        int index = searchTypeCombo->findData(type);
        if (index >= 0) {
            searchTypeCombo->setCurrentIndex(index);
        }
    }
}

void SearchDialog::setCaseSensitive(bool caseSensitive)
{
    if (caseSensitiveCheck) {
        caseSensitiveCheck->setChecked(caseSensitive);
    }
}

void SearchDialog::setInverse(bool inverse)
{
    if (inverseCheck) {
        inverseCheck->setChecked(inverse);
    }
}

void SearchDialog::setBoolean(bool boolean)
{
    if (booleanCheck) {
        booleanCheck->setChecked(boolean);
    }
}

void SearchDialog::setPlainText(bool plainText)
{
    if (plainTextCheck) {
        plainTextCheck->setChecked(plainText);
    }
}

void SearchDialog::setAutoRefresh(bool autoRefresh)
{
    if (autoRefreshCheck) {
        autoRefreshCheck->setChecked(autoRefresh);
    }
}

void SearchDialog::setStartLine(int startLine)
{
    if (startLineSpin) {
        startLineSpin->setValue(startLine);
    }
}

void SearchDialog::setEndLine(int endLine)
{
    if (endLineSpin) {
        endLineSpin->setValue(endLine);
    }
}

void SearchDialog::setHighlightColor(const QColor& color)
{
    highlightColor = color;
    updateColorButton();
}

void SearchDialog::updateColorButton()
{
    if (colorPreviewLabel) {
        colorPreviewLabel->setStyleSheet(QString("background-color: %1; border: 2px solid black; border-radius: 4px;")
                                       .arg(highlightColor.name()));
    }
} 