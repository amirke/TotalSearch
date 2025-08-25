#ifndef CONFIGURATIONDIALOG_H
#define CONFIGURATIONDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QColorDialog>
#include <QSettings>
#include <QColor>
#include <QList>
#include "highlightdialog.h"  // Include to use existing HighlightRule struct

class MainWindow;  // Forward declaration

class ConfigurationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigurationDialog(QWidget *parent = nullptr);
    ~ConfigurationDialog();
    
    // Setter for MainWindow reference
    void setMainWindow(MainWindow *mainWindow);
    
    // Getter methods for highlight engines
    bool isUseRGHighlight() const;
    bool isUseQtHighlight() const;
    bool isUseViewportHighlight() const;
    
    // Method to apply background highlighting settings immediately
    void applyBackgroundHighlightingSettings();
    
    // Method to apply layout settings immediately
    void applyLayoutSettings();

private slots:
    void onDoneClicked();
    void onCancelClicked();
    void onClearHistoryClicked();
    void onFontButtonClicked();
    void onPreviewFontButtonClicked();
    void onColorButtonClicked();
    void onBackgroundHighlightingChanged();
    void onChunkModeChanged();
    void onChunkSizeChanged();
    void onDurationChanged();
    void onLayoutSettingsChanged();

private:
    void setupUI();
    void setupTab0_General();
    void setupTab1_Fonts();
    void setupTab2_Layout();
    void setupTab3_BackgroundHighlighting();
    void setupTab4_HighlightEngines();
    void setupTab5_CustomColors();
    
    void loadSettings();
    void saveSettings();
    
    // Tab 0 - General
    QPushButton *clearHistoryButton;
    
    // Tab 1 - Fonts
    QLineEdit *fontSizeEdit;
    QLineEdit *previewFontSizeEdit;
    QPushButton *fontButton;
    QPushButton *previewFontButton;
    QLabel *fontPreviewLabel;
    QLabel *previewFontPreviewLabel;
    
    // Tab 2 - Layout
    QCheckBox *enableLogWindowCheck;
    QRadioButton *upDownLayoutRadio;
    QRadioButton *sideBySideLayoutRadio;
    QLabel *m_layoutPreviewLabel;
    
    // Tab 3 - Background Highlighting
    QCheckBox *backgroundHighlightingCheck;
    QComboBox *chunkModeCombo;
    QComboBox *chunkSizeCombo;
    QLineEdit *customLinesEdit;
    QComboBox *durationCombo;
    QLineEdit *customDurationEdit;
    QComboBox *idleDelayCombo;
    QComboBox *performanceModeCombo;
    QCheckBox *progressIndicatorCheck;
    QLabel *timingDiagram;
    
    // Tab 4 - Highlight Engines
    QCheckBox *useRGHighlightCheck;
    QCheckBox *useQtHighlightCheck;
    QCheckBox *useViewportHighlightCheck;
    
    // Tab 5 - Custom Colors
    QPushButton *searchResultsColorButton;
    QPushButton *fileContentColorButton;
    QPushButton *logWindowColorButton;
    QPushButton *statusBarColorButton;
    QLabel *searchResultsColorPreview;
    QLabel *fileContentColorPreview;
    QLabel *logWindowColorPreview;
    QLabel *statusBarColorPreview;
    
    // Main UI
    QTabWidget *tabWidget;
    QPushButton *doneButton;
    QPushButton *cancelButton;
    
    // MainWindow reference for applying settings
    MainWindow *m_mainWindow;
    
    void updateTimingDiagram();
    void updateLayoutPreview();
};

#endif // CONFIGURATIONDIALOG_H
