#ifndef RGSEARCHDIALOG_H
#define RGSEARCHDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QToolTip>
#include <QTextEdit>
#include <QColorDialog>
#include <QGroupBox>
#include "KSearchBun.h"

class RGSearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RGSearchDialog(QWidget *parent = nullptr);
    RGSearchParams getSearchParams() const;
    void setLogWidget(QTextEdit *logWidget);
    
    // Setters for pattern and path (now managed by main window)
    void setCurrentPattern(const QString &pattern) { m_currentPattern = pattern; }
    void setCurrentPath(const QString &path) { m_currentPath = path; }
    
    // KSearchBun instance (public for main window access)
    KSearchBun *kSearchBun;
    
private slots:
    void onSearchClicked();
    void onColorButtonClicked();
    void onDefaultColorButtonClicked();
    void onSaveParamsClicked();
    void onLoadParamsClicked();
    
    // Case sensitivity mutual exclusivity slots
    void onCaseSensitiveToggled(bool checked);
    void onIgnoreCaseToggled(bool checked);
    void onSmartCaseToggled(bool checked);

private:
    void setupUI();
    void setupToolTips();
    void loadSettings();
    void saveSettings();
    void loadHistory();
    void saveHistory();
    
    // UI elements (pathEdit, browseButton, patternEdit removed - now in main window)
    
    // Current search parameters (set by main window)
    QString m_currentPattern;
    QString m_currentPath;
    QCheckBox *fixedStringCheckBox;
    QLineEdit *addPatternEdit;
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *ignoreCaseCheckBox;
    QCheckBox *smartCaseCheckBox;
    QLineEdit *inclExcludeEdit;
    QCheckBox *keepFilesInCacheCheckBox;  // Keep searched files in cache
    
    // Highlight color picker
    QPushButton *colorButton;
    QPushButton *defaultColorButton;
    QLabel *colorPreviewLabel;
    QColor highlightColor;
    
    QPushButton *saveParamsButton;
    QPushButton *loadParamsButton;
    QPushButton *searchButton;
    QPushButton *cancelButton;
    
    // Log widget reference
    QTextEdit *m_logWidget;
    
    // Path memory for save/load dialogs
    QString m_lastSaveLoadPath;
};

#endif // RGSEARCHDIALOG_H 