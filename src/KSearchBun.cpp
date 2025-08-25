#include "KSearchBun.h"
#include "mainwindow.h"
#include <QProcess>
#include <QThread>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QMetaObject>
#include <QMutexLocker>
#include "logger.h"
#include <QApplication>
#include <QTextEdit>
#include <QFile>
#include <QTextStream>
#include <QListWidget> // Added for QListWidget
#include <QColor> // Added for QColor
#include <QFileInfo> // Added for QFileInfo








// Asynchronous version of parse function
void KSearchBun::parseRGMainResults_async(const QString &allOutput)
{
    LOG_INFO("KSearchBun: ===THREAD=== parseRGMainResults_async (Asynchronous) <<<<<STARTed<<<<<");
    
    // Create a worker thread to run the parsing
    QThread *parseThread = QThread::create([this, allOutput]() {
        // Run the synchronous parsing in the background thread
        QMap<QString, QVector<Match>> results = this->parseRGMainResults(allOutput);
        
        // Emit the signal when parsing completes
        emit this->asyncParseCompleted(results);
        
        LOG_INFO("KSearchBun: ===THREAD=== parseRGMainResults_async (Asynchronous) >>>>>ENDed>>>>>");
    });
    QObject::connect(parseThread, &QThread::finished, parseThread, &QObject::deleteLater);
    // Start the thread
    parseThread->start();
}

// Parse ripgrep output into search results
QMap<QString, QVector<Match>> KSearchBun::parseRGMainResults(const QString &allOutput)
{
    LOG_INFO("KSearchBun: ===THREAD=== parseRGMainResults <<<<<STARTed<<<<<");
    
    // Clear previous results
    results_by_file.clear();
    found_file_paths.clear();
    
    // ===== STEP 3: SPLIT OUTPUT INTO LINES =====
    QStringList lines = allOutput.split('\n', Qt::SkipEmptyParts);
    LOG_INFO("KSearchBun: parseRGMainResults - Split into " + QString::number(lines.size()) + " lines");
    
    // ===== STEP 4: PARSE LINES =====
    QString currentFile;
    
    for (const QString &line : lines) {
        
        // Check if this is a file heading (like "C:/file1.txt")
        if (isFileHeading(line)) {
            // This is a file heading - update current file context
            currentFile = line;
            found_file_paths.append(currentFile);
            LOG_INFO("KSearchBun: parseRGMainResults - Found file: " + currentFile);
            
            // ===== STEP 5: MAP FILE FOR THIS PATH =====
 //           KMapFile_Synch(currentFile);
            
        } else if (isMatchLine(line) && !currentFile.isEmpty()) {
            // This is a match line - parse the match data
            Match match = parseMatchLine(line);
            results_by_file[currentFile].append(match);
        }
    }
    
    // ===== STEP 6: DISPLAY RESULTS =====
    LOG_INFO("KSearchBun: parseRGMainResults - Found matches in " + QString::number(results_by_file.size()) + " files");
    
    LOG_INFO("KSearchBun: ===THREAD=== parseRGMainResults >>>>>ENDed>>>>>");
    
    return results_by_file;
}







// Check if a line is a file heading (like "C:/file1.txt")
bool KSearchBun::isFileHeading(const QString &line)
{
    QRegularExpression heading_regex(R"(^[A-Za-z]:[\\/][^:]+$)");
    return heading_regex.match(line).hasMatch();
}

// Check if a line is a match line (like "123:45:67:match_text")
bool KSearchBun::isMatchLine(const QString &line)
{
    QRegularExpression match_regex(R"(^(\d+):(\d+):(\d+):(.+)$)");
    // in regex that means:
    
    return match_regex.match(line).hasMatch();
}

