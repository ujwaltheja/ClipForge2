#include "logger.h"
#include <android/log.h>
#include <cstdarg>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace clipforge {
namespace utils {

// ============================================================================
// Logger Implementation
// ============================================================================

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

bool Logger::initialize(const std::string& logFilePath, LogLevel level, bool useLogcat) {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_logLevel = level;
    m_useLogcat = useLogcat;
    m_logFilePath = logFilePath;

    // Open log file if specified
    if (!logFilePath.empty()) {
        m_logFile = std::make_unique<std::ofstream>(logFilePath, std::ios::app);
        if (!m_logFile->is_open()) {
            logInternal(LogLevel::ERROR, "Failed to open log file: " + logFilePath);
            return false;
        }
    }

    logInternal(LogLevel::INFO, "Logger initialized");
    return true;
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile) {
        m_logFile->flush();
        m_logFile->close();
        m_logFile = nullptr;
    }
}

void Logger::setLogLevel(LogLevel level) {
    m_logLevel = level;
}

void Logger::logVerbose(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string message = formatString(format, args);
    va_end(args);
    logInternal(LogLevel::VERBOSE, message);
}

void Logger::logDebug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string message = formatString(format, args);
    va_end(args);
    logInternal(LogLevel::DEBUG, message);
}

void Logger::logInfo(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string message = formatString(format, args);
    va_end(args);
    logInternal(LogLevel::INFO, message);
}

void Logger::logWarning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string message = formatString(format, args);
    va_end(args);
    logInternal(LogLevel::WARNING, message);
}

void Logger::logError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string message = formatString(format, args);
    va_end(args);
    logInternal(LogLevel::ERROR, message);
}

void Logger::logCritical(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string message = formatString(format, args);
    va_end(args);
    logInternal(LogLevel::CRITICAL, message);
}

void Logger::log(LogLevel level, const std::string& message) {
    logInternal(level, message);
}

void Logger::logMetric(const std::string& name, double value, const std::string& unit) {
    std::string message = "[METRIC] " + name + ": " + std::to_string(value);
    if (!unit.empty()) {
        message += " " + unit;
    }
    logInternal(LogLevel::INFO, message);
}

void Logger::logException(const std::string& message, const std::exception& exception) {
    std::string fullMessage = message + ": " + exception.what();
    logInternal(LogLevel::ERROR, fullMessage);
}

int64_t Logger::getLogFileSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_logFile) return 0;
    // Get file size
    std::streampos size = m_logFile->tellp();
    return static_cast<int64_t>(size);
}

bool Logger::rotateLogFile() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFilePath.empty()) {
        return false;
    }

    if (m_logFile) {
        m_logFile->close();
    }

    // Rename current log file
    std::string backupPath = m_logFilePath + ".bak";
    std::rename(m_logFilePath.c_str(), backupPath.c_str());

    // Open new log file
    m_logFile = std::make_unique<std::ofstream>(m_logFilePath, std::ios::app);
    return m_logFile->is_open();
}

bool Logger::clearLogFile() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile) {
        m_logFile->close();
    }

    m_logFile = std::make_unique<std::ofstream>(m_logFilePath, std::ios::trunc);
    return m_logFile->is_open();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::VERBOSE: return "VERBOSE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        case LogLevel::NONE: return "NONE";
        default: return "UNKNOWN";
    }
}

LogLevel Logger::stringToLevel(const std::string& str) {
    if (str == "VERBOSE") return LogLevel::VERBOSE;
    if (str == "DEBUG") return LogLevel::DEBUG;
    if (str == "INFO") return LogLevel::INFO;
    if (str == "WARNING") return LogLevel::WARNING;
    if (str == "ERROR") return LogLevel::ERROR;
    if (str == "CRITICAL") return LogLevel::CRITICAL;
    return LogLevel::INFO;  // Default
}

std::string Logger::getLevelColorCode(LogLevel level) {
    // ANSI color codes
    switch (level) {
        case LogLevel::VERBOSE: return "\033[36m";    // Cyan
        case LogLevel::DEBUG: return "\033[32m";      // Green
        case LogLevel::INFO: return "\033[37m";       // White
        case LogLevel::WARNING: return "\033[33m";    // Yellow
        case LogLevel::ERROR: return "\033[31m";      // Red
        case LogLevel::CRITICAL: return "\033[35m";   // Magenta
        default: return "\033[0m";                    // Reset
    }
}

std::string Logger::formatString(const char* format, va_list ap) {
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, ap);
    return std::string(buffer);
}

void Logger::logInternal(LogLevel level, const std::string& message) {
    // Check if this level should be logged
    if (level < m_logLevel) {
        return;
    }

    std::string formatted = "[" + getTimestamp() + "] [" + levelToString(level) + "] " + message;

    // Call custom handler if set
    if (m_customHandler) {
        m_customHandler(level, formatted);
    }

    // Log to logcat
    if (m_useLogcat) {
        logToLogcat(level, "ClipForge", message);
    }

    // Log to file
    if (m_logFile && m_logFile->is_open()) {
        (*m_logFile) << formatted << "\n";
        m_logFile->flush();
    }

    // Update statistics
    m_messageCount++;
    if (level == LogLevel::ERROR) m_errorCount++;
    if (level == LogLevel::WARNING) m_warningCount++;
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
       << "." << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string Logger::getThreadId() {
    // Use a simple thread-local counter instead of std::thread::id
    // to avoid linker issues with Android NDK
    static thread_local uint32_t threadId = 0;
    static std::atomic<uint32_t> nextThreadId{1};

    if (threadId == 0) {
        threadId = nextThreadId.fetch_add(1, std::memory_order_relaxed);
    }

    std::stringstream ss;
    ss << "T" << threadId;
    return ss.str();
}

void Logger::logToLogcat(LogLevel level, const std::string& tag,
                        const std::string& message) {
    int androidLevel;
    switch (level) {
        case LogLevel::VERBOSE: androidLevel = ANDROID_LOG_VERBOSE; break;
        case LogLevel::DEBUG: androidLevel = ANDROID_LOG_DEBUG; break;
        case LogLevel::INFO: androidLevel = ANDROID_LOG_INFO; break;
        case LogLevel::WARNING: androidLevel = ANDROID_LOG_WARN; break;
        case LogLevel::ERROR: androidLevel = ANDROID_LOG_ERROR; break;
        case LogLevel::CRITICAL: androidLevel = ANDROID_LOG_FATAL; break;
        default: androidLevel = ANDROID_LOG_INFO; break;
    }

    __android_log_print(androidLevel, tag.c_str(), "%s", message.c_str());
}

// ============================================================================
// LogScope Implementation
// ============================================================================

LogScope::LogScope(const std::string& scopeName)
    : m_scopeName(scopeName), m_startTime(getHighResTime()) {
    LOG_DEBUG(">>> Entering scope: %s", scopeName.c_str());
}

LogScope::~LogScope() {
    int64_t endTime = getHighResTime();
    double elapsedMs = static_cast<double>(endTime - m_startTime) / 1000.0;
    LOG_DEBUG("<<< Exiting scope: %s (%.2f ms)", m_scopeName.c_str(), elapsedMs);
}

int64_t LogScope::getHighResTime() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()).count();
}

} // namespace utils
} // namespace clipforge
