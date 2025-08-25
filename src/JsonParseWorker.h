#ifndef JSONPARSEWORKER_H
#define JSONPARSEWORKER_H

#include <QObject>
#include <QString>

class JsonParseWorker : public QObject
{
    Q_OBJECT

public:
    explicit JsonParseWorker(QObject *parent = nullptr);

public slots:
    void parseJsonData(const QString &pattern, const QString &jsonData, const QString &searchPath);

signals:
    void parsingStarted();
    void parsingProgress(int percentage, int files);
    void parsingCompleted(int totalMatches, int totalFiles);
    void parsingError(const QString &error);
    void fileItemCreated(const QString &filePath, const QString &displayText);
    void matchItemCreated(const QString &filePath, int lineNumber, const QString &lineText);
    void fileStatsUpdated(const QString &filePath, const QString &elapsedTime, int matchedLines);
    void summaryParsed(const QString &summaryText, int totalMatchedLines);

private:
    QString parseRGSummary(const QStringList &lines, int* outMatchedLines = nullptr);
    void parseJsonDataInternal(const QStringList &lines, const QString &pattern, int totalMatchedLines = 0);
    QString createFileDisplayText(const QString &filePath, const QString &elapsedTime = QString(), int matchedLines = 0);
    QString createMatchDisplayText(const QString &filePath, int lineNumber, const QString &lineText);
    
    // Memory tracking
    void logMemoryUsage(const QString& stage);
    qint64 getCurrentMemoryUsage() const;
};

#endif // JSONPARSEWORKER_H