// Parse a match line into a Match structure
// Ripgrep output format: line_number:column:byte_offset:full_line_content
// Example: "7:52:1071:This is the full line content with the match" -> line=7, column=52, offset=1071, text="match", lineData="This is the full line content with the match"
Match KSearchBun::parseMatchLine(const QString &line)
{
    QRegularExpression match_regex(R"(^(\d+):(\d+):(\d+):(.+)$)");
    QRegularExpressionMatch match = match_regex.match(line);
    
    Match m;
    if (match.hasMatch()) {
        // Group 1: line_number (first number)
        m.line_number = match.captured(1).toInt();
        // Group 2: column (second number)  
        m.column = match.captured(2).toLong();
        // Group 3: byte_offset (third number)
        m.byte_offset = match.captured(3).toLong();
        // Group 4: full line content (everything after third colon)
        QString fullLineContent = match.captured(4);
        m.lineData = fullLineContent;
        
        // Extract the matched text (this would be the actual match, but since we removed --only-matching,
        // we'll use a placeholder or the first part of the line)
        if (!fullLineContent.isEmpty()) {
            // For now, use the first word or a reasonable portion as the match
            QStringList words = fullLineContent.split(' ', Qt::SkipEmptyParts);
            m.match = words.isEmpty() ? fullLineContent.left(20) : words.first();
        } else {
            m.match = "No content";
        }
        
    } else {
        LOG_WARNING("KSearchBun: parseMatchLine - No match for line: '" + line + "'");
    }
    
    return m;
}


///////////============================================================================================================
///////////============================================================================================================
///////////============================================================================================================
///////////============================================================================================================
///////////============================================================================================================
///////////============================================================================================================
///////////============================================================================================================
///////////============================================================================================================
///////////============================================================================================================
///////////============================================================================================================
///////////============================================================================================================
///////////============================================================================================================
///////////============================================================================================================

// ===== ESSENTIAL FUNCTIONS THAT ARE STILL BEING CALLED =====

// Constructor
KSearchBun::KSearchBun(QObject *parent)
    : QObject(parent)
    , m_mainWindow(nullptr)
    , m_currentSearchProcess(nullptr)
    , m_currentMapProcess(nullptr)
    , m_currentSearchThread(nullptr)
{
    LOG_INFO("KSearchBun: Constructor called");
    
    // Load search parameters from INI file to initialize persistent parameters
    loadSearchParamsFromIni();
}

// Set main window reference
void KSearchBun::setMainWindow(QWidget *mainWindow)
{
    m_mainWindow = mainWindow;
    LOG_INFO("KSearchBun: Main window reference set");
}

