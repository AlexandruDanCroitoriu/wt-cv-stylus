/**
 * @file Logger.cpp
 * @brief Implementation of the Logger class
 * 
 * This file contains the implementation of the singleton Logger class
 * providing thread-safe logging functionality.
 * 
 * @author GitHub Copilot
 * @date 2025-07-19
 * @version 1.0.0
 */

#include "../include/Logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <cstdlib>

Logger::Logger() 
    : m_level(Level::INFO)
    , m_logToFile(false)
    , m_logToConsole(true) {
    
    // Check environment variable for log level
    const char* envLevel = std::getenv("SCRIPT_RUNNER_LOG_LEVEL");
    if (envLevel) {
        std::string levelStr(envLevel);
        if (levelStr == "DEBUG") {
            m_level = Level::DEBUG;
        } else if (levelStr == "INFO") {
            m_level = Level::INFO;
        } else if (levelStr == "WARNING") {
            m_level = Level::WARNING;
        } else if (levelStr == "ERROR") {
            m_level = Level::ERROR;
        } else if (levelStr == "FATAL") {
            m_level = Level::FATAL;
        }
    }
    
    // Check environment variable for log file
    const char* envLogFile = std::getenv("SCRIPT_RUNNER_LOG_FILE");
    if (envLogFile) {
        try {
            setLogFile(envLogFile);
        } catch (const std::exception& e) {
            // Fallback to console only if file setup fails
            std::cerr << "Warning: Failed to set log file from environment: " 
                      << e.what() << std::endl;
        }
    }
}

Logger::~Logger() {
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLevel(Level level) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_level = level;
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    
    // Close existing file if open
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
    
    // Open new log file
    m_logFile.open(filename, std::ios::app);
    if (!m_logFile.is_open()) {
        throw std::runtime_error("Failed to open log file: " + filename);
    }
    
    m_logToFile = true;
    
    // Log the file opening
    log(Level::INFO, "Log file opened: " + filename);
}

void Logger::setConsoleLogging(bool enabled) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_logToConsole = enabled;
}

void Logger::log(Level level, const std::string& message) {
    if (!shouldLog(level)) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_logMutex);
    
    std::string formattedMessage = formatMessage(level, message);
    
    // Log to console if enabled
    if (m_logToConsole) {
        if (level >= Level::ERROR) {
            std::cerr << formattedMessage << std::endl;
        } else {
            std::cout << formattedMessage << std::endl;
        }
    }
    
    // Log to file if enabled and file is open
    if (m_logToFile && m_logFile.is_open()) {
        m_logFile << formattedMessage << std::endl;
        m_logFile.flush(); // Ensure immediate write for debugging
    }
}

void Logger::debug(const std::string& message) {
    log(Level::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(Level::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(Level::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(Level::ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(Level::FATAL, message);
}

std::string Logger::formatMessage(Level level, const std::string& message) {
    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
    ss << "[" << levelToString(level) << "] ";
    ss << message;
    
    return ss.str();
}

std::string Logger::levelToString(Level level) {
    switch (level) {
        case Level::DEBUG:   return "DEBUG";
        case Level::INFO:    return "INFO ";
        case Level::WARNING: return "WARN ";
        case Level::ERROR:   return "ERROR";
        case Level::FATAL:   return "FATAL";
        default:             return "UNKNOWN";
    }
}

bool Logger::shouldLog(Level level) const {
    return level >= m_level;
}
