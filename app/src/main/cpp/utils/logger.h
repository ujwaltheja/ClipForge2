#ifndef CLIPFORGE_LOGGER_H
#define CLIPFORGE_LOGGER_H

/**
 * @file logger.h
 * @brief Unified logging system for ClipForge
 *
 * Provides logging to both Android Logcat and file output with
 * multiple log levels and thread-safe operations.
 */

#include <string>
#include <memory>
#include <fstream>
#include <sstream>
#include <atomic>
#include <mutex>
#include <functional>
#include <cstdint>

namespace clipforge {
namespace utils {

/**
 * @enum LogLevel
 * @brief Severity levels for logging
 */
enum class LogLevel {
    VERBOSE = 0,   // Detailed diagnostic information
    DEBUG = 1,     // Development debugging
    INFO = 2,      // General information
    WARNING = 3,   // Warning messages
    ERROR = 4,     // Error conditions
    CRITICAL = 5,  // Critical failures
    NONE = 6,      // Disable all logging
};

/**
 * @class Logger
 * @brief Central logging facility for the application
 *
 * Provides singleton logger instance with support for:
 * - Multiple log levels
 * - Both console (logcat) and file output
 * - Custom log handlers
 * - Thread-safe operations
 * - Performance metrics logging
 *
 * Usage:
 * @code
 * Logger& logger = Logger::getInstance();
 * logger.setLogLevel(LogLevel::DEBUG);
 * logger.logInfo("Video clip added: %d ms", 5000);
 * logger.logError("Failed to load file: %s", errorMsg.c_str());
 * @endcode
 */
class Logger {
public:
    using LogHandler = std::function<void(LogLevel, const std::string&)>;

    /**
     * @brief Get the singleton logger instance
     * @return Reference to logger
     */
    static Logger& getInstance();

    /**
     * @brief Initialize logger with configuration
     * @param logFilePath Path where to write log file (empty = no file output)
     * @param level Minimum log level to output
     * @param useLogcat true to also log to Android logcat
     * @return true if initialization successful
     */
    bool initialize(const std::string& logFilePath = "", LogLevel level = LogLevel::INFO,
                   bool useLogcat = true);

    /**
     * @brief Shutdown logger and close log file
     */
    void shutdown();

    /**
     * @brief Set minimum log level
     * @param level Logs below this level will be ignored
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief Get current minimum log level
     * @return Current level
     */
    [[nodiscard]] LogLevel getLogLevel() const { return m_logLevel; }

    /**
     * @brief Set custom log handler (called for every log message)
     * @param handler Function to call with (level, message)
     */
    void setLogHandler(LogHandler handler) { m_customHandler = handler; }

    /**
     * @brief Remove custom log handler
     */
    void clearLogHandler() { m_customHandler = nullptr; }

    // ===== Logging Methods =====

    /**
     * @brief Log verbose message
     * @param format Printf-style format string
     * @param ... Variable arguments
     */
    void logVerbose(const char* format, ...);

    /**
     * @brief Log debug message
     * @param format Printf-style format string
     * @param ... Variable arguments
     */
    void logDebug(const char* format, ...);

    /**
     * @brief Log informational message
     * @param format Printf-style format string
     * @param ... Variable arguments
     */
    void logInfo(const char* format, ...);

    /**
     * @brief Log warning message
     * @param format Printf-style format string
     * @param ... Variable arguments
     */
    void logWarning(const char* format, ...);

    /**
     * @brief Log error message
     * @param format Printf-style format string
     * @param ... Variable arguments
     */
    void logError(const char* format, ...);

    /**
     * @brief Log critical message
     * @param format Printf-style format string
     * @param ... Variable arguments
     */
    void logCritical(const char* format, ...);

    /**
     * @brief Log raw message string (no formatting)
     * @param level Log level
     * @param message Message to log
     */
    void log(LogLevel level, const std::string& message);

    /**
     * @brief Log performance metric
     * @param name Metric name
     * @param value Numeric value
     * @param unit Unit of measurement (e.g., "ms", "MB")
     */
    void logMetric(const std::string& name, double value, const std::string& unit = "");

    /**
     * @brief Log exception details
     * @param message Context message
     * @param exception Exception to log
     */
    void logException(const std::string& message, const std::exception& exception);

    // ===== Statistics =====

    /**
     * @brief Get total number of messages logged
     * @return Message count
     */
    [[nodiscard]] int64_t getMessageCount() const { return m_messageCount; }

    /**
     * @brief Get number of error messages
     * @return Error count
     */
    [[nodiscard]] int64_t getErrorCount() const { return m_errorCount; }

    /**
     * @brief Get number of warning messages
     * @return Warning count
     */
    [[nodiscard]] int64_t getWarningCount() const { return m_warningCount; }

