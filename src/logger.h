#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QTextEdit>
#include <QObject>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <iostream>

class Logger : public QObject
{
    Q_OBJECT

public:
    enum LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static Logger& instance();
    
    // Initialize the logger
    void initialize(QTextEdit* logWidget = nullptr, const QString& logFilePath = QString());
    
    // Logging methods
    void log(const QString& message, LogLevel level = INFO);
    void debug(const QString& message);
    void info(const QString& message);
    void warning(const QString& message);
    void error(const QString& message);
    
    // Set log widget and file path
    void setLogWidget(QTextEdit* widget);
    void setLogFilePath(const QString& path);
    
    // Check if logger is initialized
    bool isInitialized() const { return m_initialized; }

private:
    Logger(QObject* parent = nullptr);
    ~Logger();
    
    // Singleton pattern
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void writeToConsole(const QString& message);
    void writeToFile(const QString& message);
    void writeToWidget(const QString& message);
    QString formatMessage(const QString& message, LogLevel level);
    QString levelToString(LogLevel level);
    
    QTextEdit* m_logWidget;
    QString m_logFilePath;
    bool m_initialized;
    QFile m_logFile;
};

// Convenience macros for easy logging
#define LOG_DEBUG(msg) Logger::instance().debug(msg)
#define LOG_INFO(msg) Logger::instance().info(msg)
#define LOG_WARNING(msg) Logger::instance().warning(msg)
#define LOG_ERROR(msg) Logger::instance().error(msg)

// Legacy macro for backward compatibility
#define LOG_TS qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")

#endif // LOGGER_H 