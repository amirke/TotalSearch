#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QFontDialog>
#include <QFont>
#include <QApplication>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QColor>

class PreferencesDialog : public QDialog
{
    Q_OBJECT
public:
    PreferencesDialog(QWidget *parent = nullptr);
    QFont selectedFont() const { return m_selectedFont; }
    QString searchResultsPosition() const { return m_searchResultsPosition; }
    bool showLogWindow() const { return m_showLogWindow; }
    QString searchEngine() const { return m_searchEngine; }
    bool caseSensitive() const { return m_caseSensitive; }
    bool useRegex() const { return m_useRegex; }
    bool wholeWord() const { return m_wholeWord; }
    bool ignoreHidden() const { return m_ignoreHidden; }
    bool followSymlinks() const { return m_followSymlinks; }
    QString fileTypes() const { return m_fileTypes; }
    int maxDepth() const { return m_maxDepth; }
    QString excludePatterns() const { return m_excludePatterns; }
    bool smartCase() const { return m_smartCase; }
    bool multiline() const { return m_multiline; }
    bool dotAll() const { return m_dotAll; }
    bool noIgnore() const { return m_noIgnore; }
    
    // Viewer settings
    int tabWidth() const { return m_tabWidth; }
    bool useSpacesForTabs() const { return m_useSpacesForTabs; }
    int contextLines() const { return m_contextLines; }
    QColor highlightColor() const { return m_highlightColor; }
    int chunkSize() const;
    int firstChunkSize() const;
    bool showLineNumbers() const;
    
private slots:
    void accept();
    
private:
    void loadCurrentSettings();
    
    QFont m_selectedFont;
    QString m_searchResultsPosition;
    bool m_showLogWindow;
    
    // Search engine settings
    QComboBox *searchEngineCombo;
    QCheckBox *caseSensitiveCheck;
    QCheckBox *useRegexCheck;
    QCheckBox *wholeWordCheck;
    QCheckBox *ignoreHiddenCheck;
    QCheckBox *followSymlinksCheck;
    QLineEdit *fileTypesEdit;
    QSpinBox *maxDepthSpin;
    QLineEdit *excludePatternsEdit;
    QCheckBox *smartCaseCheck;
    QCheckBox *multilineCheck;
    QCheckBox *dotAllCheck;
    QCheckBox *noIgnoreCheck;
    
    // Viewer settings controls
    QSpinBox *tabWidthSpin;
    QCheckBox *useSpacesForTabsCheck;
    QSpinBox *contextLinesSpin;
    QPushButton *highlightColorButton;
    QSpinBox *chunkSizeSpin;
    QSpinBox *firstChunkSizeSpin;
    QCheckBox *showLineNumbersCheck;
    
    QString m_searchEngine;
    bool m_caseSensitive;
    bool m_useRegex;
    bool m_wholeWord;
    bool m_ignoreHidden;
    bool m_followSymlinks;
    QString m_fileTypes;
    int m_maxDepth;
    QString m_excludePatterns;
    bool m_smartCase;
    bool m_multiline;
    bool m_dotAll;
    bool m_noIgnore;
    int m_tabWidth;
    bool m_useSpacesForTabs;
    int m_contextLines;
    QColor m_highlightColor;
    int m_chunkSize;
    int m_firstChunkSize;
    bool m_showLineNumbers;
};

#endif // PREFERENCESDIALOG_H 