    /**
     * @brief Get log file size
     * @return Size in bytes
     */
    [[nodiscard]] int64_t getLogFileSize() const;

    /**
     * @brief Get log file path
     * @return Current log file path
     */
    [[nodiscard]] const std::string& getLogFilePath() const { return m_logFilePath; }

    /**
     * @brief Rotate log file (rename current and start new)
     * @return true if rotation successful
     */
    bool rotateLogFile();

    /**
     * @brief Clear log file
     * @return true if cleared successfully
     */
    bool clearLogFile();

    // ===== Level Conversion =====

    /**
     * @brief Convert LogLevel to string
     * @param level The level
     * @return String representation (e.g., "ERROR")
     */
    [[nodiscard]] static std::string levelToString(LogLevel level);

    /**
     * @brief Convert string to LogLevel
     * @param str String to convert
     * @return Corresponding LogLevel (defaults to INFO)
     */
    [[nodiscard]] static LogLevel stringToLevel(const std::string& str);

    /**
     * @brief Get color code for log level (ANSI)
     * @param level The level
     * @return ANSI color code string
     */
    [[nodiscard]] static std::string getLevelColorCode(LogLevel level);

    // ===== Utility =====

    /**
     * @brief Format a string using printf-style formatting
     * @param format Format string
     * @param ap Variable argument list
     * @return Formatted string
     */
    [[nodiscard]] static std::string formatString(const char* format, va_list ap);

private:
    // Singleton
    Logger() = default;
    ~Logger() { shutdown(); }

    // Prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // State
    LogLevel m_logLevel = LogLevel::INFO;
    bool m_useLogcat = true;
    std::string m_logFilePath;
    std::unique_ptr<std::ofstream> m_logFile;
    mutable std::mutex m_mutex;
    LogHandler m_customHandler;

    // Statistics
    std::atomic<int64_t> m_messageCount{0};
    std::atomic<int64_t> m_errorCount{0};
    std::atomic<int64_t> m_warningCount{0};

    /**
     * @brief Internal logging implementation
     * @param level Log level
     * @param message Message to log
     */
    void logInternal(LogLevel level, const std::string& message);

    /**
     * @brief Format timestamp for log output
     * @return Formatted timestamp string
     */
    [[nodiscard]] static std::string getTimestamp();

    /**
     * @brief Get thread ID for logging
     * @return Thread ID as string
     */
    [[nodiscard]] static std::string getThreadId();

    /**
     * @brief Log to Android logcat
     * @param level Log level
     * @param tag Tag for logcat
     * @param message Message to log
     */
    static void logToLogcat(LogLevel level, const std::string& tag,
                           const std::string& message);
};

/**
 * @class LogScope
 * @brief RAII helper for scope-based logging
 *
 * Automatically logs entry/exit of a scope with timing information.
 *
 * Usage:
 * @code
 * {
 *     LogScope scope("ProcessFrame");
 *     // ... do processing ...
 * } // Automatically logs exit and elapsed time
 * @endcode
 */
class LogScope {
public:
    /**
     * @brief Enter a logging scope
     * @param scopeName Name of the scope
     */
    explicit LogScope(const std::string& scopeName);

    /**
     * @brief Exit scope (destructor)
     *
     * Logs elapsed time and closes scope.
     */
    ~LogScope();

    // Prevent copying
    LogScope(const LogScope&) = delete;
    LogScope& operator=(const LogScope&) = delete;

private:
    std::string m_scopeName;
    int64_t m_startTime;

    /**
     * @brief Get high-resolution timestamp
     * @return Microseconds since epoch
     */
    static int64_t getHighResTime();
};

// ===== Convenience Macros =====

#ifdef CLIPFORGE_DEBUG_BUILD
    #define LOG_VERBOSE(...) ::clipforge::utils::Logger::getInstance().logVerbose(__VA_ARGS__)
    #define LOG_DEBUG(...) ::clipforge::utils::Logger::getInstance().logDebug(__VA_ARGS__)
    #define LOG_SCOPE(name) ::clipforge::utils::LogScope __log_scope(name)
#else
    #define LOG_VERBOSE(...) ((void)0)
    #define LOG_DEBUG(...) ((void)0)
    #define LOG_SCOPE(name) ((void)0)
#endif

#define LOG_INFO(...) ::clipforge::utils::Logger::getInstance().logInfo(__VA_ARGS__)
#define LOG_WARNING(...) ::clipforge::utils::Logger::getInstance().logWarning(__VA_ARGS__)
#define LOG_ERROR(...) ::clipforge::utils::Logger::getInstance().logError(__VA_ARGS__)
#define LOG_CRITICAL(...) ::clipforge::utils::Logger::getInstance().logCritical(__VA_ARGS__)

} // namespace utils
} // namespace clipforge

#endif // CLIPFORGE_LOGGER_H
