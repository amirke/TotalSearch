#include "highlightdialog.h"
#include "configurationdialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QPainter>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFileInfo>
#include "logger.h"

HighlightDialog::HighlightDialog(QWidget *parent)
    : QDialog(parent)
    , currentParamCount(4)
    , m_configDialog(nullptr)
{
    setWindowTitle("Highlight in File");
    setModal(true);
    resize(600, 500);
    
    setupDefaultColors();
    setupUI();
    loadSettings();
    
    // Connect signals
    connect(paramCountCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &HighlightDialog::onParamCountChanged);
    connect(doneButton, &QPushButton::clicked, this, &HighlightDialog::onDoneClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveParamsButton, &QPushButton::clicked, this, &HighlightDialog::onSaveParamsClicked);
    connect(loadParamsButton, &QPushButton::clicked, this, &HighlightDialog::onLoadParamsClicked);
    

}

HighlightDialog::~HighlightDialog()
{
    saveSettings();
}

void HighlightDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Parameter count selection
    QHBoxLayout *paramLayout = new QHBoxLayout();
    QLabel *paramLabel = new QLabel("How many parameters:");
    paramCountCombo = new QComboBox(this);
    for (int i = 1; i <= 10; ++i) {
        paramCountCombo->addItem(QString::number(i));
    }
    paramCountCombo->setCurrentText("4"); // Default to 4
    
    paramLayout->addWidget(paramLabel);
    paramLayout->addWidget(paramCountCombo);
    paramLayout->addStretch();
    mainLayout->addLayout(paramLayout);
    
    // Global options group - REMOVED (moved to Configuration dialog)
    // The 3 engine checkboxes are now managed in Configuration dialog Tab 4
    

    

    
    // Highlight rules group
    QGroupBox *rulesGroup = new QGroupBox("Highlight Rules");
    highlightLayout = new QVBoxLayout(rulesGroup);
    
    // Case sensitive option (moved here from global options)
    caseSensitiveCheck = new QCheckBox("Case Sensitive", this);
    caseSensitiveCheck->setToolTip("Make all highlight rules case sensitive");
    highlightLayout->addWidget(caseSensitiveCheck);
    
    // Highlight All Sentence option (moved here from global options)
    highlightSentenceCheck = new QCheckBox("Highlight All Sentence", this);
    highlightSentenceCheck->setToolTip("Highlight the entire line instead of just the matched phrase");
    highlightLayout->addWidget(highlightSentenceCheck);
    
    // Create initial highlight lines
    updateHighlightLines();
    
    mainLayout->addWidget(rulesGroup);
    
    // Save/Load Parameters buttons (left side)
    QHBoxLayout *saveLoadLayout = new QHBoxLayout();
    
    saveParamsButton = new QPushButton("💾 Save Parameters");
    saveParamsButton->setToolTip("Save all highlight parameters to a .highlight file\n\nSaves:\n- All checkbox states\n- Text field values\n- Highlight colors\n- Case sensitivity settings\n- Number of parameters");
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
    loadParamsButton->setToolTip("Load highlight parameters from a .highlight file\n\nLoads:\n- All checkbox states\n- Text field values\n- Highlight colors\n- Case sensitivity settings\n- Number of parameters");
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
    doneButton = new QPushButton("Apply");
    // Create check mark icon using Unicode character
    QPixmap checkPixmap(16, 16);
    checkPixmap.fill(Qt::transparent);
    QPainter painter(&checkPixmap);
    painter.setPen(QPen(Qt::white, 2));
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(checkPixmap.rect(), Qt::AlignCenter, "✓");
    doneButton->setIcon(QIcon(checkPixmap));
    cancelButton = new QPushButton("Cancel");
    cancelButton->setIcon(QIcon(":/icons/close_window.png"));
    
    // Apply config dialog styling
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
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(doneButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);
    
    // Set tooltips
    paramCountCombo->setToolTip("Select the number of highlight rules to configure");
    doneButton->setToolTip("Apply and save the highlight rules");
    cancelButton->setToolTip("Cancel without saving changes");
}

