/**
 * @file Exceptions.cpp
 * @brief Implementation of exception classes
 * 
 * This file contains the implementation of all exception classes
 * used throughout the Script Runner application.
 * 
 * @author GitHub Copilot
 * @date 2025-07-19
 * @version 1.0.0
 */

#include "../include/Exceptions.h"

// ScriptRunnerException implementation
ScriptRunnerException::ScriptRunnerException(const std::string& message) 
    : m_message(message) {
}

const char* ScriptRunnerException::what() const noexcept {
    return m_message.c_str();
}

// UIException implementation
UIException::UIException(const std::string& message) 
    : ScriptRunnerException("UI Error: " + message) {
}

// ProcessException implementation
ProcessException::ProcessException(const std::string& message) 
    : ScriptRunnerException("Process Error: " + message) {
}

// FileSystemException implementation
FileSystemException::FileSystemException(const std::string& message) 
    : ScriptRunnerException("File System Error: " + message) {
}

// ConfigurationException implementation
ConfigurationException::ConfigurationException(const std::string& message) 
    : ScriptRunnerException("Configuration Error: " + message) {
}
