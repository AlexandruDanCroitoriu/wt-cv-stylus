/**
 * @file Exceptions.h
 * @brief Exception class hierarchy for the Script Runner application
 * 
 * This file defines all exception classes used throughout the application,
 * providing a clear hierarchy for error handling and recovery.
 * 
 * @author GitHub Copilot
 * @date 2025-07-19
 * @version 1.0.0
 */

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>

/**
 * @brief Base exception class for all Script Runner errors
 * 
 * This serves as the base class for all application-specific exceptions,
 * providing a common interface for error handling.
 */
class ScriptRunnerException : public std::exception {
public:
    /**
     * @brief Construct exception with error message
     * 
     * @param message Error description
     */
    explicit ScriptRunnerException(const std::string& message);
    
    /**
     * @brief Get error message
     * 
     * @return const char* Error message string
     */
    const char* what() const noexcept override;

private:
    std::string m_message;  ///< Error message
};

/**
 * @brief Exception for UI-related errors
 * 
 * Thrown when terminal UI operations fail, such as ncurses initialization,
 * window creation, or rendering operations.
 */
class UIException : public ScriptRunnerException {
public:
    /**
     * @brief Construct UI exception with error message
     * 
     * @param message UI-specific error description
     */
    explicit UIException(const std::string& message);
};

/**
 * @brief Exception for process execution errors
 * 
 * Thrown when script execution fails, processes cannot be started,
 * or process management operations encounter errors.
 */
class ProcessException : public ScriptRunnerException {
public:
    /**
     * @brief Construct process exception with error message
     * 
     * @param message Process-specific error description
     */
    explicit ProcessException(const std::string& message);
};

/**
 * @brief Exception for file system access errors
 * 
 * Thrown when file or directory operations fail, such as script discovery,
 * log file creation, or configuration file access.
 */
class FileSystemException : public ScriptRunnerException {
public:
    /**
     * @brief Construct file system exception with error message
     * 
     * @param message File system-specific error description
     */
    explicit FileSystemException(const std::string& message);
};

/**
 * @brief Exception for configuration-related errors
 * 
 * Thrown when configuration parsing fails or invalid configuration
 * values are encountered.
 */
class ConfigurationException : public ScriptRunnerException {
public:
    /**
     * @brief Construct configuration exception with error message
     * 
     * @param message Configuration-specific error description
     */
    explicit ConfigurationException(const std::string& message);
};

#endif // EXCEPTIONS_H
