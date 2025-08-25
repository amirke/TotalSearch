#ifndef FILEVIEWER_H
#define FILEVIEWER_H

#include <QWidget>

class FileViewer : public QWidget
{
    Q_OBJECT
public:
    FileViewer(QWidget *parent = nullptr);
    ~FileViewer();
};

#endif // FILEVIEWER_H 