void HighlightDialog::createHighlightLine(int index)
{
    QWidget *lineWidget = new QWidget(this);
    QHBoxLayout *lineLayout = new QHBoxLayout(lineWidget);
    lineLayout->setContentsMargins(0, 0, 0, 0);
    
    // Pattern field
    QLabel *patternLabel = new QLabel(QString("Pattern %1:").arg(index));
    QLineEdit *patternEdit = new QLineEdit(this);
    if (index == 0) {
        patternEdit->setPlaceholderText("RG Search pattern (auto-filled)");
        patternEdit->setToolTip("Pattern 0: Automatically filled from RG Search dialog\n\nThis pattern will be highlighted when enabled\n\nNote: This field is read-only and populated from RG Search");
        patternEdit->setReadOnly(true);
        patternEdit->setStyleSheet("QLineEdit { background-color: #f0f0f0; color: #808080; }");  // Grey background and text
    } else {
        patternEdit->setPlaceholderText("Enter regex pattern...");
        patternEdit->setToolTip("Enter a regular expression pattern to highlight\n\nExamples:\n- error|warning|fail\n- \\b\\w+@\\w+\\.\\w+\\b\n- \\d{4}-\\d{2}-\\d{2}");
    }
    
    // Color picker
    QPushButton *colorButton = new QPushButton("Choose Color", this);
    colorButton->setToolTip("Choose highlight color for this pattern");
    colorButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #007bff, stop:1 #0056b3);"
        "    border: none;"
        "    color: white;"
        "    padding: 6px 12px;"
        "    font-size: 12px;"
        "    font-weight: 500;"
        "    border-radius: 4px;"
        "    min-width: 100px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0056b3, stop:1 #004085);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #004085, stop:1 #002752);"
        "}"
    );
    
    // Color preview
    QLabel *colorPreview = new QLabel(this);
    colorPreview->setFixedSize(30, 20);
    
    // Use default color for this pattern
    QColor defaultColor = (index < defaultColors.size()) ? defaultColors[index] : Qt::yellow;
    colorPreview->setStyleSheet(QString("border: 1px solid black; background-color: %1;").arg(defaultColor.name()));
    colorPreview->setToolTip("Preview of selected color");
    
    // Enabled checkbox
    QCheckBox *enabledCheck = new QCheckBox("Enabled", this);
    enabledCheck->setChecked(true);
    enabledCheck->setToolTip("Enable or disable this highlight rule");
    
    // Default color button
    QPushButton *defaultColorButton = new QPushButton("Default Color", this);
    defaultColorButton->setToolTip("Reset to default color for this pattern");
    defaultColorButton->setStyleSheet(
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6c757d, stop:1 #495057);"
        "    border: none;"
        "    color: white;"
        "    padding: 6px 12px;"
        "    font-size: 12px;"
        "    font-weight: 500;"
        "    border-radius: 4px;"
        "    min-width: 100px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #868e96, stop:1 #6c757d);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #495057, stop:1 #343a40);"
        "}"
    );
    
    // Add to layout
    lineLayout->addWidget(patternLabel);
    lineLayout->addWidget(patternEdit);
    lineLayout->addWidget(colorButton);
    lineLayout->addWidget(defaultColorButton);
    lineLayout->addWidget(colorPreview);
    lineLayout->addWidget(enabledCheck);
    lineLayout->addStretch();
    
    // Store references
    highlightLineWidgets.append(lineWidget);
    patternEdits.append(patternEdit);
    colorButtons.append(colorButton);
    defaultColorButtons.append(defaultColorButton);
    colorPreviewLabels.append(colorPreview);
    enabledChecks.append(enabledCheck);
    
    // Connect buttons
    connect(colorButton, &QPushButton::clicked, this, &HighlightDialog::onColorButtonClicked);
    connect(defaultColorButton, &QPushButton::clicked, this, &HighlightDialog::onDefaultColorButtonClicked);
    
    // Add to main layout
    highlightLayout->addWidget(lineWidget);
}

void HighlightDialog::clearHighlightLines()
{
    for (QWidget *widget : highlightLineWidgets) {
        highlightLayout->removeWidget(widget);
        delete widget;
    }
    
    highlightLineWidgets.clear();
    patternEdits.clear();
    colorButtons.clear();
    defaultColorButtons.clear();
    colorPreviewLabels.clear();
    enabledChecks.clear();
}

