#include "logger.h"
#include <QDebug>
#include <QTextCursor>
#include <QApplication>

Logger::Logger(QObject* parent)
    : QObject(parent)
    , m_logWidget(nullptr)
    , m_initialized(false)
{
}

Logger::~Logger()
{
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

void Logger::initialize(QTextEdit* logWidget, const QString& logFilePath)
{
    m_logWidget = logWidget;
    m_logFilePath = logFilePath;
    
    // Open log file if path is provided
    if (!m_logFilePath.isEmpty()) {
        m_logFile.setFileName(m_logFilePath);
        // Use Truncate to clear any existing content and ensure clean start
        if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            qWarning() << "Failed to open log file:" << m_logFilePath;
        }
    }
    
    m_initialized = true;
    
    // Log initialization
    info("Logger initialized");
}

void Logger::log(const QString& message, LogLevel level)
{
    if (!m_initialized) {
        qWarning() << "Logger not initialized, message:" << message;
        return;
    }
    
    QString formattedMessage = formatMessage(message, level);
    
    writeToConsole(formattedMessage);
    writeToFile(formattedMessage);
    writeToWidget(formattedMessage);
}

void Logger::debug(const QString& message)
{
    log(message, DEBUG);
}

void Logger::info(const QString& message)
{
    log(message, INFO);
}

void Logger::warning(const QString& message)
{
    log(message, WARNING);
}

void Logger::error(const QString& message)
{
    log(message, ERROR);
}

void Logger::setLogWidget(QTextEdit* widget)
{
    m_logWidget = widget;
}

void Logger::setLogFilePath(const QString& path)
{
    m_logFilePath = path;
    
    // Close existing file if open
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
    
    // Open new log file
    if (!m_logFilePath.isEmpty()) {
        m_logFile.setFileName(m_logFilePath);
        // Use Truncate to clear any existing content and ensure clean start
        if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            qWarning() << "Failed to open log file:" << m_logFilePath;
        }
    }
}

void Logger::writeToConsole(const QString& message)
{
    std::cout << message.toStdString() << std::endl;
}

void Logger::writeToFile(const QString& message)
{
    if (m_logFile.isOpen()) {
        QTextStream out(&m_logFile);
        out << message << "\n";
        m_logFile.flush();
    }
}

void Logger::writeToWidget(const QString& message)
{
    if (m_logWidget) {
        m_logWidget->append(message);
        
        // Auto-scroll to bottom
        QTextCursor cursor = m_logWidget->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_logWidget->setTextCursor(cursor);
        
        // Process events to update the widget immediately
        QApplication::processEvents();
    }
}

QString Logger::formatMessage(const QString& message, LogLevel level)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString levelStr = levelToString(level);
    return QString("[%1] [%2] %3").arg(timestamp, levelStr, message);
}

QString Logger::levelToString(LogLevel level)
{
    switch (level) {
        case DEBUG:   return "DEBUG";
        case INFO:    return "INFO";
        case WARNING: return "WARN";
        case ERROR:   return "ERROR";
        default:      return "INFO";
    }
} 