// Clear all data
void KSearchBun::clearAllData()
{
    try {
        LOG_INFO("KSearchBun: Starting clearAllData");
        
        // Clear data structures
        results_by_file.clear();
        found_file_paths.clear();
        file_mappings.clear();
        
        LOG_INFO("KSearchBun: Data structures cleared");
        
        // Clean up processes with defensive programming
        QMutexLocker locker(&m_processMutex);
        
        // Clean up search process
        if (m_currentSearchProcess) {
            try {
                LOG_INFO("KSearchBun: Cleaning up search process");
                if (m_currentSearchProcess->state() != QProcess::NotRunning) {
                    LOG_INFO("KSearchBun: Terminating search process");
                    m_currentSearchProcess->terminate();
                    if (!m_currentSearchProcess->waitForFinished(5000)) {
                        LOG_WARNING("KSearchBun: Force killing search process");
                        m_currentSearchProcess->kill();
                        m_currentSearchProcess->waitForFinished(2000);
                    }
                }
                delete m_currentSearchProcess;
                m_currentSearchProcess = nullptr;
                LOG_INFO("KSearchBun: Search process cleaned up successfully");
            } catch (const std::exception& e) {
                LOG_ERROR("KSearchBun: Exception cleaning up search process: " + QString(e.what()));
            } catch (...) {
                LOG_ERROR("KSearchBun: Unknown exception cleaning up search process");
            }
        }
        
        // Clean up map process
        if (m_currentMapProcess) {
            try {
                LOG_INFO("KSearchBun: Cleaning up map process");
                if (m_currentMapProcess->state() != QProcess::NotRunning) {
                    LOG_INFO("KSearchBun: Terminating map process");
                    m_currentMapProcess->terminate();
                    if (!m_currentMapProcess->waitForFinished(5000)) {
                        LOG_WARNING("KSearchBun: Force killing map process");
                        m_currentMapProcess->kill();
                        m_currentMapProcess->waitForFinished(2000);
                    }
                }
                delete m_currentMapProcess;
                m_currentMapProcess = nullptr;
                LOG_INFO("KSearchBun: Map process cleaned up successfully");
            } catch (const std::exception& e) {
                LOG_ERROR("KSearchBun: Exception cleaning up map process: " + QString(e.what()));
            } catch (...) {
                LOG_ERROR("KSearchBun: Unknown exception cleaning up map process");
            }
        }
        
        // Clean up search thread - SKIP THIS FOR NOW
        // The thread should have already finished naturally from the previous search
        // Trying to delete it causes crashes when it's in an invalid state
        if (m_currentSearchThread) {
            LOG_INFO("KSearchBun: Skipping search thread cleanup to prevent crash");
            // Don't delete the thread - let it be cleaned up naturally
            m_currentSearchThread = nullptr;
        }
        
        LOG_INFO("KSearchBun: All data cleared successfully");
        
    } catch (const std::exception& e) {
        LOG_ERROR("KSearchBun: Exception in clearAllData: " + QString(e.what()));
    } catch (...) {
        LOG_ERROR("KSearchBun: Unknown exception in clearAllData");
    }
}

// KMap function - gets file path and returns list of line offsets
QVector<long> KSearchBun::KMap(const QString &file_path)
{
    QElapsedTimer timer;
    timer.start();
    
    LOG_INFO("KSearchBun: ===THREAD=== KMap for file: " + file_path + " <<<<<STARTed<<<<<");
    
    QVector<long> line_offsets;
    
    // Check if file exists
    QFileInfo fileInfo(file_path);
    if (!fileInfo.exists()) {
        LOG_ERROR("KSearchBun: KMap - File does not exist: " + file_path);
        LOG_INFO("KSearchBun: ===THREAD=== KMap for file: " + file_path + " >>>>>ENDed>>>>>");
        return line_offsets;
    }
    
    // Build ripgrep command for line mapping
        QStringList arguments;
    arguments << "-n";                    // Show line numbers
    arguments << "--threads" << "0";      // Use all available threads
    arguments << "--mmap";                // Use memory-mapped I/O
    arguments << "--column";              // Show column numbers
    arguments << "--byte-offset";         // Show byte offsets
    arguments << "--heading";             // Show filename as heading
    arguments << "--no-filename";         // Don't show filename in output
    arguments << "--no-line-number";      // Don't show line numbers in output
    arguments << "--only-matching";       // Show only matching parts
    arguments << ".";                     // Search for any character (matches all lines)
    arguments << file_path;               // File to search
    
    // Create and start process
    QMutexLocker locker(&m_processMutex);
    
    // Clean up any existing mapping process
    if (m_currentMapProcess) {
        delete m_currentMapProcess;
    }
    
    m_currentMapProcess = new QProcess();
    m_currentMapProcess->start("lib\\rg.exe", arguments);
    
    if (!m_currentMapProcess->waitForStarted()) {
        LOG_ERROR("KSearchBun: KMap - Failed to start ripgrep process");
        return line_offsets;
    }
    
    // Wait for completion
    if (!m_currentMapProcess->waitForFinished(30000)) { // 30 second timeout
        LOG_ERROR("KSearchBun: KMap - Process timed out");
        m_currentMapProcess->kill();
        return line_offsets;
    }
    
    // Get output
    QByteArray allOutput = m_currentMapProcess->readAllStandardOutput();
    QByteArray errorOutput = m_currentMapProcess->readAllStandardError();
    int exitCode = m_currentMapProcess->exitCode();
    
    LOG_INFO("KSearchBun: KMap - Ripgrep exit code: " + QString::number(exitCode));
    
    if (exitCode != 0 && !errorOutput.isEmpty()) {
        LOG_WARNING("KSearchBun: KMap - Ripgrep errors: " + errorOutput);
    }
    
    LOG_INFO("KSearchBun: KMap - Received " + QString::number(allOutput.size()) + " characters from ripgrep");
    LOG_INFO("KSearchBun: KMap - Error output size: " + QString::number(errorOutput.size()) + " characters");
    
    // Parse output to extract line offsets
    QString output = QString::fromUtf8(allOutput);
    QStringList parts = output.split(':', Qt::SkipEmptyParts);
    
    LOG_INFO("KSearchBun: KMap - Split into " + QString::number(parts.size()) + " parts by ':'");
    
    for (const QString &part : parts) {
        bool ok;
        long offset = part.toLong(&ok);
        if (ok && offset > 0) {
            line_offsets.append(offset);
    } else {
            LOG_WARNING("KSearchBun: KMap - Invalid offset: " + part);
        }
    }
    
    LOG_INFO("KSearchBun: KMap - Extracted " + QString::number(line_offsets.size()) + " line offsets");
    LOG_INFO("KSearchBun: ===THREAD=== KMap for file: " + file_path + " >>>>>ENDed>>>>>");
    LOG_INFO("KSearchBun: TIMING: KMap took " + QString::number(timer.elapsed() / 1000.0, 'f', 3) + " sec");
    
    return line_offsets;
}