void HighlightDialog::updateHighlightLines()
{
    // Store current rules before clearing
    QList<HighlightRule> currentRules;
    for (int i = 0; i < patternEdits.size(); ++i) {
        QString pattern = patternEdits[i]->text().trimmed();
        QLabel *previewLabel = colorPreviewLabels[i];
        QString style = previewLabel->styleSheet();
        QColor color = Qt::yellow; // Default
        
        // Extract color from style sheet
        int bgIndex = style.indexOf("background-color:");
        if (bgIndex != -1) {
            int startIndex = style.indexOf(":", bgIndex) + 1;
            int endIndex = style.indexOf(";", startIndex);
            if (endIndex == -1) endIndex = style.length();
            QString colorName = style.mid(startIndex, endIndex - startIndex).trimmed();
            color = QColor(colorName);
        }
        
        bool enabled = enabledChecks[i]->isChecked();
        currentRules.append(HighlightRule(pattern, color, enabled));
    }
    
    // Also check if we have saved rules that should be restored
    int totalRulesToConsider = qMax(currentRules.size(), highlightRules.size());
    
    clearHighlightLines();
    
    for (int i = 0; i < currentParamCount; ++i) {
        createHighlightLine(i);
        
        // First try to restore from current UI rules
        if (i < currentRules.size()) {
            const HighlightRule &rule = currentRules[i];
            patternEdits[i]->setText(rule.pattern);
            colorPreviewLabels[i]->setStyleSheet(QString("border: 1px solid black; background-color: %1;").arg(rule.color.name()));
            enabledChecks[i]->setChecked(rule.enabled);
        }
        // If not in current rules, try to restore from saved highlightRules (for disabled rules)
        else if (i < highlightRules.size()) {
            const HighlightRule &rule = highlightRules[i];
            patternEdits[i]->setText(rule.pattern);
            colorPreviewLabels[i]->setStyleSheet(QString("border: 1px solid black; background-color: %1;").arg(rule.color.name()));
            enabledChecks[i]->setChecked(rule.enabled);
            LOG_INFO("HighlightDialog: Restored disabled rule " + QString::number(i + 1) + 
                     " (pattern: '" + rule.pattern + "') with enabled state: " + QString(rule.enabled ? "Yes" : "No"));
        }
    }
    
    LOG_INFO("HighlightDialog: Updated highlight lines - preserved " + QString::number(qMin(totalRulesToConsider, currentParamCount)) + 
             " rules out of " + QString::number(totalRulesToConsider) + " total rules");
    
    // Apply pending RG search pattern to Pattern 0 if we have one
    if (!m_pendingRGSearchPattern.isEmpty() && !patternEdits.isEmpty()) {
        patternEdits[0]->setText(m_pendingRGSearchPattern);
        LOG_INFO("HighlightDialog: Applied pending RG Search pattern to Pattern 0: '" + m_pendingRGSearchPattern + "'");
    }
}

void HighlightDialog::onParamCountChanged(int index)
{
    int newParamCount = index + 1; // Convert from 0-based to 1-based
    int oldParamCount = currentParamCount;
    
    LOG_INFO("HighlightDialog: Parameter count changing from " + QString::number(oldParamCount) + 
             " to " + QString::number(newParamCount));
    
    // If reducing count, preserve rules by disabling them in settings
    if (newParamCount < oldParamCount) {
        LOG_INFO("HighlightDialog: Reducing parameter count - preserving rules by disabling excess ones");
        
        // Collect all current rules before the change
        QList<HighlightRule> allRules;
        for (int i = 0; i < patternEdits.size(); ++i) {
            QString pattern = patternEdits[i]->text().trimmed();
            QLabel *previewLabel = colorPreviewLabels[i];
            QString style = previewLabel->styleSheet();
            QColor color = Qt::yellow; // Default
            
            // Extract color from style sheet
            int bgIndex = style.indexOf("background-color:");
            if (bgIndex != -1) {
                int startIndex = style.indexOf(":", bgIndex) + 1;
                int endIndex = style.indexOf(";", startIndex);
                if (endIndex == -1) endIndex = style.length();
                QString colorName = style.mid(startIndex, endIndex - startIndex).trimmed();
                color = QColor(colorName);
            }
            
            bool enabled = enabledChecks[i]->isChecked();
            
            // Disable rules that will be hidden
            if (i >= newParamCount) {
                enabled = false;
                LOG_INFO("HighlightDialog: Disabling rule " + QString::number(i + 1) + 
                         " (pattern: '" + pattern + "') as it will be hidden");
            }
            
            allRules.append(HighlightRule(pattern, color, enabled));
        }
        
        // Update the highlightRules member to preserve disabled rules
        highlightRules = allRules;
    }
    
    currentParamCount = newParamCount;
    updateHighlightLines();
}

