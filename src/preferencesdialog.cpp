#include "preferencesdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFontDialog>
#include <QColorDialog>
#include <QApplication>
#include <QSettings>
#include <QTabWidget>

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
    , m_searchEngine("ripgrep")
    , m_caseSensitive(false)
    , m_useRegex(false)
    , m_wholeWord(false)
    , m_ignoreHidden(true)
    , m_followSymlinks(false)
    , m_fileTypes("*.txt,*.log,*.cpp,*.h")
    , m_maxDepth(10)
    , m_excludePatterns("")
    , m_smartCase(false)
    , m_multiline(false)
    , m_dotAll(false)
    , m_noIgnore(false)
    , m_tabWidth(4)
    , m_useSpacesForTabs(true)
    , m_contextLines(5)
    , m_highlightColor(Qt::yellow)
    , m_chunkSize(5)  // Default 5MB like KLOGG
    , m_firstChunkSize(16)  // Default 16MB like KLOGG
    , m_showLineNumbers(true)  // Default show line numbers
{
    setWindowTitle("Preferences");
    setModal(true);
    resize(600, 500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Create tab widget
    QTabWidget *tabWidget = new QTabWidget();
    
    // Search tab
    QWidget *searchTab = new QWidget();
    QVBoxLayout *searchLayout = new QVBoxLayout(searchTab);
    
    // Search engine
    QHBoxLayout *engineLayout = new QHBoxLayout();
    QLabel *engineLabel = new QLabel("Search Engine:");
    searchEngineCombo = new QComboBox();
    searchEngineCombo->addItems({"ripgrep", "builtin"});
    engineLayout->addWidget(engineLabel);
    engineLayout->addWidget(searchEngineCombo);
    searchLayout->addLayout(engineLayout);
    
    // Search options
    QGroupBox *searchOptionsGroup = new QGroupBox("Search Options");
    QGridLayout *searchOptionsLayout = new QGridLayout(searchOptionsGroup);
    
    caseSensitiveCheck = new QCheckBox("Case Sensitive");
    useRegexCheck = new QCheckBox("Use Regular Expressions");
    wholeWordCheck = new QCheckBox("Whole Word");
    ignoreHiddenCheck = new QCheckBox("Ignore Hidden Files");
    followSymlinksCheck = new QCheckBox("Follow Symlinks");
    smartCaseCheck = new QCheckBox("Smart Case");
    multilineCheck = new QCheckBox("Multiline");
    dotAllCheck = new QCheckBox("Dot All");
    noIgnoreCheck = new QCheckBox("No Ignore");
    
    searchOptionsLayout->addWidget(caseSensitiveCheck, 0, 0);
    searchOptionsLayout->addWidget(useRegexCheck, 0, 1);
    searchOptionsLayout->addWidget(wholeWordCheck, 1, 0);
    searchOptionsLayout->addWidget(ignoreHiddenCheck, 1, 1);
    searchOptionsLayout->addWidget(followSymlinksCheck, 2, 0);
    searchOptionsLayout->addWidget(smartCaseCheck, 2, 1);
    searchOptionsLayout->addWidget(multilineCheck, 3, 0);
    searchOptionsLayout->addWidget(dotAllCheck, 3, 1);
    searchOptionsLayout->addWidget(noIgnoreCheck, 4, 0);
    
    searchLayout->addWidget(searchOptionsGroup);
    
    // File filters
    QGroupBox *fileFiltersGroup = new QGroupBox("File Filters");
    QGridLayout *fileFiltersLayout = new QGridLayout(fileFiltersGroup);
    
    QLabel *fileTypesLabel = new QLabel("File Types:");
    fileTypesEdit = new QLineEdit();
    fileTypesEdit->setPlaceholderText("*.txt,*.log,*.cpp,*.h");
    
    QLabel *maxDepthLabel = new QLabel("Max Depth:");
    maxDepthSpin = new QSpinBox();
    maxDepthSpin->setRange(0, 100);
    maxDepthSpin->setValue(10);
    
    QLabel *excludePatternsLabel = new QLabel("Exclude Patterns:");
    excludePatternsEdit = new QLineEdit();
    excludePatternsEdit->setPlaceholderText("*.tmp,*.bak");
    
    fileFiltersLayout->addWidget(fileTypesLabel, 0, 0);
    fileFiltersLayout->addWidget(fileTypesEdit, 0, 1);
    fileFiltersLayout->addWidget(maxDepthLabel, 1, 0);
    fileFiltersLayout->addWidget(maxDepthSpin, 1, 1);
    fileFiltersLayout->addWidget(excludePatternsLabel, 2, 0);
    fileFiltersLayout->addWidget(excludePatternsEdit, 2, 1);
    
    searchLayout->addWidget(fileFiltersGroup);
    
    tabWidget->addTab(searchTab, "Search");
    
    // Viewer tab
    QWidget *viewerTab = new QWidget();
    QVBoxLayout *viewerLayout = new QVBoxLayout(viewerTab);
    
    // Viewer options
    QGroupBox *viewerOptionsGroup = new QGroupBox("Viewer Options");
    QGridLayout *viewerOptionsLayout = new QGridLayout(viewerOptionsGroup);
    
    QLabel *tabWidthLabel = new QLabel("Tab Width:");
    tabWidthSpin = new QSpinBox();
    tabWidthSpin->setRange(1, 16);
    tabWidthSpin->setValue(4);
    
    useSpacesForTabsCheck = new QCheckBox("Use Spaces for Tabs");
    useSpacesForTabsCheck->setChecked(true);
    
    QLabel *contextLinesLabel = new QLabel("Context Lines:");
    contextLinesSpin = new QSpinBox();
    contextLinesSpin->setRange(0, 100);
    contextLinesSpin->setValue(5);
    
    QLabel *highlightColorLabel = new QLabel("Highlight Color:");
    highlightColorButton = new QPushButton();
    highlightColorButton->setFixedSize(50, 25);
    highlightColorButton->setStyleSheet(QString("background-color: %1; border: 1px solid black;").arg(m_highlightColor.name()));
    
    connect(highlightColorButton, &QPushButton::clicked, [this]() {
        QColor color = QColorDialog::getColor(m_highlightColor, this);
        if (color.isValid()) {
            m_highlightColor = color;
            highlightColorButton->setStyleSheet(QString("background-color: %1; border: 1px solid black;").arg(color.name()));
        }
    });
    
    viewerOptionsLayout->addWidget(tabWidthLabel, 0, 0);
    viewerOptionsLayout->addWidget(tabWidthSpin, 0, 1);
    viewerOptionsLayout->addWidget(useSpacesForTabsCheck, 1, 0, 1, 2);
    viewerOptionsLayout->addWidget(contextLinesLabel, 2, 0);
    viewerOptionsLayout->addWidget(contextLinesSpin, 2, 1);
    viewerOptionsLayout->addWidget(highlightColorLabel, 3, 0);
    viewerOptionsLayout->addWidget(highlightColorButton, 3, 1);
    
    viewerLayout->addWidget(viewerOptionsGroup);
    
    // KLOGG-style chunk sizes
    QGroupBox *chunkSizesGroup = new QGroupBox("KLOGG-Style Chunk Sizes");
    QGridLayout *chunkSizesLayout = new QGridLayout(chunkSizesGroup);
    
    QLabel *chunkSizeLabel = new QLabel("Indexing Block Size (MB):");
    chunkSizeSpin = new QSpinBox();
    chunkSizeSpin->setRange(1, 100);
    chunkSizeSpin->setValue(5);  // Default 5MB like KLOGG
    chunkSizeSpin->setSuffix(" MB");
    
    QLabel *firstChunkSizeLabel = new QLabel("First Chunk Size (MB):");
    firstChunkSizeSpin = new QSpinBox();
    firstChunkSizeSpin->setRange(1, 100);
    firstChunkSizeSpin->setValue(16);  // Default 16MB like KLOGG
    firstChunkSizeSpin->setSuffix(" MB");
    
    showLineNumbersCheck = new QCheckBox("Show Line Numbers");
    showLineNumbersCheck->setChecked(true);  // Default show line numbers
    
    chunkSizesLayout->addWidget(chunkSizeLabel, 0, 0);
    chunkSizesLayout->addWidget(chunkSizeSpin, 0, 1);
    chunkSizesLayout->addWidget(firstChunkSizeLabel, 1, 0);
    chunkSizesLayout->addWidget(firstChunkSizeSpin, 1, 1);
    chunkSizesLayout->addWidget(showLineNumbersCheck, 2, 0, 1, 2);
    
    viewerLayout->addWidget(chunkSizesGroup);
    
    tabWidget->addTab(viewerTab, "Viewer");
    
    mainLayout->addWidget(tabWidget);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("OK");
    QPushButton *cancelButton = new QPushButton("Cancel");
    
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    loadCurrentSettings();
}

void PreferencesDialog::loadCurrentSettings()
{
    QSettings settings("TotalSearch", "TotalSearch");
    
    // Load search parameters
    searchEngineCombo->setCurrentText(settings.value("searchEngine", "ripgrep").toString());
    caseSensitiveCheck->setChecked(settings.value("caseSensitive", false).toBool());
    useRegexCheck->setChecked(settings.value("useRegex", false).toBool());
    wholeWordCheck->setChecked(settings.value("wholeWord", false).toBool());
    ignoreHiddenCheck->setChecked(settings.value("ignoreHidden", true).toBool());
    followSymlinksCheck->setChecked(settings.value("followSymlinks", false).toBool());
    fileTypesEdit->setText(settings.value("fileTypes", "*.txt,*.log,*.cpp,*.h").toString());
    maxDepthSpin->setValue(settings.value("maxDepth", 10).toInt());
    excludePatternsEdit->setText(settings.value("excludePatterns", "").toString());
    smartCaseCheck->setChecked(settings.value("smartCase", false).toBool());
    multilineCheck->setChecked(settings.value("multiline", false).toBool());
    dotAllCheck->setChecked(settings.value("dotAll", false).toBool());
    noIgnoreCheck->setChecked(settings.value("noIgnore", false).toBool());
    
    // Load viewer settings
    tabWidthSpin->setValue(settings.value("tabWidth", 4).toInt());
    useSpacesForTabsCheck->setChecked(settings.value("useSpacesForTabs", true).toBool());
    contextLinesSpin->setValue(settings.value("contextLines", 5).toInt());
    m_highlightColor = settings.value("highlightColor", QColor(Qt::yellow)).value<QColor>();
    
    // Load KLOGG-style chunk sizes (convert from bytes to MB)
    int chunkSizeBytes = settings.value("chunkSize", 5 * 1024 * 1024).toInt(); // Default 5MB like KLOGG
    m_chunkSize = chunkSizeBytes / (1024 * 1024); // Convert to MB
    
    int firstChunkSizeBytes = settings.value("firstChunkSize", 16 * 1024 * 1024).toInt(); // Default 16MB like KLOGG
    m_firstChunkSize = firstChunkSizeBytes / (1024 * 1024); // Convert to MB
    
    // Load line numbers setting
    m_showLineNumbers = settings.value("showLineNumbers", true).toBool(); // Default show line numbers
    
    // Update UI with loaded values
    chunkSizeSpin->setValue(m_chunkSize);
    firstChunkSizeSpin->setValue(m_firstChunkSize);
    showLineNumbersCheck->setChecked(m_showLineNumbers);
    
    // Update highlight color display
    if (m_highlightColor.isValid()) {
        highlightColorButton->setStyleSheet(QString("background-color: %1; border: 1px solid black;").arg(m_highlightColor.name()));
    }
}

void PreferencesDialog::accept()
{
    // Save current values to member variables
    m_searchEngine = searchEngineCombo->currentText();
    m_caseSensitive = caseSensitiveCheck->isChecked();
    m_useRegex = useRegexCheck->isChecked();
    m_wholeWord = wholeWordCheck->isChecked();
    m_ignoreHidden = ignoreHiddenCheck->isChecked();
    m_followSymlinks = followSymlinksCheck->isChecked();
    m_fileTypes = fileTypesEdit->text();
    m_maxDepth = maxDepthSpin->value();
    m_excludePatterns = excludePatternsEdit->text();
    m_smartCase = smartCaseCheck->isChecked();
    m_multiline = multilineCheck->isChecked();
    m_dotAll = dotAllCheck->isChecked();
    m_noIgnore = noIgnoreCheck->isChecked();
    m_tabWidth = tabWidthSpin->value();
    m_useSpacesForTabs = useSpacesForTabsCheck->isChecked();
    m_contextLines = contextLinesSpin->value();
    m_chunkSize = chunkSizeSpin->value();
    m_firstChunkSize = firstChunkSizeSpin->value();
    m_showLineNumbers = showLineNumbersCheck->isChecked();
    m_highlightColor = m_highlightColor.isValid() ? m_highlightColor : QColor(224, 224, 224);
    
    QDialog::accept();
}

int PreferencesDialog::chunkSize() const
{
    return chunkSizeSpin->value();
}

int PreferencesDialog::firstChunkSize() const
{
    return firstChunkSizeSpin->value();
}

bool PreferencesDialog::showLineNumbers() const
{
    return showLineNumbersCheck->isChecked();
} 