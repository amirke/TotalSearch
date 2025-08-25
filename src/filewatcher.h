#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>

class FileWatcher : public QObject
{
    Q_OBJECT
public:
    FileWatcher(QObject *parent = nullptr);
    ~FileWatcher();
};

#endif // FILEWATCHER_H 