// Get full ripgrep command
QString KSearchBun::getFullRipgrepCommand(const RGSearchParams &params) const
{
    // Update persistent search parameters
    const_cast<KSearchBun*>(this)->updateSearchParams(params);
    
        QStringList arguments;
    arguments << "lib\\rg.exe";
    arguments << "-a";                    // Search binary files
    arguments << "-n";                    // Show line numbers
    arguments << "--threads" << "0";      // Use all available threads
    arguments << "--mmap";                // Use memory-mapped I/O
    arguments << "--column";              // Show column numbers
    arguments << "--byte-offset";         // Show byte offsets
    arguments << "--stats";               // Show statistics
    arguments << "--heading";             // Show filename as heading
    arguments << "--json";                // Always output in JSON format
    
    // Add pattern flags
    if (params.fixed_string) {
        arguments << "-F";                // Fixed string mode
    }
    
    // Add case sensitivity flags (mutually exclusive)
    if (params.case_sensitive) {
        arguments << "-s";                // Case sensitive
    } else if (params.ignore_case) {
        arguments << "-i";                // Ignore case
    } else if (params.smart_case) {
        arguments << "-S";                // Smart case
    }
    
    // Add include/exclude patterns
    if (!params.incl_exclude.isEmpty()) {
        QStringList patterns = params.incl_exclude.split(',', Qt::SkipEmptyParts);
        for (const QString &pattern : patterns) {
            QString trimmedPattern = pattern.trimmed();
            if (trimmedPattern.startsWith("!")) {
                // Exclude pattern (starts with !)
                arguments << "-g" << QString("!%1").arg(trimmedPattern.mid(1));
                    } else {
                // Include pattern
                arguments << "-g" << trimmedPattern;
            }
        }
    }
    
    // Add pattern (combine main pattern with additional pattern using OR operator)
    arguments << "-e";
    QString full_pattern = params.pattern;
    if (!params.add_pattern.isEmpty()) {
        full_pattern += "|" + params.add_pattern;
    }
    
    // Update Rule 1 with the combined pattern
    updateRule1WithCombinedPattern(params.pattern, params.add_pattern);
    
    // Pass pattern directly to ripgrep (no quoting needed for regex patterns)
    arguments << full_pattern;
    
    // Add path
    arguments << params.path;


    
    return arguments.join(' ');
}

