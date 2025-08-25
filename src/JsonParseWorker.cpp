#include "JsonParseWorker.h"
#include "logger.h"
#include "KSearch.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QElapsedTimer>
#include <QApplication>
#include <windows.h>
#include <psapi.h>
#include <QFileInfo>

JsonParseWorker::JsonParseWorker(QObject *parent)
    : QObject(parent)
{
}

void JsonParseWorker::parseJsonData(const QString &pattern, const QString &jsonData, const QString &searchPath)
{
    LOG_INFO("JsonParseWorker: START - parseJsonData");
    QElapsedTimer functionTimer;
    functionTimer.start();
    
    logMemoryUsage("parseJsonData - start");
    
    try {
        emit parsingStarted();
        
        // Split JSON data once at the top level
        QElapsedTimer splitTimer;
        splitTimer.start();
        
        QStringList lines = jsonData.split('\n', Qt::SkipEmptyParts);
        
        qint64 splitTime = splitTimer.elapsed();
        LOG_INFO("JsonParseWorker: Top-level split time: " + QString::number(splitTime) + " ms for " + QString::number(lines.size()) + " lines");
        
        // Parse RG summary first to get summary data and match count (pass split lines)
        LOG_INFO("JsonParseWorker: About to call parseRGSummary");
        int totalMatchedLines = 0;
        QString summaryText = parseRGSummary(lines, &totalMatchedLines);
        LOG_INFO("JsonParseWorker: parseRGSummary completed successfully");
        LOG_INFO("JsonParseWorker: Total matched lines: " + QString::number(totalMatchedLines));
        
        // Emit summary parsed signal
        emit summaryParsed(summaryText, totalMatchedLines);
        
                // Parse the actual JSON data (pass split lines)
        parseJsonDataInternal(lines, pattern, totalMatchedLines);
        
        // Explicit QStringList cleanup to force memory deallocation
        LOG_INFO("JsonParseWorker: Starting explicit QStringList cleanup");
        logMemoryUsage("Before QStringList cleanup");
        lines.clear();
        lines.squeeze();  // Qt equivalent of shrink_to_fit()
        logMemoryUsage("After QStringList cleanup");
        LOG_INFO("JsonParseWorker: QStringList cleanup completed");
        
        LOG_INFO("JsonParseWorker: parseJsonData completed successfully");
        LOG_INFO("JsonParseWorker: Total function time: " + QString::number(functionTimer.elapsed()) + " ms");
        
        logMemoryUsage("parseJsonData - end");
        
    } catch (const std::exception &e) {
        LOG_ERROR("JsonParseWorker: Exception in parseJsonData: " + QString(e.what()));
        emit parsingError(QString("Exception: %1").arg(e.what()));
    } catch (...) {
        LOG_ERROR("JsonParseWorker: Unknown exception in parseJsonData");
        emit parsingError("Unknown exception occurred");
    }
    
    LOG_INFO("JsonParseWorker: END - parseJsonData");
}



void JsonParseWorker::logMemoryUsage(const QString& stage)
{
    qint64 memoryUsage = getCurrentMemoryUsage();
    LOG_INFO("JsonParseWorker: Memory usage at " + stage + ": " + QString::number(memoryUsage) + " bytes (" + QString::number(memoryUsage / 1024 / 1024) + " MB)");
}

qint64 JsonParseWorker::getCurrentMemoryUsage() const
{
    // Get process memory usage on Windows
    HANDLE process = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
}

