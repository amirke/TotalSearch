#ifndef HIGHLIGHTDIALOG_H
#define HIGHLIGHTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QColorDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSettings>
#include <QColor>
#include <QList>

// Forward declaration
class ConfigurationDialog;

struct HighlightRule {
    QString pattern;
    QColor color;
    bool enabled;
    
    HighlightRule() : color(Qt::yellow), enabled(true) {}
    HighlightRule(const QString &p, const QColor &c, bool e) : pattern(p), color(c), enabled(e) {}
};

class HighlightDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HighlightDialog(QWidget *parent = nullptr);
    ~HighlightDialog();
    
    // Set configuration dialog reference
    void setConfigurationDialog(ConfigurationDialog *configDialog);

    QList<HighlightRule> getHighlightRules() const;
    void setHighlightRules(const QList<HighlightRule> &rules);
    
    // RG Search pattern integration
    void setRGSearchPattern(const QString &pattern);
    
    // New properties
    bool isCaseSensitive() const;
    bool isHighlightSentence() const;
    bool isUseRGHighlight() const;
    bool isUseQtHighlight() const;
    bool isUseViewportHighlight() const;
    
    // Background highlighting getters - REMOVED (moved to configuration dialog)

private slots:
    void onParamCountChanged(int count);
    void onColorButtonClicked();
    void onDefaultColorButtonClicked();
    void onDoneClicked();
    void onSaveParamsClicked();
    void onLoadParamsClicked();

    void loadSettings();
    void saveSettings();

private:
    void setupUI();
    void createHighlightLine(int index);
    void clearHighlightLines();
    void updateHighlightLines();

    QComboBox *paramCountCombo;
    QPushButton *saveParamsButton;
    QPushButton *loadParamsButton;
    QPushButton *doneButton;
    QPushButton *cancelButton;
    QVBoxLayout *highlightLayout;
    QList<QWidget*> highlightLineWidgets;
    QList<QLineEdit*> patternEdits;
    QList<QPushButton*> colorButtons;
    QList<QPushButton*> defaultColorButtons;
    QList<QCheckBox*> enabledChecks;
    QList<QLabel*> colorPreviewLabels;
    
    // New UI elements
    QCheckBox *caseSensitiveCheck;
    QCheckBox *highlightSentenceCheck;
    // Engine checkboxes removed - now managed in Configuration dialog
    
    // Background highlighting configuration - REMOVED (moved to configuration dialog)
    
    int currentParamCount;
    QList<HighlightRule> highlightRules;
    
    // RG Search pattern to be set after UI initialization
    QString m_pendingRGSearchPattern;
    
    // Reference to configuration dialog for engine settings
    ConfigurationDialog *m_configDialog;
    
    // Default colors for each pattern (1-10)
    QList<QColor> defaultColors;
    void setupDefaultColors();
    
    // Path memory for save/load dialogs
    QString m_lastSaveLoadPath;
};

#endif // HIGHLIGHTDIALOG_H