void HighlightDialog::onColorButtonClicked()
{
    QPushButton *senderButton = qobject_cast<QPushButton*>(sender());
    if (!senderButton) return;
    
    // Find the index of the button
    int buttonIndex = colorButtons.indexOf(senderButton);
    if (buttonIndex == -1) return;
    
    // Get current color from preview label
    QLabel *previewLabel = colorPreviewLabels[buttonIndex];
    QString currentStyle = previewLabel->styleSheet();
    QColor currentColor = Qt::yellow; // Default
    
    // Extract color from style sheet
    int bgIndex = currentStyle.indexOf("background-color:");
    if (bgIndex != -1) {
        int startIndex = currentStyle.indexOf(":", bgIndex) + 1;
        int endIndex = currentStyle.indexOf(";", startIndex);
        if (endIndex == -1) endIndex = currentStyle.length();
        QString colorName = currentStyle.mid(startIndex, endIndex - startIndex).trimmed();
        currentColor = QColor(colorName);
    }
    
    // Open color dialog
    QColor newColor = QColorDialog::getColor(currentColor, this, 
                                            QString("Choose Color for Pattern %1").arg(buttonIndex + 1));
    if (newColor.isValid()) {
        // Update preview label
        QString newStyle = QString("border: 1px solid black; background-color: %1;").arg(newColor.name());
        previewLabel->setStyleSheet(newStyle);
        LOG_INFO("HighlightDialog: Color changed for pattern " + QString::number(buttonIndex + 1) + 
                 " to " + newColor.name() + " (RGB: " + QString::number(newColor.rgb()) + 
                 " R:" + QString::number(newColor.red()) + 
                 " G:" + QString::number(newColor.green()) + 
                 " B:" + QString::number(newColor.blue()) + ")");
        LOG_INFO("HighlightDialog: Updated style sheet to: '" + newStyle + "'");
    }
}

void HighlightDialog::onDoneClicked()
{
    // Collect all highlight rules
    highlightRules.clear();
    
    LOG_INFO("HighlightDialog: Collecting highlight rules from " + QString::number(patternEdits.size()) + " pattern edits");
    
    for (int i = 0; i < patternEdits.size(); ++i) {
        QString pattern = patternEdits[i]->text().trimmed();
        
        // Special logging for Pattern 0 (Rule 0)
        if (i == 0) {
            LOG_INFO("HighlightDialog: Pattern 0 (Rule 0) text: '" + pattern + "' - This should be the current search pattern");
        } else {
            LOG_INFO("HighlightDialog: Pattern " + QString::number(i + 1) + " text: '" + pattern + "'");
        }
        
        // Get color from preview label
        QLabel *previewLabel = colorPreviewLabels[i];
        QString style = previewLabel->styleSheet();
        QColor color = Qt::yellow; // Default
        
        LOG_INFO("HighlightDialog: Extracting color from style: '" + style + "'");
        
        int bgIndex = style.indexOf("background-color:");
        if (bgIndex != -1) {
            int startIndex = style.indexOf(":", bgIndex) + 1;
            int endIndex = style.indexOf(";", startIndex);
            if (endIndex == -1) endIndex = style.length();
            QString colorName = style.mid(startIndex, endIndex - startIndex).trimmed();
            color = QColor(colorName);
            LOG_INFO("HighlightDialog: Extracted color name: '" + colorName + "', QColor valid: " + QString(color.isValid() ? "Yes" : "No"));
        } else {
            LOG_WARNING("HighlightDialog: Could not find 'background-color:' in style: '" + style + "'");
        }
        
        bool enabled = enabledChecks[i]->isChecked();
        
        // Always add the rule, even if pattern is empty (user might want to save the color/enabled state)
        highlightRules.append(HighlightRule(pattern, color, enabled));
        LOG_INFO("HighlightDialog: Rule " + QString::number(i + 1) + " - Pattern: '" + pattern + 
                 "', Color: " + color.name() + ", Enabled: " + QString(enabled ? "Yes" : "No"));
    }
    
    LOG_INFO("HighlightDialog: Total rules collected: " + QString::number(highlightRules.size()));
    saveSettings();
    accept();
}

