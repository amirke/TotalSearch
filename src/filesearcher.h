#ifndef FILESEARCHER_H
#define FILESEARCHER_H

#include <QObject>

class FileSearcher : public QObject
{
    Q_OBJECT
public:
    FileSearcher(QObject *parent = nullptr);
    ~FileSearcher();
};

#endif // FILESEARCHER_H 