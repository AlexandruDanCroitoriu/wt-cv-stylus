/**
 * @file ProcessManager.cpp
 * @brief Implementation of the ProcessManager class
 * 
 * This file contains the implementation of process management functionality
 * including script discovery, execution, and output capture.
 * 
 * @author GitHub Copilot
 * @date 2025-07-19
 * @version 1.0.0
 */

#include "../include/ProcessManager.h"
#include "../include/Logger.h"
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

// ScriptProcess implementation
ProcessManager::ScriptProcess::ScriptProcess() 
    : pid(-1)
    , stdout_fd(-1)
    , stderr_fd(-1)
    , status(ProcessStatus::IDLE)
    , shouldStop(false)
    , exitCode(-1) {
}

ProcessManager::ScriptProcess::~ScriptProcess() {
    // Ensure cleanup in destructor
    if (outputThread && outputThread->joinable()) {
        shouldStop = true;
        outputThread->join();
    }
}

void ProcessManager::ScriptProcess::reset() {
    // Stop output thread if running
    if (outputThread && outputThread->joinable()) {
        shouldStop = true;
        outputThread->join();
    }
    
    // Reset all fields
    pid = -1;
    stdout_fd = -1;
    stderr_fd = -1;
    status = ProcessStatus::IDLE;
    command.clear();
    shouldStop = false;
    exitCode = -1;
    
    // Clear output buffer
    std::lock_guard<std::mutex> lock(outputMutex);
    while (!outputBuffer.empty()) {
        outputBuffer.pop();
    }
}

// ProcessManager implementation
ProcessManager::ProcessManager() {
    LOG_INFO("ProcessManager initialized");
    
    // Initialize process slots
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        m_processes[i] = std::make_unique<ScriptProcess>();
    }
}

ProcessManager::~ProcessManager() {
    LOG_INFO("ProcessManager shutting down");
    
    // Terminate all running processes
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        if (isRunning(i)) {
            terminateScript(i);
        }
    }
    
    LOG_INFO("ProcessManager shutdown complete");
}

std::vector<std::string> ProcessManager::discoverScripts(const std::string& directory) {
    LOG_DEBUG("Discovering scripts in directory: " + directory);
    
    std::vector<std::string> scripts;
    
    try {
        // Check if directory exists
        if (!std::filesystem::exists(directory)) {
            LOG_WARNING("Script directory does not exist: " + directory);
            return scripts;
        }
        
        if (!std::filesystem::is_directory(directory)) {
            LOG_WARNING("Path is not a directory: " + directory);
            return scripts;
        }
        
        // Recursively scan directory
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string filePath = entry.path().string();
                
                if (isExecutable(filePath)) {
                    scripts.push_back(filePath);
                    LOG_DEBUG("Found executable script: " + filePath);
                }
            }
        }
        
        // Sort scripts for consistent ordering
        std::sort(scripts.begin(), scripts.end());
        
        LOG_INFO("Discovered " + std::to_string(scripts.size()) + " executable scripts");
        
    } catch (const std::filesystem::filesystem_error& e) {
        throw FileSystemException("Failed to scan directory '" + directory + "': " + e.what());
    }
    
    return scripts;
}

bool ProcessManager::isExecutable(const std::string& filePath) {
    struct stat statBuf;
    
    if (stat(filePath.c_str(), &statBuf) != 0) {
        return false;
    }
    
    // Check if it's a regular file and has execute permission
    return S_ISREG(statBuf.st_mode) && (statBuf.st_mode & S_IXUSR);
}