QList<HighlightRule> HighlightDialog::getHighlightRules() const
{
    return highlightRules;
}

bool HighlightDialog::isCaseSensitive() const
{
    return caseSensitiveCheck->isChecked();
}

bool HighlightDialog::isHighlightSentence() const
{
    return highlightSentenceCheck->isChecked();
}

void HighlightDialog::setConfigurationDialog(ConfigurationDialog *configDialog)
{
    m_configDialog = configDialog;
}

bool HighlightDialog::isUseRGHighlight() const
{
    if (m_configDialog) {
        return m_configDialog->isUseRGHighlight();
    }
    return true; // Default to true if no config dialog
}

bool HighlightDialog::isUseQtHighlight() const
{
    if (m_configDialog) {
        return m_configDialog->isUseQtHighlight();
    }
    return false; // Default to false if no config dialog
}

bool HighlightDialog::isUseViewportHighlight() const
{
    if (m_configDialog) {
        return m_configDialog->isUseViewportHighlight();
    }
    return true; // Default to true if no config dialog
}



void HighlightDialog::setHighlightRules(const QList<HighlightRule> &rules)
{
    LOG_INFO("HighlightDialog: setHighlightRules called with " + QString::number(rules.size()) + " rules");
    
    highlightRules = rules;
    
    // Update UI to reflect the rules
    // If we have more rules than current param count, increase param count to show all rules
    if (rules.size() > currentParamCount) {
        currentParamCount = rules.size();
        LOG_INFO("HighlightDialog: Increasing param count to " + QString::number(currentParamCount) + " to show all rules");
    } else if (rules.size() == 0) {
        currentParamCount = 4; // Default
    }
    
    LOG_INFO("HighlightDialog: Setting param count to " + QString::number(currentParamCount));
    paramCountCombo->setCurrentText(QString::number(currentParamCount));
    updateHighlightLines();
    
    // Populate the fields
    for (int i = 0; i < rules.size() && i < patternEdits.size(); ++i) {
        const HighlightRule &rule = rules[i];
        
        // Special handling for Pattern 0 - always use the current search pattern
        if (i == 0 && !m_pendingRGSearchPattern.isEmpty()) {
            patternEdits[i]->setText(m_pendingRGSearchPattern);
            LOG_INFO("HighlightDialog: Populated field " + QString::number(i + 1) + " - Pattern: '" + m_pendingRGSearchPattern + 
                     "' (current search pattern, overriding saved rule: '" + rule.pattern + "')");
        } else {
            patternEdits[i]->setText(rule.pattern);
            LOG_INFO("HighlightDialog: Populated field " + QString::number(i + 1) + " - Pattern: '" + rule.pattern + 
                     "', Color: " + rule.color.name() + ", Enabled: " + QString(rule.enabled ? "Yes" : "No"));
        }
        
        colorPreviewLabels[i]->setStyleSheet(QString("border: 1px solid black; background-color: %1;").arg(rule.color.name()));
        enabledChecks[i]->setChecked(rule.enabled);
    }
}

