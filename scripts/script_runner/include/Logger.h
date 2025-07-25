/**
 * @file Logger.h
 * @brief Comprehensive logging system for the Script Runner application
 * 
 * This file defines the Logger class which provides centralized logging
 * functionality with multiple severity levels and thread-safe operation.
 * 
 * @author GitHub Copilot
 * @date 2025-07-19
 * @version 1.0.0
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <memory>

/**
 * @brief Singleton logging class for application-wide debug and error reporting
 * 
 * The Logger class provides centralized logging functionality with configurable
 * severity levels, thread-safe operation, and multiple output destinations.
 * 
 * Usage:
 * @code
 * auto& logger = Logger::getInstance();
 * logger.setLevel(Logger::Level::DEBUG);
 * logger.info("Application starting");
 * @endcode
 * 
 * Or use convenience macros:
 * @code
 * LOG_INFO("Application starting");
 * LOG_ERROR("Something went wrong");
 * @endcode
 */
class Logger {
public:
    /**
     * @brief Logging severity levels
     */
    enum class Level {
        DEBUG,      ///< Detailed debugging information
        INFO,       ///< General informational messages
        WARNING,    ///< Warning messages for potential issues
        ERROR,      ///< Error messages for failures
        FATAL       ///< Fatal errors that cause application termination
    };

    /**
     * @brief Get the singleton Logger instance
     * 
     * @return Logger& Reference to the global logger instance
     */
    static Logger& getInstance();

    // Delete copy constructor and assignment operator (singleton pattern)
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Set the minimum logging level
     * 
     * Only messages at or above this level will be logged.
     * 
     * @param level Minimum logging level
     */
    void setLevel(Level level);

    /**
     * @brief Set the log file path
     * 
     * @param filename Path to log file (creates if doesn't exist)
     * 
     * @throws std::runtime_error If file cannot be created or opened
     */
    void setLogFile(const std::string& filename);

    /**
     * @brief Enable or disable console logging
     * 
     * @param enabled True to enable console output, false to disable
     */
    void setConsoleLogging(bool enabled);

    /**
     * @brief Log a message at the specified level
     * 
     * @param level Message severity level
     * @param message Message text to log
     */
    void log(Level level, const std::string& message);

    /**
     * @brief Log a debug message
     * 
     * @param message Debug message text
     */
    void debug(const std::string& message);

    /**
     * @brief Log an informational message
     * 
     * @param message Info message text
     */
    void info(const std::string& message);

    /**
     * @brief Log a warning message
     * 
     * @param message Warning message text
     */
    void warning(const std::string& message);

    /**
     * @brief Log an error message
     * 
     * @param message Error message text
     */
    void error(const std::string& message);

    /**
     * @brief Log a fatal error message
     * 
     * @param message Fatal error message text
     */
    void fatal(const std::string& message);

private:
    /**
     * @brief Private constructor for singleton pattern
     */
    Logger();

    /**
     * @brief Destructor - closes log file if open
     */
    ~Logger();

    Level m_level;              ///< Current minimum logging level
    std::ofstream m_logFile;    ///< Log file stream
    std::mutex m_logMutex;      ///< Mutex for thread-safe logging
    bool m_logToFile;           ///< Whether to log to file
    bool m_logToConsole;        ///< Whether to log to console

    /**
     * @brief Format a log message with timestamp and level
     * 
     * @param level Message severity level
     * @param message Message text
     * @return std::string Formatted message
     */
    std::string formatMessage(Level level, const std::string& message);

    /**
     * @brief Convert logging level to string representation
     * 
     * @param level Logging level to convert
     * @return std::string String representation of level
     */
    std::string levelToString(Level level);

    /**
     * @brief Check if a message should be logged based on current level
     * 
     * @param level Message level to check
     * @return true If message should be logged
     * @return false If message should be filtered out
     */
    bool shouldLog(Level level) const;
};

// Convenience macros for easier logging
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
#define LOG_FATAL(msg) Logger::getInstance().fatal(msg)

#endif // LOGGER_H
