#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QColorDialog>
#include <QLabel>
#include <QSpinBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QColor>

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = nullptr);

    // Get search parameters
    QString getPattern() const;
    bool isCaseSensitive() const;
    bool isInverse() const;
    bool isBoolean() const;
    bool isPlainText() const;
    bool isAutoRefresh() const;
    int getStartLine() const;
    int getEndLine() const;
    QColor getHighlightColor() const;
    QString getSearchEngine() const;
    QString getSearchType() const;
    
    // Set search parameters (for loading saved settings)
    void setPattern(const QString& pattern);
    void setSearchEngine(const QString& engine);
    void setSearchType(const QString& type);
    void setCaseSensitive(bool caseSensitive);
    void setInverse(bool inverse);
    void setBoolean(bool boolean);
    void setPlainText(bool plainText);
    void setAutoRefresh(bool autoRefresh);
    void setStartLine(int startLine);
    void setEndLine(int endLine);
    void setHighlightColor(const QColor& color);
    
private:
    void updateColorButton();

private slots:
    void onSearchClicked();
    void onColorButtonClicked();
    void updateToolTips();

private:
    void setupUI();
    void setupToolTips();

    // UI Components
    QLineEdit *patternEdit;
    QPushButton *colorButton;
    QPushButton *searchButton;
    QPushButton *cancelButton;
    
    QColor highlightColor;
    QLabel *colorPreviewLabel;
    
    // Make controls public for external access
public:
    QComboBox *searchEngineCombo;
    QComboBox *searchTypeCombo;
    QCheckBox *caseSensitiveCheck;
    QCheckBox *inverseCheck;
    QCheckBox *booleanCheck;
    QCheckBox *plainTextCheck;
    QCheckBox *autoRefreshCheck;
    QSpinBox *startLineSpin;
    QSpinBox *endLineSpin;
};

#endif // SEARCHDIALOG_H 