void HighlightDialog::loadSettings()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = appDir + "/App.ini";
    QSettings settings(configFile, QSettings::IniFormat);
    settings.beginGroup("HighlightDialog");
    
    currentParamCount = settings.value("ParamCount", 4).toInt();
    paramCountCombo->setCurrentText(QString::number(currentParamCount));
    
    // Load global options
    bool caseSensitive = settings.value("CaseSensitive", false).toBool();
    bool highlightSentence = settings.value("HighlightSentence", false).toBool();
    bool useRGHighlight = settings.value("UseRGHighlight", false).toBool();
    bool useQtHighlight = settings.value("UseQtHighlight", true).toBool(); // Default to true
    bool useViewportHighlight = settings.value("UseViewportHighlight", false).toBool();
    caseSensitiveCheck->setChecked(caseSensitive);
    highlightSentenceCheck->setChecked(highlightSentence);
    // Engine settings are now managed in Configuration dialog
    
    // Load the last save/load path preference
    m_lastSaveLoadPath = settings.value("LastSaveLoadPath", "").toString();
    

    
    // Load highlight rules
    highlightRules.clear();
    int ruleCount = settings.value("RuleCount", 0).toInt();
    
    for (int i = 0; i < ruleCount; ++i) {
        QString groupKey = QString("Rule%1").arg(i);
        settings.beginGroup(groupKey);
        
        QString pattern = settings.value("Pattern", "").toString();
        QColor color = settings.value("Color", QColor(Qt::yellow)).value<QColor>();
        bool enabled = settings.value("Enabled", true).toBool();
        
        if (!pattern.isEmpty()) {
            highlightRules.append(HighlightRule(pattern, color, enabled));
        }
        
        settings.endGroup();
    }
    
    settings.endGroup();
    
    // Update UI
    updateHighlightLines();
    setHighlightRules(highlightRules);
    
    LOG_INFO("HighlightDialog: Loaded " + QString::number(highlightRules.size()) + " highlight rules from settings");
    LOG_INFO("HighlightDialog: Global options - Case Sensitive: " + QString(caseSensitive ? "Yes" : "No") + 
             ", Highlight Sentence: " + QString(highlightSentence ? "Yes" : "No"));
    

}

void HighlightDialog::saveSettings()
{
    LOG_INFO("HighlightDialog: Starting saveSettings with " + QString::number(highlightRules.size()) + " rules");
    
    QString appDir = QCoreApplication::applicationDirPath();
    QString configFile = appDir + "/App.ini";
    QSettings settings(configFile, QSettings::IniFormat);
    settings.beginGroup("HighlightDialog");
    
    settings.setValue("ParamCount", currentParamCount);
    settings.setValue("RuleCount", highlightRules.size());
    
    // Save global options
    settings.setValue("CaseSensitive", caseSensitiveCheck->isChecked());
    settings.setValue("HighlightSentence", highlightSentenceCheck->isChecked());
    // Engine settings are now managed in Configuration dialog
    
    // Save the last save/load path preference
    settings.setValue("LastSaveLoadPath", m_lastSaveLoadPath);
    

    
    // Save highlight rules
    for (int i = 0; i < highlightRules.size(); ++i) {
        QString groupKey = QString("Rule%1").arg(i);
        settings.beginGroup(groupKey);
        
        const HighlightRule &rule = highlightRules[i];
        settings.setValue("Pattern", rule.pattern);
        settings.setValue("Color", rule.color);
        settings.setValue("Enabled", rule.enabled);
        
        LOG_INFO("HighlightDialog: Saving rule " + QString::number(i + 1) + " - Pattern: '" + rule.pattern + 
                 "', Color: " + rule.color.name() + ", Enabled: " + QString(rule.enabled ? "Yes" : "No"));
        
        settings.endGroup();
    }
    
    settings.endGroup();
    
    LOG_INFO("HighlightDialog: Global options - Case Sensitive: " + QString(caseSensitiveCheck->isChecked() ? "Yes" : "No") + 
             ", Highlight Sentence: " + QString(highlightSentenceCheck->isChecked() ? "Yes" : "No"));
    LOG_INFO("HighlightDialog: Saved " + QString::number(highlightRules.size()) + " highlight rules to settings");
}

void HighlightDialog::setRGSearchPattern(const QString &pattern)
{
    // Store the pattern to be applied after UI initialization
    m_pendingRGSearchPattern = pattern;
    LOG_INFO("HighlightDialog: Stored RG Search pattern for later application: '" + pattern + "'");
    
    // If UI is already initialized, apply it immediately
    if (!patternEdits.isEmpty()) {
        patternEdits[0]->setText(pattern);
        LOG_INFO("HighlightDialog: Applied RG Search pattern to Pattern 0: '" + pattern + "'");
    }
}