// ===== METHOD 3: SYNCHRONOUS IMPLEMENTATION (SIMPLE AND RELIABLE) =====
// This method does everything synchronously in one function - no signals, no threads, just simple execution
QString KSearchBun::K_RGresults_method3(const RGSearchParams &params)
{
    QElapsedTimer timer;
    timer.start();
    
    // Use persistent search parameters instead of passed parameters
    RGSearchParams currentParams = m_currentSearchParams;
    
    LOG_INFO("KSearchBun: ===THREAD=== K_RGresults_method3 (Synchronous) for path: " + currentParams.path + " <<<<<STARTed<<<<<");
    LOG_DEBUG("KSearchBun: Method3 - 112 Using persistent RGSearchParams:");
    LOG_DEBUG("  path: " + currentParams.path);
    LOG_DEBUG("  pattern: " + currentParams.pattern);
    LOG_DEBUG("  fixed_string: " + QString(currentParams.fixed_string ? "true" : "false"));
    LOG_DEBUG("  add_pattern: " + currentParams.add_pattern);
    LOG_DEBUG("  case_sensitive: " + QString(currentParams.case_sensitive ? "true" : "false"));
    LOG_DEBUG("  ignore_case: " + QString(currentParams.ignore_case ? "true" : "false"));
    LOG_DEBUG("  smart_case: " + QString(currentParams.smart_case ? "true" : "false"));
    LOG_DEBUG("  incl_exclude: " + currentParams.incl_exclude);
    LOG_DEBUG("  keep_files_in_cache: " + QString(currentParams.keep_files_in_cache ? "true" : "false"));
    LOG_DEBUG("  highlight_color: " + currentParams.highlight_color.name());
    
    // Clear previous results
    results_by_file.clear();
    found_file_paths.clear();
    file_mappings.clear();
        
        // ===== STEP 1: BUILD RIPGREP COMMAND =====
    QStringList arguments;
    arguments << "-a";                    // Search binary files
    arguments << "-n";                    // Show line numbers
    arguments << "--threads" << "0";      // Use all available threads
    arguments << "--mmap";                // Use memory-mapped I/O
    arguments << "--column";              // Show column numbers
    arguments << "--byte-offset";         // Show byte offsets
    arguments << "--stats";               // Show statistics
    arguments << "--heading";             // Show filename as heading
    arguments << "--json";                // Always output in JSON format
    
    // Add pattern flags
    if (currentParams.fixed_string) {
        arguments << "-F";                // Fixed string mode
    }
    
    // Add case sensitivity flags (mutually exclusive)
    if (currentParams.case_sensitive) {
        arguments << "-s";                // Case sensitive
    } else if (currentParams.ignore_case) {
        arguments << "-i";                // Ignore case
    } else if (currentParams.smart_case) {
        arguments << "-S";                // Smart case
    }
    
    // Add include/exclude patterns
    if (!currentParams.incl_exclude.isEmpty()) {
        QStringList patterns = currentParams.incl_exclude.split(',', Qt::SkipEmptyParts);
        for (const QString &pattern : patterns) {
            QString trimmedPattern = pattern.trimmed();
            if (trimmedPattern.startsWith("!")) {
                // Exclude pattern (starts with !)
                arguments << "-g" << QString("!%1").arg(trimmedPattern.mid(1));
                // Exclude pattern (starts with !)
            } else {
                // Include pattern
                arguments << "-g" << trimmedPattern;
            }
        }
    }
    
    // Add pattern (pass directly to ripgrep)
    arguments << "-e";
    QString full_pattern = currentParams.pattern;
    if (!currentParams.add_pattern.isEmpty()) {
        full_pattern += "|" + currentParams.add_pattern;
    }
    
    // Update Rule 1 with the combined pattern
    updateRule1WithCombinedPattern(currentParams.pattern, currentParams.add_pattern);
    
    // Pass pattern directly to ripgrep (no quoting needed for regex patterns)
    arguments << full_pattern;
    
    // Add search path
    arguments << currentParams.path;
    
    LOG_INFO("KSearchBun: Method3 - Ripgrep command: lib\\rg.exe " + arguments.join(' '));
    
    // ===== STEP 2: EXECUTE RIPGREP AND GET ALL OUTPUT =====
    QMutexLocker locker(&m_processMutex);
    
    // Clean up any existing process
    if (m_currentSearchProcess) {
        delete m_currentSearchProcess;
    }
    
    m_currentSearchProcess = new QProcess();
    m_currentSearchProcess->start("lib\\rg.exe", arguments);
    m_currentSearchProcess->waitForFinished();
    
    // Get all output as QString
    QString allOutput = QString::fromUtf8(m_currentSearchProcess->readAllStandardOutput());
    QString errorOutput = QString::fromUtf8(m_currentSearchProcess->readAllStandardError());
    
    if (!errorOutput.isEmpty()) {
        LOG_WARNING("KSearchBun: Method3 - Ripgrep errors: " + errorOutput);
    }
    
    LOG_INFO("KSearchBun: Method3 ENDed - Received " + QString::number(allOutput.size()) + " characters from ripgrep");
    
    // Return the raw output for parsing in separate function
    return allOutput;
}

