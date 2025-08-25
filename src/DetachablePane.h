#ifndef DETACHABLEPANE_H
#define DETACHABLEPANE_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QSettings>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QResizeEvent>

class DetachablePane : public QWidget
{
    Q_OBJECT

public:
    explicit DetachablePane(const QString& title, QWidget* parent = nullptr);
    virtual ~DetachablePane();

    // Detach/attach functionality
    void detachFromMain();
    void attachToMain();
    bool isDetached() const { return m_isDetached; }
    
    // Parent management
    void setOriginalParent(QWidget* parent, QLayout* layout);

    // Window management
    void saveWindowState();
    void restoreWindowState();
    void setDefaultSize(const QSize& size);

    // Content management
    void setContentWidget(QWidget* widget);
    QWidget* getContentWidget() const { return m_contentWidget; }

    // Dynamic title updates
    void setPaneTitle(const QString& title);
    void setWindowTitle(const QString& title);
    QString getPaneTitle() const { return m_paneTitle; }

signals:
    void paneDetached(DetachablePane* pane);
    void paneAttached(DetachablePane* pane);
    void paneClosed(DetachablePane* pane);

protected:
    void closeEvent(QCloseEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onDetachButtonClicked();
    void onAttachButtonClicked();
    void onMaximizeButtonClicked();
    void onCloseButtonClicked();

private:
    void setupUI();
    void setupDetachedWindow();
    void setupAttachedWidget();

    // UI elements
    QWidget* m_contentWidget;
    QPushButton* m_detachButton;
    QPushButton* m_attachButton;
    QPushButton* m_maximizeButton;
    QPushButton* m_closeButton;
    QLabel* m_titleLabel;
    QHBoxLayout* m_titleBarLayout;
    QVBoxLayout* m_mainLayout;

    // State
    bool m_isDetached;
    QString m_paneTitle;
    QSize m_defaultSize;
    
    // Window state for detached mode
    QPoint m_detachedPosition;
    QSize m_detachedSize;
    bool m_detachedMaximized;
    
    // Original parent information
    QWidget* m_originalParent;
    QLayout* m_originalLayout;
};

#endif // DETACHABLEPANE_H
