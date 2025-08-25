#ifndef FILELINEMODEL_H
#define FILELINEMODEL_H

#include <QAbstractListModel>
#include <QFile>
#include <QVector>
#include <QString>

class FileLineModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit FileLineModel(QObject *parent = nullptr);
    ~FileLineModel();

    // Load a file for line-by-line access
    bool loadFile(const QString &filePath);
    
    // Load segment around target line (targetLine - 500 to targetLine + 500)
    bool loadFileSegment(const QString &filePath, int targetLine);
    
    // Check if file is already loaded and target line is in current segment
    bool isFileLoaded(const QString &filePath, int targetLine) const;
    
    void clear();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QString getLine(int row) const;
    int lineCount() const;
    int maxLineWidth() const { return m_maxLineWidth; }
    int maxLineLength() const { return m_maxLineLength; }
    QString currentFilePath() const { return m_filePath; }
    
    // Get the actual line number for a given row (accounting for segment offset)
    int getActualLineNumber(int row) const;
    
    // Get the segment info
    int getSegmentStart() const { return m_segmentStart; }
    int getSegmentEnd() const { return m_segmentEnd; }

private:
    void loadSegment(int startLine, int endLine);
    
    mutable QFile m_file;
    QVector<qint64> m_lineOffsets; // Byte offsets for each line start
    QString m_filePath;
    mutable QString m_lastLine;
    int m_maxLineWidth = 0;
    int m_maxLineLength = 0;
    
    // Segment loading support
    int m_segmentStart = 0;
    int m_segmentEnd = 0;
    int m_totalLines = 0;
};

#endif // FILELINEMODEL_H 