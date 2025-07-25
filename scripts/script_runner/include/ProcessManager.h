/**
 * @file ProcessManager.h
 * @brief Script execution and process management system
 * 
 * This file defines the ProcessManager class which handles script discovery,
 * execution, output capture, and process lifecycle management.
 * 
 * @author GitHub Copilot
 * @date 2025-07-19
 * @version 1.0.0
 */

#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "Exceptions.h"
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>

/**
 * @brief Process execution status
 */
enum class ProcessStatus {
    IDLE,       ///< No process running
    STARTING,   ///< Process is being launched
    RUNNING,    ///< Process is actively running
    FINISHED,   ///< Process completed successfully
    ERROR,      ///< Process failed with error
    TERMINATED  ///< Process was terminated by user
};

/**
 * @brief Manages script discovery, execution, and output capture
 * 
 * The ProcessManager class provides comprehensive process management
 * functionality including script discovery, concurrent execution,
 * real-time output capture, and process lifecycle control.
 * 
 * Key features:
 * - Automatic script discovery and type detection
 * - Concurrent execution of up to 2 scripts
 * - Real-time output capture with thread-safe buffering
 * - Process termination and cleanup
 * - Resource monitoring and management
 */
class ProcessManager {
public:
    /**
     * @brief Construct a new Process Manager object
     * 
     * Initializes the process management subsystem and prepares
     * for script discovery and execution.
     */
    ProcessManager();

    /**
     * @brief Destroy the Process Manager object
     * 
     * Terminates all running processes and cleans up resources.
     * Ensures all worker threads are properly joined.
     */
    ~ProcessManager();

    // Delete copy constructor and assignment (resource management)
    ProcessManager(const ProcessManager&) = delete;
    ProcessManager& operator=(const ProcessManager&) = delete;

    /**
     * @brief Discover all executable scripts in the specified directory
     * 
     * Recursively scans directory for executable files and builds
     * appropriate execution commands based on file type detection.
     * 
     * @param directory Directory path to scan
     * @return std::vector<std::string> List of discovered script paths
     * 
     * @throws FileSystemException If directory access fails
     */
    std::vector<std::string> discoverScripts(const std::string& directory);

    /**
     * @brief Check if a file is executable
     * 
     * Verifies file permissions and determines if file can be executed.
     * 
     * @param filePath Path to file to check
     * @return true If file is executable
     * @return false If file is not executable
     */
    bool isExecutable(const std::string& filePath);

    /**
     * @brief Start script execution in the specified pane
     * 
     * Launches script process and begins output capture in a separate thread.
     * 
     * @param scriptPath Path to script to execute
     * @param paneIndex Target output pane (0 or 1)
     * @return true If script started successfully
     * @return false If script failed to start
     * 
     * @throws ProcessException If execution setup fails
     * @throws std::invalid_argument If paneIndex is invalid
     * 
     * @pre scriptPath must be valid and executable
     * @pre paneIndex must be 0 or 1
     * @post Process is running and output capture is active
     */
    bool startScript(const std::string& scriptPath, int paneIndex);

    /**
     * @brief Terminate script in the specified pane
     * 
     * Sends SIGTERM to process, waits briefly, then sends SIGKILL if necessary.
     * Cleans up all associated resources including pipes and threads.
     * 
     * @param paneIndex Target pane (0 or 1)
     * 
     * @throws std::invalid_argument If paneIndex is invalid
     * 
     * @pre paneIndex must be 0 or 1
     * @post Process is terminated and resources cleaned up
     */
    void terminateScript(int paneIndex);

    /**
     * @brief Check if script is running in the specified pane
     * 
     * @param paneIndex Target pane (0 or 1)
     * @return true If script is currently running
     * @return false If pane is idle
     * 
     * @throws std::invalid_argument If paneIndex is invalid
     * 
     * @pre paneIndex must be 0 or 1
     */
    bool isRunning(int paneIndex);

    /**
     * @brief Get new output from the specified pane
     * 
     * Returns accumulated output since last call (non-blocking).
     * Output is automatically removed from buffer after retrieval.
     * 
     * @param paneIndex Target pane (0 or 1)
     * @return std::string New output text (empty if none available)
     * 
     * @throws std::invalid_argument If paneIndex is invalid
     * 
     * @pre paneIndex must be 0 or 1
     */
    std::string getNewOutput(int paneIndex);

    /**
     * @brief Clear output buffer for the specified pane
     * 
     * Removes all accumulated output from the pane buffer.
     * 
     * @param paneIndex Target pane (0 or 1)
     * 
     * @throws std::invalid_argument If paneIndex is invalid
     * 
     * @pre paneIndex must be 0 or 1
     * @post Output buffer is empty
     */
    void clearOutput(int paneIndex);