bool ProcessManager::startScript(const std::string& scriptPath, int paneIndex) {
    validatePaneIndex(paneIndex);
    
    LOG_INFO("Starting script: " + scriptPath + " in pane " + std::to_string(paneIndex));
    
    auto& process = m_processes[paneIndex];
    
    // Check if pane is already in use
    if (process->status != ProcessStatus::IDLE) {
        LOG_WARNING("Pane " + std::to_string(paneIndex) + " is already in use");
        return false;
    }
    
    // Reset process state
    process->reset();
    process->status = ProcessStatus::STARTING;
    
    try {
        // Build command
        process->command = buildCommand(scriptPath);
        LOG_DEBUG("Executing command: " + process->command);
        
        // Set up pipes for stdout and stderr
        int stdout_pipe[2], stderr_pipe[2];
        if (!setupPipes(stdout_pipe, stderr_pipe)) {
            throw ProcessException("Failed to create pipes");
        }
        
        // Fork process
        pid_t pid = fork();
        
        if (pid == -1) {
            // Fork failed
            closePipe(stdout_pipe[0]);
            closePipe(stdout_pipe[1]);
            closePipe(stderr_pipe[0]);
            closePipe(stderr_pipe[1]);
            throw ProcessException("Failed to fork process: " + std::string(strerror(errno)));
        }
        
        if (pid == 0) {
            // Child process
            
            // Close read ends of pipes
            close(stdout_pipe[0]);
            close(stderr_pipe[0]);
            
            // Redirect stdout and stderr to pipes
            dup2(stdout_pipe[1], STDOUT_FILENO);
            dup2(stderr_pipe[1], STDERR_FILENO);
            
            // Close write ends (now duplicated)
            close(stdout_pipe[1]);
            close(stderr_pipe[1]);
            
            // Execute script using shell
            execl("/bin/sh", "sh", "-c", process->command.c_str(), nullptr);
            
            // If we get here, exec failed
            // Write error to stderr pipe (which will be captured)
            const char* errorMsg = "Failed to execute script\n";
            write(STDERR_FILENO, errorMsg, strlen(errorMsg));
            _exit(1);
        } else {
            // Parent process
            
            // Close write ends of pipes
            close(stdout_pipe[1]);
            close(stderr_pipe[1]);
            
            // Store process information
            process->pid = pid;
            process->stdout_fd = stdout_pipe[0];
            process->stderr_fd = stderr_pipe[0];
            process->startTime = std::chrono::steady_clock::now();
            process->status = ProcessStatus::RUNNING;
            
            // Make pipes non-blocking
            fcntl(process->stdout_fd, F_SETFL, O_NONBLOCK);
            fcntl(process->stderr_fd, F_SETFL, O_NONBLOCK);
            
            // Start output capture thread
            process->outputThread = std::make_unique<std::thread>(
                &ProcessManager::outputReaderThread, this, paneIndex);
            
            LOG_INFO("Script started successfully with PID: " + std::to_string(pid));
            return true;
        }
        
    } catch (const std::exception& e) {
        process->status = ProcessStatus::ERROR;
        LOG_ERROR("Failed to start script: " + std::string(e.what()));
        return false;
    }
}

void ProcessManager::terminateScript(int paneIndex) {
    validatePaneIndex(paneIndex);
    
    auto& process = m_processes[paneIndex];
    
    if (process->status == ProcessStatus::IDLE) {
        LOG_WARNING("No script running in pane " + std::to_string(paneIndex));
        return;
    }
    
    LOG_INFO("Terminating script in pane " + std::to_string(paneIndex) + 
             " (PID: " + std::to_string(process->pid) + ")");
    
    // Signal output thread to stop
    process->shouldStop = true;
    
    // Check if process has already finished naturally
    if (process->status == ProcessStatus::FINISHED || process->status == ProcessStatus::ERROR) {
        LOG_DEBUG("Process already finished, skipping kill signal");
    } else if (process->pid > 0) {
        // Only try to kill if process is still running
        if (isProcessAlive(process->pid)) {
            killProcess(process->pid);
        } else {
            LOG_DEBUG("Process already dead, skipping kill signal");
        }
    }
    
    // Close file descriptors
    closePipe(process->stdout_fd);
    closePipe(process->stderr_fd);
    
    // Wait for output thread to finish
    if (process->outputThread && process->outputThread->joinable()) {
        process->outputThread->join();
    }
    
    // Update status
    process->status = ProcessStatus::TERMINATED;
    
    LOG_INFO("Script terminated in pane " + std::to_string(paneIndex));
    
    // Only add delay if we actually had to kill a running process
    if (process->pid > 0 && isProcessAlive(process->pid)) {
        // Reset process after a brief delay to allow UI to show termination status
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    process->reset();
}

bool ProcessManager::isRunning(int paneIndex) {
    validatePaneIndex(paneIndex);
    
    auto& process = m_processes[paneIndex];
    ProcessStatus status = process->status;
    
    // Quick check if process has died but status hasn't been updated yet
    if ((status == ProcessStatus::STARTING || status == ProcessStatus::RUNNING) && 
        process->pid > 0 && !isProcessAlive(process->pid)) {
        // Process died but status not updated yet - update it now
        process->status = ProcessStatus::FINISHED;
        LOG_DEBUG("Process " + std::to_string(process->pid) + " detected as finished in isRunning check");
        return false;
    }
    
    return status == ProcessStatus::STARTING || status == ProcessStatus::RUNNING;
}

std::string ProcessManager::getNewOutput(int paneIndex) {
    validatePaneIndex(paneIndex);
    
    auto& process = m_processes[paneIndex];
    std::lock_guard<std::mutex> lock(process->outputMutex);
    
    if (process->outputBuffer.empty()) {
        return "";
    }
    
    std::stringstream ss;
    while (!process->outputBuffer.empty()) {
        ss << process->outputBuffer.front();
        process->outputBuffer.pop();
    }
    
    return ss.str();
}

void ProcessManager::clearOutput(int paneIndex) {
    validatePaneIndex(paneIndex);
    
    auto& process = m_processes[paneIndex];
    std::lock_guard<std::mutex> lock(process->outputMutex);
    
    while (!process->outputBuffer.empty()) {
        process->outputBuffer.pop();
    }
    
    LOG_DEBUG("Cleared output buffer for pane " + std::to_string(paneIndex));
}

ProcessStatus ProcessManager::getStatus(int paneIndex) {
    validatePaneIndex(paneIndex);
    return m_processes[paneIndex]->status;
}

std::chrono::seconds ProcessManager::getRuntime(int paneIndex) {
    validatePaneIndex(paneIndex);
    
    auto& process = m_processes[paneIndex];
    
    if (!isRunning(paneIndex)) {
        return std::chrono::seconds(0);
    }
    
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
        now - process->startTime);
    
    return duration;
}