// Asynchronous version of method 3
void KSearchBun::K_RGresults_method3_async(const RGSearchParams &params)
{
    LOG_INFO("KSearchBun: ===THREAD=== K_RGresults_method3_async (Asynchronous) for path: " + params.path + " <<<<<STARTed<<<<<");
    
    // Clean up any existing search thread
    QMutexLocker locker(&m_processMutex);
    if (m_currentSearchThread) {
        delete m_currentSearchThread;
    }
    
    // Create a thread to run the synchronous method and ensure it is deleted on finish
    m_currentSearchThread = QThread::create([this]() {
        QString rawOutput = this->K_RGresults_method3(m_currentSearchParams);
        emit this->asyncSearchCompleted(rawOutput);
    });
    QObject::connect(m_currentSearchThread, &QThread::finished, m_currentSearchThread, &QObject::deleteLater);
    m_currentSearchThread->start();
    
    LOG_INFO("KSearchBun: ===THREAD=== K_RGresults_method3_async (Asynchronous) for path: " + params.path + " >>>>>ENDed>>>>>");
}



QThread* KSearchBun::getCurrentSearchThread() const
{
    return m_currentSearchThread;
}

// Persistent search parameters management
void KSearchBun::updateSearchParams(const RGSearchParams &params)
{
    m_currentSearchParams = params;
    LOG_INFO("KSearchBun: Updated persistent search parameters");
    LOG_INFO("  Path: " + params.path);
    LOG_INFO("  Pattern: " + params.pattern);
    LOG_INFO("  Fixed String: " + QString(params.fixed_string ? "Yes" : "No"));
    LOG_INFO("  Add Pattern: " + params.add_pattern);
    LOG_INFO("  Case Sensitive: " + QString(params.case_sensitive ? "Yes" : "No"));
    LOG_INFO("  Ignore Case: " + QString(params.ignore_case ? "Yes" : "No"));
    LOG_INFO("  Smart Case: " + QString(params.smart_case ? "Yes" : "No"));
    LOG_INFO("  Include/Exclude: " + params.incl_exclude);
    LOG_INFO("  Keep Files in Cache: " + QString(params.keep_files_in_cache ? "Yes" : "No"));
    LOG_INFO("  Highlight Color: " + params.highlight_color.name());
}

RGSearchParams KSearchBun::getCurrentSearchParams() const
{
    return m_currentSearchParams;
}