    /**
     * @brief Get current process status for the specified pane
     * 
     * @param paneIndex Target pane (0 or 1)
     * @return ProcessStatus Current status
     * 
     * @throws std::invalid_argument If paneIndex is invalid
     * 
     * @pre paneIndex must be 0 or 1
     */
    ProcessStatus getStatus(int paneIndex);

    /**
     * @brief Get process runtime for the specified pane
     * 
     * @param paneIndex Target pane (0 or 1)
     * @return std::chrono::seconds Runtime duration (0 if not running)
     * 
     * @throws std::invalid_argument If paneIndex is invalid
     * 
     * @pre paneIndex must be 0 or 1
     */
    std::chrono::seconds getRuntime(int paneIndex);

    /**
     * @brief Get the command string for the specified pane
     * 
     * @param paneIndex Target pane (0 or 1)
     * @return std::string Command being executed (empty if idle)
     * 
     * @throws std::invalid_argument If paneIndex is invalid
     * 
     * @pre paneIndex must be 0 or 1
     */
    std::string getCommand(int paneIndex);

    /**
     * @brief Clean up finished processes automatically
     * 
     * Checks all panes for processes that have finished naturally
     * and cleans up their resources. Should be called periodically
     * to prevent accumulation of zombie processes.
     * 
     * @return int Number of processes cleaned up
     */
    int cleanupFinishedProcesses();

private:
    /**
     * @brief Internal structure representing a running script process
     */
    struct ScriptProcess {
        pid_t pid;                                    ///< Process ID (-1 if not running)
        int stdout_fd;                               ///< stdout file descriptor
        int stderr_fd;                               ///< stderr file descriptor
        std::unique_ptr<std::thread> outputThread;  ///< Output reader thread
        std::queue<std::string> outputBuffer;       ///< Buffered output lines
        std::mutex outputMutex;                      ///< Output buffer protection
        std::atomic<ProcessStatus> status;           ///< Process status
        std::string command;                         ///< Command being executed
        std::chrono::steady_clock::time_point startTime; ///< Process start time
        std::atomic<bool> shouldStop;                ///< Flag to stop output thread
        int exitCode;                                ///< Process exit code
        
        ScriptProcess();
        ~ScriptProcess();
        void reset();
    };
    
    static constexpr int MAX_PROCESSES = 2;          ///< Maximum concurrent processes
    std::array<std::unique_ptr<ScriptProcess>, MAX_PROCESSES> m_processes; ///< Process slots
    
    // Configuration
    static constexpr size_t MAX_BUFFER_SIZE = 1000;  ///< Maximum output lines per buffer
    static constexpr int TERMINATION_TIMEOUT_MS = 5000; ///< Timeout for graceful termination
    
    /**
     * @brief Validate pane index parameter
     * 
     * @param paneIndex Index to validate
     * @throws std::invalid_argument If index is out of range
     */
    void validatePaneIndex(int paneIndex) const;
    
    /**
     * @brief Output capture thread function
     * 
     * Runs in separate thread to capture stdout/stderr from process.
     * 
     * @param paneIndex Process pane index
     */
    void outputReaderThread(int paneIndex);
    
    /**
     * @brief Determine script type from file path
     * 
     * Analyzes file extension and content to determine appropriate interpreter.
     * 
     * @param filePath Path to script file
     * @return std::string Script type identifier
     */
    std::string determineScriptType(const std::string& filePath);
    
    /**
     * @brief Build execution command for script
     * 
     * Creates appropriate command string based on script type.
     * 
     * @param scriptPath Path to script file
     * @return std::string Complete command to execute script
     */
    std::string buildCommand(const std::string& scriptPath);
    
    /**
     * @brief Check if process is still alive
     * 
     * @param pid Process ID to check
     * @return true If process is running
     * @return false If process has terminated
     */
    bool isProcessAlive(pid_t pid);
    
    /**
     * @brief Kill process with escalating signals
     * 
     * Sends SIGTERM first, waits, then SIGKILL if necessary.
     * 
     * @param pid Process ID to terminate
     * @return true If process was terminated
     * @return false If termination failed
     */
    bool killProcess(pid_t pid);
    
    /**
     * @brief Set up pipes for process communication
     * 
     * @param stdout_pipe Array to store stdout pipe descriptors
     * @param stderr_pipe Array to store stderr pipe descriptors
     * @return true If pipes created successfully
     * @return false If pipe creation failed
     */
    bool setupPipes(int stdout_pipe[2], int stderr_pipe[2]);
    
    /**
     * @brief Close pipe descriptors safely
     * 
     * @param fd File descriptor to close (-1 is ignored)
     */
    void closePipe(int fd);
};

#endif // PROCESSMANAGER_H
