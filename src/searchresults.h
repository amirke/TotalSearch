#ifndef SEARCHRESULTS_H
#define SEARCHRESULTS_H

#include <QWidget>

class SearchResults : public QWidget
{
    Q_OBJECT
public:
    SearchResults(QWidget *parent = nullptr);
    ~SearchResults();
};

#endif // SEARCHRESULTS_H 