void HighlightDialog::setupDefaultColors()
{
    defaultColors.clear();
    // Create 11 distinct default colors for patterns 0-10
    defaultColors = {
        QColor(255, 100, 100),  // Light Red (Pattern 0 - RG Search pattern)
        QColor(255, 255, 0),    // Yellow (Pattern 1)
        QColor(255, 0, 0),      // Red (Pattern 2)
        QColor(0, 255, 0),      // Green (Pattern 3)
        QColor(0, 0, 255),      // Blue (Pattern 4)
        QColor(255, 0, 255),    // Magenta (Pattern 5)
        QColor(0, 255, 255),    // Cyan (Pattern 6)
        QColor(255, 165, 0),    // Orange (Pattern 7)
        QColor(128, 0, 128),    // Purple (Pattern 8)
        QColor(255, 192, 203),  // Pink (Pattern 9)
        QColor(165, 42, 42)     // Brown (Pattern 10)
    };
    
    LOG_INFO("HighlightDialog: Setup " + QString::number(defaultColors.size()) + " default colors");
}

void HighlightDialog::onDefaultColorButtonClicked()
{
    QPushButton *senderButton = qobject_cast<QPushButton*>(sender());
    if (!senderButton) return;
    
    // Find the index of the button
    int buttonIndex = defaultColorButtons.indexOf(senderButton);
    if (buttonIndex == -1) return;
    
    // Get the default color for this pattern
    QColor defaultColor = (buttonIndex < defaultColors.size()) ? defaultColors[buttonIndex] : Qt::yellow;
    
    // Update preview label
    QLabel *previewLabel = colorPreviewLabels[buttonIndex];
    QString newStyle = QString("border: 1px solid black; background-color: %1;").arg(defaultColor.name());
    previewLabel->setStyleSheet(newStyle);
    
        LOG_INFO("HighlightDialog: Reset color for pattern " + QString::number(buttonIndex + 1) +
             " to default: " + defaultColor.name());
}

void HighlightDialog::onSaveParamsClicked()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString configPath = appDir + "/data/config";
    
    // Ensure the config directory exists
    QDir().mkpath(configPath);
    
    QString defaultPath = m_lastSaveLoadPath.isEmpty() ? 
        configPath + "/highlight1.highlight" :
        m_lastSaveLoadPath + "/highlight1.highlight";
    
    QString fileName = QFileDialog::getSaveFileName(this, 
        "Save Highlight Parameters", 
        defaultPath,
        "Highlight Files (*.highlight);;All Files (*.*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Ensure .highlight extension
    if (!fileName.endsWith(".highlight")) {
        fileName += ".highlight";
    }
    
    // Collect current UI state before saving
    QList<HighlightRule> currentUIRules;
    for (int i = 0; i < patternEdits.size(); ++i) {
        QString pattern = patternEdits[i]->text().trimmed();
        QLabel *previewLabel = colorPreviewLabels[i];
        QString style = previewLabel->styleSheet();
        QColor color = Qt::yellow; // Default
        
        // Extract color from style sheet
        int bgIndex = style.indexOf("background-color:");
        if (bgIndex != -1) {
            int startIndex = style.indexOf(":", bgIndex) + 1;
            int endIndex = style.indexOf(";", startIndex);
            if (endIndex == -1) endIndex = style.length();
            QString colorName = style.mid(startIndex, endIndex - startIndex).trimmed();
            color = QColor(colorName);
        }
        
        bool enabled = enabledChecks[i]->isChecked();
        currentUIRules.append(HighlightRule(pattern, color, enabled));
        
        LOG_INFO("HighlightDialog: Saving rule " + QString::number(i + 1) + 
                 " - Pattern: '" + pattern + "', Color: " + color.name() + 
                 ", Enabled: " + QString(enabled ? "Yes" : "No"));
    }
    
    QSettings paramsFile(fileName, QSettings::IniFormat);
    paramsFile.beginGroup("HighlightParameters");
    
    // Save all current parameter values
    paramsFile.setValue("ParamCount", currentParamCount);
    paramsFile.setValue("CaseSensitive", caseSensitiveCheck->isChecked());
    paramsFile.setValue("HighlightSentence", highlightSentenceCheck->isChecked());
    
    LOG_INFO("HighlightDialog: Saving global parameters - ParamCount: " + QString::number(currentParamCount) + 
             ", CaseSensitive: " + QString(caseSensitiveCheck->isChecked() ? "Yes" : "No") + 
             ", HighlightSentence: " + QString(highlightSentenceCheck->isChecked() ? "Yes" : "No"));
    
    // Save highlight rules from current UI state
    paramsFile.setValue("RuleCount", currentUIRules.size());
    for (int i = 0; i < currentUIRules.size(); ++i) {
        QString groupKey = QString("Rule%1").arg(i);
        paramsFile.beginGroup(groupKey);
        
        const HighlightRule &rule = currentUIRules[i];
        paramsFile.setValue("Pattern", rule.pattern);
        paramsFile.setValue("Color", rule.color);
        paramsFile.setValue("Enabled", rule.enabled);
        
        paramsFile.endGroup();
    }
    
    paramsFile.endGroup();
    
    // Remember the directory for next time
    QFileInfo fileInfo(fileName);
    m_lastSaveLoadPath = fileInfo.absolutePath();
    
    LOG_INFO("HighlightDialog: Parameters saved to: " + fileName + " - Total rules saved: " + QString::number(currentUIRules.size()));
    QMessageBox::information(this, "Parameters Saved", 
        "Highlight parameters have been saved to:\n" + fileName);
}