QString JsonParseWorker::parseRGSummary(const QStringList &lines, int* outMatchedLines)
{
    LOG_INFO("JsonParseWorker: START - parseRGSummary");
    QElapsedTimer functionTimer;
    functionTimer.start();
    
    try {
        // Get the last non-empty line from the pre-split data
        QElapsedTimer lastLineTimer;
        lastLineTimer.start();
        
        QString lastLine;
        
        if (!lines.isEmpty()) {
            lastLine = lines.last().trimmed();
        }
        
        qint64 lastLineTime = lastLineTimer.elapsed();
        LOG_INFO("JsonParseWorker: Last line extraction time: " + QString::number(lastLineTime) + " ms");
        LOG_INFO("JsonParseWorker: Last line length: " + QString::number(lastLine.length()) + " characters");
        
        QString summaryText;
        int totalMatchedLines = 0;
        
        if (!lastLine.isEmpty()) {
            LOG_INFO("JsonParseWorker: Last line: " + lastLine);

   

            if (!lastLine.isEmpty()) {
                QJsonParseError parseError;
                QJsonDocument doc = QJsonDocument::fromJson(lastLine.toUtf8(), &parseError);
                
                if (parseError.error == QJsonParseError::NoError) {
                    QJsonObject obj = doc.object();
                    QString type = obj[QString("type")].toString();
                    
                    if (type == "summary") {
                        QJsonObject dataObj = obj["data"].toObject();
                
                        // Extract summary information
                        QJsonObject elapsedTotalObj = dataObj["elapsed_total"].toObject();
                        QString elapsedTotalStr = elapsedTotalObj["human"].toString();
                        
                        QJsonObject statsObj = dataObj["stats"].toObject();
                        int matchedLines = statsObj["matched_lines"].toInt();
                        int searchesWithMatch = statsObj["searches_with_match"].toInt();
                                
                        totalMatchedLines = matchedLines;
                        
                        summaryText = QString("📊 Found %1 matches ( files: %2,  duration: %3 )")
                            .arg(matchedLines)
                            .arg(searchesWithMatch)
                            .arg(elapsedTotalStr);
                        
                        LOG_INFO("JsonParseWorker: Summary processed - " + summaryText);
                    } else {
                        LOG_WARNING("JsonParseWorker: Last line is not a summary, type: " + type);
                    }
                } else {
                    LOG_WARNING("JsonParseWorker: JSON parse error in last line: " + parseError.errorString());
                }
            } else {
                LOG_WARNING("JsonParseWorker: Last line is empty");
            }
        } else {
            LOG_WARNING("JsonParseWorker: Last line is empty or no valid JSON found");
        }
        
        if (outMatchedLines) {
            *outMatchedLines = totalMatchedLines;
        }
        
        LOG_INFO("JsonParseWorker: parseRGSummary completed successfully");
        LOG_INFO("JsonParseWorker: Total function time: " + QString::number(functionTimer.elapsed()) + " ms");
        
        return summaryText;
        
    } catch (const std::exception &e) {
        LOG_ERROR("JsonParseWorker: Exception in parseRGSummary: " + QString(e.what()));
        throw;
    } catch (...) {
        LOG_ERROR("JsonParseWorker: Unknown exception in parseRGSummary");
        throw;
    }
    
    LOG_INFO("JsonParseWorker: END - parseRGSummary");
}