std::string ProcessManager::getCommand(int paneIndex) {
    validatePaneIndex(paneIndex);
    return m_processes[paneIndex]->command;
}

// Private method implementations

void ProcessManager::validatePaneIndex(int paneIndex) const {
    if (paneIndex < 0 || paneIndex >= MAX_PROCESSES) {
        throw std::invalid_argument("Invalid pane index: " + std::to_string(paneIndex) + 
                                   " (must be 0-" + std::to_string(MAX_PROCESSES - 1) + ")");
    }
}

void ProcessManager::outputReaderThread(int paneIndex) {
    auto& process = m_processes[paneIndex];
    
    LOG_DEBUG("Output reader thread started for pane " + std::to_string(paneIndex));
    
    char buffer[4096];
    fd_set readSet;
    struct timeval timeout;
    
    while (!process->shouldStop) {
        FD_ZERO(&readSet);
        FD_SET(process->stdout_fd, &readSet);
        FD_SET(process->stderr_fd, &readSet);
        
        timeout.tv_sec = 0;
        timeout.tv_usec = 50000; // Reduced to 50ms timeout for more responsiveness
        
        int maxFd = std::max(process->stdout_fd, process->stderr_fd);
        int selectResult = select(maxFd + 1, &readSet, nullptr, nullptr, &timeout);
        
        if (selectResult > 0) {
            // Read from stdout
            if (FD_ISSET(process->stdout_fd, &readSet)) {
                ssize_t bytesRead = read(process->stdout_fd, buffer, sizeof(buffer) - 1);
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    
                    std::lock_guard<std::mutex> lock(process->outputMutex);
                    process->outputBuffer.push(std::string(buffer));
                    
                    // Limit buffer size
                    while (process->outputBuffer.size() > MAX_BUFFER_SIZE) {
                        process->outputBuffer.pop();
                    }
                } else if (bytesRead == 0) {
                    // EOF on stdout - process likely finished
                    LOG_DEBUG("EOF on stdout for pane " + std::to_string(paneIndex));
                }
            }
            
            // Read from stderr
            if (FD_ISSET(process->stderr_fd, &readSet)) {
                ssize_t bytesRead = read(process->stderr_fd, buffer, sizeof(buffer) - 1);
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    
                    std::lock_guard<std::mutex> lock(process->outputMutex);
                    process->outputBuffer.push(std::string(buffer));
                    
                    // Limit buffer size
                    while (process->outputBuffer.size() > MAX_BUFFER_SIZE) {
                        process->outputBuffer.pop();
                    }
                } else if (bytesRead == 0) {
                    // EOF on stderr - process likely finished
                    LOG_DEBUG("EOF on stderr for pane " + std::to_string(paneIndex));
                }
            }
        }
        
        // Check if process is still alive
        if (process->pid > 0 && !isProcessAlive(process->pid)) {
            // Process has terminated
            int status;
            if (waitpid(process->pid, &status, WNOHANG) > 0) {
                process->exitCode = WEXITSTATUS(status);
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                    process->status = ProcessStatus::FINISHED;
                } else {
                    process->status = ProcessStatus::ERROR;
                }
            }
            break;
        }
    }
    
    LOG_DEBUG("Output reader thread finished for pane " + std::to_string(paneIndex));
}

std::string ProcessManager::determineScriptType(const std::string& filePath) {
    // Get file extension
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "executable"; // No extension, assume binary executable
    }
    
    std::string extension = filePath.substr(dotPos);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    // Map extensions to script types
    if (extension == ".py") return "python";
    if (extension == ".sh") return "shell";
    if (extension == ".bash") return "shell";
    if (extension == ".js") return "javascript";
    if (extension == ".rb") return "ruby";
    if (extension == ".pl") return "perl";
    
    return "executable";
}