void HighlightDialog::onLoadParamsClicked()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString defaultConfigPath = appDir + "/data/config";
    
    QString defaultPath = m_lastSaveLoadPath.isEmpty() ? 
        defaultConfigPath :
        m_lastSaveLoadPath;
    
    QString fileName = QFileDialog::getOpenFileName(this, 
        "Load Highlight Parameters", 
        defaultPath,
        "Highlight Files (*.highlight);;All Files (*.*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QSettings paramsFile(fileName, QSettings::IniFormat);
    paramsFile.beginGroup("HighlightParameters");
    
    // Load all parameter values
    currentParamCount = paramsFile.value("ParamCount", 4).toInt();
    caseSensitiveCheck->setChecked(paramsFile.value("CaseSensitive", false).toBool());
    highlightSentenceCheck->setChecked(paramsFile.value("HighlightSentence", true).toBool());
    
    LOG_INFO("HighlightDialog: Loading global parameters - ParamCount: " + QString::number(currentParamCount) + 
             ", CaseSensitive: " + QString(caseSensitiveCheck->isChecked() ? "Yes" : "No") + 
             ", HighlightSentence: " + QString(highlightSentenceCheck->isChecked() ? "Yes" : "No"));
    
    // Load highlight rules
    int ruleCount = paramsFile.value("RuleCount", 0).toInt();
    highlightRules.clear();
    
    for (int i = 0; i < ruleCount; ++i) {
        QString groupKey = QString("Rule%1").arg(i);
        paramsFile.beginGroup(groupKey);
        
        HighlightRule rule;
        rule.pattern = paramsFile.value("Pattern", "").toString();
        rule.color = paramsFile.value("Color", QColor(Qt::yellow)).value<QColor>();
        rule.enabled = paramsFile.value("Enabled", true).toBool();
        
        highlightRules.append(rule);
        
        LOG_INFO("HighlightDialog: Loading rule " + QString::number(i + 1) + 
                 " - Pattern: '" + rule.pattern + "', Color: " + rule.color.name() + 
                 ", Enabled: " + QString(rule.enabled ? "Yes" : "No"));
        
        paramsFile.endGroup();
    }
    
    paramsFile.endGroup();
    
    // Remember the directory for next time
    QFileInfo fileInfo(fileName);
    m_lastSaveLoadPath = fileInfo.absolutePath();
    
    // Update the UI to reflect the loaded parameters
    paramCountCombo->setCurrentText(QString::number(currentParamCount));
    
    // Clear and recreate highlight lines to ensure loaded rules are applied
    clearHighlightLines();
    
    for (int i = 0; i < currentParamCount; ++i) {
        createHighlightLine(i);
        
        // Apply loaded rules to UI
        if (i < highlightRules.size()) {
            const HighlightRule &rule = highlightRules[i];
            patternEdits[i]->setText(rule.pattern);
            colorPreviewLabels[i]->setStyleSheet(QString("border: 1px solid black; background-color: %1;").arg(rule.color.name()));
            enabledChecks[i]->setChecked(rule.enabled);
            LOG_INFO("HighlightDialog: Applied loaded rule " + QString::number(i + 1) + 
                     " to UI - Pattern: '" + rule.pattern + "', Color: " + rule.color.name() + 
                     ", Enabled: " + QString(rule.enabled ? "Yes" : "No"));
        }
    }
    
    LOG_INFO("HighlightDialog: Parameters loaded from: " + fileName + " - Total rules loaded: " + QString::number(ruleCount));
}