void JsonParseWorker::parseJsonDataInternal(const QStringList &lines, const QString &pattern, int totalMatchedLines)
{
    LOG_INFO("JsonParseWorker: START - parseJsonDataInternal");
    QElapsedTimer functionTimer;
    functionTimer.start();
    
    logMemoryUsage("parseJsonDataInternal - start");
    
    try {
        LOG_INFO("JsonParseWorker: Processing " + QString::number(lines.size()) + " JSON lines");
        
        int totalMatches = 0;
        int filesWithMatches = 0;
        int lineNumberCounter = 0;
        int lastProgressUpdate = 0;
        
        QMap<QString, QPair<QString, int>> fileStats; // filePath -> (elapsed_time, matched_lines)
        QMap<QString, QString> fileDisplayTexts; // filePath -> displayText
        
        for (int i = 0; i < lines.size(); ++i) {

            if (g_currentSearchState == SearchState::STOP) {
                LOG_INFO("JsonParseWorker: Stopping parsing");
                break;
            }

            const QString &line = lines[i];
            if (line.trimmed().isEmpty()) continue;
            
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &parseError);
            
            if (parseError.error != QJsonParseError::NoError) {
                LOG_WARNING("JsonParseWorker: JSON parse error in line " + QString::number(i) + ": " + parseError.errorString());
                // Explicit cleanup for malformed JSON
                doc = QJsonDocument();
                continue;
            }
            
            QJsonObject obj = doc.object();
            QString type = obj[QString("type")].toString();
            
            if (type == "begin") {
                QString filePath = obj[QString("data")][QString("path")][QString("text")].toString();
                double elapsedTime = obj[QString("data")][QString("elapsed_total_s")].toDouble();
                
                // Initialize file stats
                fileStats[filePath] = qMakePair(QString::number(elapsedTime, 'f', 3), 0);
                filesWithMatches++;
                
                // Create file display text
                QString displayText = createFileDisplayText(filePath, QString::number(elapsedTime, 'f', 3), 0);
                fileDisplayTexts[filePath] = displayText;
                
                // Emit file item created signal
                emit fileItemCreated(filePath, displayText);
                
            } else if (type == "match") {
                QString filePath = obj[QString("data")][QString("path")][QString("text")].toString();
                int lineNumber = obj[QString("data")][QString("line_number")].toInt();
                QString lineText = obj[QString("data")][QString("lines")][QString("text")].toString().trimmed();
                
                totalMatches++;
                lineNumberCounter++;
                
                // Update file stats
                if (fileStats.contains(filePath)) {
                    fileStats[filePath].second++;
                }
                
                // Create match display text
                QString matchDisplayText = createMatchDisplayText(filePath, lineNumber, lineText);
                
                // Emit match item created signal
                emit matchItemCreated(filePath, lineNumber, lineText);
                
                // Update progress every 5% (0, 5, 10, 15, ..., 95, 100)
                if (totalMatchedLines > 0) {
                    int currentProgress = (lineNumberCounter * 100) / totalMatchedLines;
                    int progressStep = (currentProgress / 5) * 5;  // Round down to nearest 5%
                    
                    if (progressStep > lastProgressUpdate) {
                        lastProgressUpdate = progressStep;
                        // Send progress percentage (0-100) instead of raw counts
                        emit parsingProgress(progressStep, filesWithMatches);
                    }
                }
                
            } else if (type == "end") {
                QString filePath = obj[QString("data")][QString("path")][QString("text")].toString();
                
                LOG_INFO("JsonParseWorker: Processing 'end' type for file: " + filePath);
                
                // Extract elapsed time and matched lines from "end" type JSON
                QJsonObject statsObj = obj[QString("data")][QString("stats")].toObject();
                QString elapsedTime = statsObj[QString("elapsed")][QString("human")].toString();
                int matchedLines = statsObj[QString("matched_lines")].toInt();
                
                LOG_INFO("JsonParseWorker: End stats - " + filePath + " has " + QString::number(matchedLines) + " matches in " + elapsedTime);
                
                // Emit file stats updated signal with correct data from "end" type
                emit fileStatsUpdated(filePath, elapsedTime, matchedLines);
            }
            
            // Explicit JSON object cleanup after processing each line
            obj = QJsonObject();
            doc = QJsonDocument();
        }
        
        LOG_INFO("JsonParseWorker: Processing completed - " + QString::number(totalMatches) + " matches in " + QString::number(filesWithMatches) + " files");
        LOG_INFO("JsonParseWorker: Total function time: " + QString::number(functionTimer.elapsed()) + " ms");
        
        logMemoryUsage("parseJsonDataInternal - end");
        
        // Process any pending Qt events to help with memory cleanup
        QApplication::processEvents();
        logMemoryUsage("After processEvents");
        
        // Emit completion signal
        emit parsingCompleted(totalMatches, filesWithMatches);
        
    } catch (const std::exception &e) {
        LOG_ERROR("JsonParseWorker: Exception in parseJsonDataInternal: " + QString(e.what()));
        throw;
    } catch (...) {
        LOG_ERROR("JsonParseWorker: Unknown exception in parseJsonDataInternal");
        throw;
    }
    
    LOG_INFO("JsonParseWorker: END - parseJsonDataInternal");
}

QString JsonParseWorker::createFileDisplayText(const QString &filePath, const QString &elapsedTime, int matchedLines)
{
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    QString directory = fileInfo.absolutePath();
    
    QString displayText = QString("📄 %1 (%2 matches, %3s)").arg(fileName).arg(matchedLines).arg(elapsedTime);
    return displayText;
}

QString JsonParseWorker::createMatchDisplayText(const QString &filePath, int lineNumber, const QString &lineText)
{
    QString displayText = QString("Line %1: %2").arg(lineNumber).arg(lineText);
    return displayText;
}
