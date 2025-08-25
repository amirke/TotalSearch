#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QWidget>

class LogViewer : public QWidget
{
    Q_OBJECT
public:
    LogViewer(QWidget *parent = nullptr);
    ~LogViewer();
};

#endif // LOGVIEWER_H 