std::string ProcessManager::buildCommand(const std::string& scriptPath) {
    std::string scriptType = determineScriptType(scriptPath);
    
    if (scriptType == "python") {
        return "python3 \"" + scriptPath + "\"";
    } else if (scriptType == "shell") {
        return "bash \"" + scriptPath + "\"";
    } else if (scriptType == "javascript") {
        return "node \"" + scriptPath + "\"";
    } else if (scriptType == "ruby") {
        return "ruby \"" + scriptPath + "\"";
    } else if (scriptType == "perl") {
        return "perl \"" + scriptPath + "\"";
    } else {
        // Executable binary
        return "\"" + scriptPath + "\"";
    }
}

bool ProcessManager::isProcessAlive(pid_t pid) {
    if (pid <= 0) return false;
    
    // Send signal 0 to check if process exists
    return kill(pid, 0) == 0;
}

bool ProcessManager::killProcess(pid_t pid) {
    if (pid <= 0) return false;
    
    LOG_DEBUG("Attempting to terminate process " + std::to_string(pid));
    
    // Check if process is already dead
    if (!isProcessAlive(pid)) {
        LOG_DEBUG("Process " + std::to_string(pid) + " already terminated");
        return true;
    }
    
    // First try SIGTERM for graceful shutdown
    if (kill(pid, SIGTERM) == 0) {
        // Wait for graceful termination with shorter initial checks
        for (int i = 0; i < 10; ++i) { // Check first 1 second with 100ms intervals
            if (!isProcessAlive(pid)) {
                LOG_DEBUG("Process " + std::to_string(pid) + " terminated gracefully");
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // If still alive after 1 second, continue with longer timeout for stubborn processes
        for (int i = 0; i < (TERMINATION_TIMEOUT_MS - 1000) / 100; ++i) {
            if (!isProcessAlive(pid)) {
                LOG_DEBUG("Process " + std::to_string(pid) + " terminated gracefully");
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // If still alive, force kill
        LOG_WARNING("Process " + std::to_string(pid) + " did not terminate gracefully, using SIGKILL");
        if (kill(pid, SIGKILL) == 0) {
            // Wait a bit more for SIGKILL to take effect
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Reduced from 500ms
            return !isProcessAlive(pid);
        }
    }
    
    LOG_ERROR("Failed to terminate process " + std::to_string(pid));
    return false;
}

bool ProcessManager::setupPipes(int stdout_pipe[2], int stderr_pipe[2]) {
    if (pipe(stdout_pipe) == -1) {
        LOG_ERROR("Failed to create stdout pipe: " + std::string(strerror(errno)));
        return false;
    }
    
    if (pipe(stderr_pipe) == -1) {
        LOG_ERROR("Failed to create stderr pipe: " + std::string(strerror(errno)));
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        return false;
    }
    
    return true;
}

void ProcessManager::closePipe(int fd) {
    if (fd >= 0) {
        close(fd);
    }
}

int ProcessManager::cleanupFinishedProcesses() {
    int cleanedUp = 0;
    
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        auto& process = m_processes[i];
        
        // Check if process is finished but not yet cleaned up
        if (process->status == ProcessStatus::FINISHED || process->status == ProcessStatus::ERROR) {
            LOG_INFO("Auto-cleaning up finished process in pane " + std::to_string(i) + 
                    " (status: " + (process->status == ProcessStatus::FINISHED ? "FINISHED" : "ERROR") + 
                    ", exit code: " + std::to_string(process->exitCode) + ")");
            
            // Add a completion message to the output
            {
                std::lock_guard<std::mutex> lock(process->outputMutex);
                std::string completionMsg = "\n" + std::string(40, '-') + "\n";
                completionMsg += "🏁 Process completed ";
                if (process->status == ProcessStatus::FINISHED) {
                    if (process->exitCode == 0) {
                        completionMsg += "successfully ✅";
                    } else {
                        completionMsg += "with exit code " + std::to_string(process->exitCode) + " ⚠️";
                    }
                } else {
                    completionMsg += "with error ❌";
                }
                completionMsg += "\n" + std::string(40, '-') + "\n";
                process->outputBuffer.push(completionMsg);
            }
            
            // Reset the process to clean state
            process->reset();
            cleanedUp++;
        }
    }
    
    if (cleanedUp > 0) {
        LOG_DEBUG("Cleaned up " + std::to_string(cleanedUp) + " finished processes");
    }
    
    return cleanedUp;
}