void KSearchBun::updateRule1WithCombinedPattern(const QString &pattern, const QString &addPattern) const
{
    // Combine pattern and add_pattern with | separator
    QString combinedPattern = pattern;
    if (!addPattern.isEmpty()) {
        combinedPattern += "|" + addPattern;
    }
    
    LOG_INFO("KSearchBun: Updating Rule 1 with combined pattern: '" + combinedPattern + "'");
    
    // Update Rule 1 in the INI file
    QSettings settings("app.ini", QSettings::IniFormat);
    settings.beginGroup("HighlightDialog");
    
    // Ensure Rule 1 exists by setting it
    settings.beginGroup("Rule0");  // Rule 0 in GUI is Rule 1 in logic
    settings.setValue("Pattern", combinedPattern);
    settings.setValue("Color", QColor(255, 100, 100));  // Light Red for Rule 1
    settings.setValue("Enabled", true);
    settings.endGroup();
    
    // Update RuleCount if needed
    int currentRuleCount = settings.value("RuleCount", 0).toInt();
    if (currentRuleCount < 1) {
        settings.setValue("RuleCount", 1);
    }
    
    settings.endGroup();
    
    LOG_INFO("KSearchBun: Rule 1 updated in INI file with pattern: '" + combinedPattern + "'");
    
    // Also update the in-memory rules in MainWindow if available
    if (m_mainWindow) {
        MainWindow* mainWindow = qobject_cast<MainWindow*>(m_mainWindow);
        if (mainWindow && mainWindow->getKSearch()) {
            // Use the public method to update Rule 1
            mainWindow->getKSearch()->updateRule1Pattern(combinedPattern);
        }
    }
}

void KSearchBun::loadSearchParamsFromIni()
{
    QSettings settings("app.ini", QSettings::IniFormat);
    settings.beginGroup("RGSearch");
    
    // Load search parameters from INI file
    m_currentSearchParams.fixed_string = settings.value("LastFixedString", false).toBool();
    m_currentSearchParams.add_pattern = settings.value("LastAddPattern", "").toString();
    m_currentSearchParams.case_sensitive = settings.value("LastCaseSensitive", false).toBool();
    m_currentSearchParams.ignore_case = settings.value("LastIgnoreCase", false).toBool();
    m_currentSearchParams.smart_case = settings.value("LastSmartCase", false).toBool();
    m_currentSearchParams.incl_exclude = settings.value("LastInclExclude", "").toString();
    m_currentSearchParams.keep_files_in_cache = settings.value("LastKeepFilesInCache", false).toBool();
    m_currentSearchParams.highlight_color = settings.value("LastHighlightColor", QColor(130, 130, 130)).value<QColor>();
    
    // Set default values for path and pattern (these come from main window UI)
    m_currentSearchParams.path = "";
    m_currentSearchParams.pattern = "";
    
    settings.endGroup();
    
    LOG_INFO("KSearchBun: Loaded search parameters from INI file");
    LOG_INFO("  Fixed String: " + QString(m_currentSearchParams.fixed_string ? "Yes" : "No"));
    LOG_INFO("  Add Pattern: " + m_currentSearchParams.add_pattern);
    LOG_INFO("  Case Sensitive: " + QString(m_currentSearchParams.case_sensitive ? "Yes" : "No"));
    LOG_INFO("  Ignore Case: " + QString(m_currentSearchParams.ignore_case ? "Yes" : "No"));
    LOG_INFO("  Smart Case: " + QString(m_currentSearchParams.smart_case ? "Yes" : "No"));
    LOG_INFO("  Include/Exclude: " + m_currentSearchParams.incl_exclude);
    LOG_INFO("  Keep Files in Cache: " + QString(m_currentSearchParams.keep_files_in_cache ? "Yes" : "No"));
    LOG_INFO("  Highlight Color: " + m_currentSearchParams.highlight_color.name());
}





