#ifndef KSEARCHBUN_H
#define KSEARCHBUN_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QVector>
#include <QThread>
#include <QMutex>
#include <QProcess>
#include <QElapsedTimer>
#include <QDateTime>
#include <QSettings>
#include <QRegularExpression>
#include <QListWidget>
#include <QColor>

// Forward declarations
struct Match;
struct FileMapping;

// Search parameters structure
struct RGSearchParams {
    QString path;           // Full path of library
    QString pattern;        // Pattern to search
    bool fixed_string;      // Treat patterns as literal text
    QString add_pattern;    // Additional pattern (regex with |)
    bool case_sensitive;    // Case sensitive flag
    bool ignore_case;       // Ignore case flag
    bool smart_case;        // Smart case flag
    QString incl_exclude;   // Include/exclude patterns
    bool keep_files_in_cache; // Keep searched files in memory cache
    QColor highlight_color; // Color to highlight matching text
    
    RGSearchParams() : fixed_string(false), case_sensitive(false), 
                      ignore_case(false), smart_case(false), keep_files_in_cache(false),
                      highlight_color(Qt::yellow) {}
};



class KSearchBun : public QObject
{
    Q_OBJECT

    // Allow KSearch to access private members for process termination
    friend class KSearch;

public:
    explicit KSearchBun(QObject *parent = nullptr);
    
    // Method 3: Synchronous implementation (simple and reliable)
    QString K_RGresults_method3(const RGSearchParams &params);
    
    // Parse ripgrep output into search results
    QMap<QString, QVector<Match>> parseRGMainResults(const QString &allOutput);
    
    // Method 3 Async: Asynchronous version of method 3
    void K_RGresults_method3_async(const RGSearchParams &params);
    
    // Parse Async: Asynchronous version of parse function
    void parseRGMainResults_async(const QString &allOutput);
    
    // KMap function - gets file path and returns list of line offsets
    QVector<long> KMap(const QString &file_path);
    
    // Set main window reference for accessing UI components
    void setMainWindow(QWidget *mainWindow);
    
    // Get the full ripgrep command that would be executed
    QString getFullRipgrepCommand(const RGSearchParams &params) const;
    
    // Clear all internal data structures for fresh start
    void clearAllData();
    
    // Persistent search parameters management
    void updateSearchParams(const RGSearchParams &params);
    RGSearchParams getCurrentSearchParams() const;
    void loadSearchParamsFromIni();  // Load search parameters from INI file
    
    // Update Rule 1 with combined search pattern
    void updateRule1WithCombinedPattern(const QString &pattern, const QString &addPattern) const;
    

    
    // Get current search thread for direct access
    QThread* getCurrentSearchThread() const;

private:
    // ===== DATA STRUCTURES =====
    QMap<QString, QVector<Match>> results_by_file;
    QMap<QString, FileMapping> file_mappings;
    QStringList found_file_paths;
    
    // ===== PERSISTENT SEARCH PARAMETERS =====
    RGSearchParams m_currentSearchParams;  // Persistent search parameters (never cleaned up)
    
    // Main window reference for accessing UI components
    QWidget *m_mainWindow;
    
    // ===== PROCESS MANAGEMENT =====
    QProcess *m_currentSearchProcess;  // Current ripgrep search process
    QProcess *m_currentMapProcess;     // Current ripgrep mapping process
    QThread *m_currentSearchThread;    // Current search thread for method3_async
    QMutex m_processMutex;            // Protect process access
    
    // ===== HELPER FUNCTIONS =====
    // Parse ripgrep output line and extract match data
    void parseStreamingLine(const QByteArray &line, QString *current_file, QMap<QString, QVector<Match>> *results);
    
    // Check if a line is a file heading (e.g., "C:/file.txt")
    bool isFileHeading(const QString &line);
    
    // Check if a line contains match data (e.g., "7:52:1071:RCB")
    bool isMatchLine(const QString &line);
    
    // Parse a match line and extract line, column, offset, and text
    Match parseMatchLine(const QString &line);

signals:
    // Signal emitted when async search completes with raw output
    void asyncSearchCompleted(const QString &rawOutput);
    
    // Signal emitted when async parsing completes
    void asyncParseCompleted(const QMap<QString, QVector<Match>> &results);
};

// Data structures
struct Match {
    long byte_offset;
    int line_number;
    long column;
    QString match;
    QString lineData;  // Full line content captured from ripgrep
};

struct FileMapping {
    QString file_path;
    QVector<long> line_offsets;
};

// Structure for search result display
struct SearchResult {
    QString file_path;
    int line_number;
    long column_offset;
    long line_start_offset;
    long line_end_offset;
    QString line_content;
    QString matched_text;
};

#endif // KSEARCHBUN_H 