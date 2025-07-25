# API Reference - C++ Terminal Script Runner

## Overview

This document provides comprehensive API documentation for all classes and interfaces in the Script Runner application. The API is designed for extensibility and follows modern C++ best practices.

## Table of Contents
1. [ScriptRunner Class](#scriptrunner-class)
2. [UIManager Class](#uimanager-class)
3. [ProcessManager Class](#processmanager-class)
4. [Logger Class](#logger-class)
5. [Data Structures](#data-structures)
6. [Enumerations](#enumerations)
7. [Utility Functions](#utility-functions)
8. [Error Classes](#error-classes)

---

## ScriptRunner Class

**File**: `src/ScriptRunner.h`, `src/ScriptRunner.cpp`

Main application controller class responsible for coordinating all subsystems.

### Public Interface

#### Constructor/Destructor
```cpp
/**
 * @brief Construct a new Script Runner object
 * 
 * Initializes all subsystems (UI, Process Management, Logging) and sets up
 * the application environment.
 * 
 * @throws ScriptRunnerException If initialization fails
 */
ScriptRunner();

/**
 * @brief Destroy the Script Runner object
 * 
 * Ensures clean shutdown of all subsystems and resource cleanup.
 */
~ScriptRunner();
```

#### Application Lifecycle
```cpp
/**
 * @brief Run the main application loop
 * 
 * Starts the event loop and handles user interaction until application exit.
 * This is the main entry point for application execution.
 * 
 * @return int Exit code (0 for success, non-zero for error)
 * 
 * @throws UIException If terminal initialization fails
 * @throws ProcessException If process management setup fails
 */
int run();

/**
 * @brief Initiate application shutdown
 * 
 * Performs graceful shutdown by terminating all running processes,
 * cleaning up resources, and preparing for application exit.
 * 
 * @post All processes terminated, resources cleaned up
 */
void shutdown();
```

#### Script Management
```cpp
/**
 * @brief Execute a script in the specified output pane
 * 
 * Initiates execution of the selected script using the ProcessManager,
 * updates the UI to reflect the new process state, and begins output
 * capture for real-time display.
 * 
 * @param scriptPath Absolute path to the script file to execute
 * @param paneIndex Target output pane (0 or 1)
 * 
 * @throws ProcessException If script execution fails
 * @throws FileSystemException If script file is not accessible
 * 
 * @pre scriptPath must be a valid, executable file
 * @pre paneIndex must be 0 or 1
 * @post Process is started and output capture begins
 */
void executeScript(const std::string& scriptPath, int paneIndex);

/**
 * @brief Terminate the script running in the specified pane
 * 
 * Sends termination signal to the process and cleans up associated resources.
 * 
 * @param paneIndex Target output pane (0 or 1)
 * 
 * @pre paneIndex must be 0 or 1
 * @post Process is terminated and resources cleaned up
 */
void terminateScript(int paneIndex);

/**
 * @brief Refresh the list of available scripts
 * 
 * Rescans the scripts directory and updates the UI with newly discovered
 * or removed scripts.
 * 
 * @post Script list is updated in UI
 */
void refreshScriptList();
```

#### Event Handling
```cpp
/**
 * @brief Handle keyboard input from the user
 * 
 * Processes keyboard events and routes them to appropriate handlers.
 * 
 * @param key Key code from ncurses getch()
 */
void handleKeyPress(int key);

/**
 * @brief Handle terminal resize events
 * 
 * Recalculates layout and updates all UI components to match new terminal size.
 */
void handleResize();
```

### Private Members
```cpp
private:
    std::unique_ptr<UIManager> m_uiManager;         ///< UI management subsystem
    std::unique_ptr<ProcessManager> m_processManager; ///< Process execution subsystem
    std::unique_ptr<Logger> m_logger;               ///< Logging subsystem
    
    bool m_running;                                 ///< Application running state
    std::vector<std::string> m_availableScripts;   ///< Cached script list
    int m_selectedScript;                           ///< Currently selected script index
    int m_activePaneIndex;                          ///< Currently active output pane
```

---

## UIManager Class

**File**: `src/UIManager.h`, `src/UIManager.cpp`

Manages all terminal user interface operations using ncurses.

### Public Interface

#### Constructor/Destructor
```cpp
/**
 * @brief Construct a new UI Manager object
 * 
 * Sets up ncurses environment and initializes color pairs.
 */
UIManager();

/**
 * @brief Destroy the UI Manager object
 * 
 * Cleans up ncurses resources and restores terminal state.
 */
~UIManager();
```

#### Initialization
```cpp
/**
 * @brief Initialize the UI subsystem
 * 
 * Sets up ncurses, creates windows, and initializes color pairs.
 * 
 * @return true If initialization successful
 * @return false If initialization failed
 * 
 * @throws UIException If ncurses initialization fails
 */
bool initialize();

/**
 * @brief Clean up UI resources
 * 
 * Destroys windows and restores terminal to original state.
 */
void cleanup();
```

#### Layout Management
```cpp
/**
 * @brief Create the main layout with all panes
 * 
 * Calculates window sizes and positions based on terminal dimensions.
 * 
 * @pre Terminal must be at least 80x24 characters
 * @post All windows created and positioned
 */
void createLayout();

/**
 * @brief Handle terminal resize events
 * 
 * Recalculates layout and repositions all windows.
 */
void handleResize();
```

#### Content Updates
```cpp
/**
 * @brief Update the script list display
 * 
 * Refreshes the script list pane with current available scripts.
 * 
 * @param scripts Vector of script paths to display
 */
void updateScriptList(const std::vector<std::string>& scripts);

/**
 * @brief Update output in the specified pane
 * 
 * Adds new output lines to the specified output pane with automatic scrolling.
 * 
 * @param paneIndex Target pane (0 or 1)
 * @param output New output text to append
 * 
 * @pre paneIndex must be 0 or 1
 */
void updateOutput(int paneIndex, const std::string& output);

/**
 * @brief Update the status bar information
 * 
 * Refreshes status bar with current application state.
 * 
 * @param status Status message to display
 */
void updateStatus(const std::string& status);
```

#### User Interaction
```cpp
/**
 * @brief Get user input from keyboard
 * 
 * Non-blocking keyboard input capture.
 * 
 * @return int Key code (ERR if no input available)
 */
int getInput();

/**
 * @brief Highlight the specified script in the list
 * 
 * Updates visual highlighting for script selection.
 * 
 * @param index Script index to highlight
 * 
 * @pre index must be valid script list index
 */
void highlightScript(int index);

/**
 * @brief Switch focus to the specified pane
 * 
 * Changes active pane and updates visual indicators.
 * 
 * @param paneIndex Target pane (0=script list, 1=output1, 2=output2)
 * 
 * @pre paneIndex must be 0, 1, or 2
 */
void switchPane(int paneIndex);
```

#### Display Management
```cpp
/**
 * @brief Refresh all screen content
 * 
 * Forces complete redraw of all windows.
 */
void refresh();

/**
 * @brief Show or hide the help overlay
 * 
 * Toggles help screen display.
 */
void showHelp();
```

### Private Members
```cpp
private:
    WINDOW* m_mainWindow;      ///< Main application window
    WINDOW* m_scriptListPane;  ///< Script list pane
    WINDOW* m_outputPane1;     ///< First output pane
    WINDOW* m_outputPane2;     ///< Second output pane
    WINDOW* m_statusBar;       ///< Status bar window
    
    int m_selectedScript;      ///< Currently selected script
    int m_activePane;          ///< Currently active pane
    bool m_helpVisible;        ///< Help overlay visibility state
    
    // Layout calculations
    void calculateLayout();    ///< Calculate window positions and sizes
    void drawBorders();        ///< Draw window borders and decorations
    void drawHeaders();        ///< Draw pane headers and titles
```

---

## ProcessManager Class

**File**: `src/ProcessManager.h`, `src/ProcessManager.cpp`

Handles script discovery, execution, and output capture.

### Public Interface

#### Constructor/Destructor
```cpp
/**
 * @brief Construct a new Process Manager object
 * 
 * Initializes process management subsystem.
 */
ProcessManager();

/**
 * @brief Destroy the Process Manager object
 * 
 * Terminates all running processes and cleans up resources.
 */
~ProcessManager();
```

#### Script Discovery
```cpp
/**
 * @brief Discover all executable scripts in the specified directory
 * 
 * Recursively scans directory for executable files and builds command strings.
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
```

#### Process Execution
```cpp
/**
 * @brief Start script execution in the specified pane
 * 
 * Launches script process and begins output capture.
 * 
 * @param scriptPath Path to script to execute
 * @param paneIndex Target output pane (0 or 1)
 * @return true If script started successfully
 * @return false If script failed to start
 * 
 * @throws ProcessException If execution setup fails
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
 * 
 * @param paneIndex Target pane (0 or 1)
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
 * @pre paneIndex must be 0 or 1
 */
bool isRunning(int paneIndex);
```

#### Output Management
```cpp
/**
 * @brief Get new output from the specified pane
 * 
 * Returns accumulated output since last call (non-blocking).
 * 
 * @param paneIndex Target pane (0 or 1)
 * @return std::string New output text (empty if none available)
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
 * @pre paneIndex must be 0 or 1
 * @post Output buffer is empty
 */
void clearOutput(int paneIndex);
```

#### Status Information
```cpp
/**
 * @brief Get current process status for the specified pane
 * 
 * @param paneIndex Target pane (0 or 1)
 * @return ProcessStatus Current status
 * 
 * @pre paneIndex must be 0 or 1
 */
ProcessStatus getStatus(int paneIndex);
```

### Private Members
```cpp
private:
    /**
     * @brief Internal structure representing a running script process
     */
    struct ScriptProcess {
        pid_t pid;                                    ///< Process ID
        int stdout_fd;                               ///< stdout file descriptor
        int stderr_fd;                               ///< stderr file descriptor
        std::thread outputThread;                    ///< Output reader thread
        std::queue<std::string> outputBuffer;       ///< Buffered output lines
        std::mutex outputMutex;                      ///< Output buffer protection
        bool running;                                ///< Process running state
        std::string command;                         ///< Command being executed
        std::chrono::steady_clock::time_point startTime; ///< Process start time
    };
    
    std::array<std::unique_ptr<ScriptProcess>, 2> m_processes; ///< Process slots
    
    // Internal methods
    void outputReaderThread(int paneIndex);         ///< Output capture thread function
    std::string determineScriptType(const std::string& filePath); ///< Detect script type
    std::string buildCommand(const std::string& scriptPath);      ///< Build execution command
```

---

## Logger Class

**File**: `src/Logger.h`, `src/Logger.cpp`

Singleton logging system for application-wide debug and error reporting.

### Public Interface

#### Singleton Access
```cpp
/**
 * @brief Get the singleton Logger instance
 * 
 * @return Logger& Reference to the global logger instance
 */
static Logger& getInstance();
```

#### Configuration
```cpp
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
 * @throws FileSystemException If file cannot be created or opened
 */
void setLogFile(const std::string& filename);
```

#### Logging Methods
```cpp
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
```

### Convenience Macros
```cpp
// Use these macros for efficient logging with automatic file/line information
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
#define LOG_FATAL(msg) Logger::getInstance().fatal(msg)
```

---

## Data Structures

### ProcessStatus Enumeration
```cpp
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
```

### Logger Level Enumeration
```cpp
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
```

### ColorPair Enumeration
```cpp
/**
 * @brief ncurses color pair identifiers
 */
enum class ColorPair {
    DEFAULT = 1,    ///< Default terminal colors
    HEADER = 2,     ///< Pane headers (cyan on black)
    SELECTED = 3,   ///< Selected items (black on yellow)
    RUNNING = 4,    ///< Running process indicator (green on black)
    ERROR = 5,      ///< Error messages (red on black)
    SUCCESS = 6,    ///< Success messages (green on black)
    STATUS = 7,     ///< Status bar (white on blue)
    BORDER = 8      ///< Window borders (white on black)
};
```

### SystemStatus Structure
```cpp
/**
 * @brief System resource status information
 */
struct SystemStatus {
    size_t memoryUsageMB;           ///< Current memory usage in MB
    double cpuUsagePercent;         ///< Current CPU usage percentage
    std::chrono::seconds uptime;    ///< Application uptime
    int activeProcesses;            ///< Number of running processes
    size_t availableScripts;        ///< Number of discovered scripts
};
```

---

## Utility Functions

### File System Utilities
```cpp
/**
 * @brief Get file extension from path
 * 
 * @param filePath Path to file
 * @return std::string File extension (including dot, e.g., ".py")
 */
std::string getFileExtension(const std::string& filePath);

/**
 * @brief Get base filename from path
 * 
 * @param filePath Full file path
 * @return std::string Filename without directory path
 */
std::string getBaseName(const std::string& filePath);

/**
 * @brief Check if file exists and is accessible
 * 
 * @param filePath Path to check
 * @return true If file exists and is readable
 * @return false If file doesn't exist or is inaccessible
 */
bool fileExists(const std::string& filePath);
```

### String Utilities
```cpp
/**
 * @brief Split string by delimiter
 * 
 * @param str String to split
 * @param delimiter Character to split on
 * @return std::vector<std::string> Vector of split strings
 */
std::vector<std::string> splitString(const std::string& str, char delimiter);

/**
 * @brief Trim whitespace from string
 * 
 * @param str String to trim
 * @return std::string Trimmed string
 */
std::string trimString(const std::string& str);

/**
 * @brief Format time duration as human-readable string
 * 
 * @param duration Duration to format
 * @return std::string Formatted duration (e.g., "2h15m30s")
 */
std::string formatDuration(std::chrono::seconds duration);
```

### System Utilities
```cpp
/**
 * @brief Get current system memory usage
 * 
 * @return size_t Memory usage in bytes
 */
size_t getMemoryUsage();

/**
 * @brief Get current CPU usage percentage
 * 
 * @return double CPU usage as percentage (0.0-100.0)
 */
double getCpuUsage();

/**
 * @brief Check if process is still running
 * 
 * @param pid Process ID to check
 * @return true If process is running
 * @return false If process has terminated
 */
bool isProcessRunning(pid_t pid);
```

---

## Error Classes

### Base Exception Class
```cpp
/**
 * @brief Base exception class for all Script Runner errors
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
```

### UI Exception Class
```cpp
/**
 * @brief Exception for UI-related errors
 */
class UIException : public ScriptRunnerException {
public:
    explicit UIException(const std::string& message);
};
```

### Process Exception Class
```cpp
/**
 * @brief Exception for process execution errors
 */
class ProcessException : public ScriptRunnerException {
public:
    explicit ProcessException(const std::string& message);
};
```

### File System Exception Class
```cpp
/**
 * @brief Exception for file system access errors
 */
class FileSystemException : public ScriptRunnerException {
public:
    explicit FileSystemException(const std::string& message);
};
```

---

## Usage Examples

### Basic Application Setup
```cpp
#include "ScriptRunner.h"

int main() {
    try {
        ScriptRunner app;
        return app.run();
    } catch (const ScriptRunnerException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
```

### Custom Logging Configuration
```cpp
#include "Logger.h"

void setupLogging() {
    auto& logger = Logger::getInstance();
    logger.setLevel(Logger::Level::DEBUG);
    logger.setLogFile("script_runner.log");
    
    LOG_INFO("Application starting");
}
```

### Process Management Example
```cpp
#include "ProcessManager.h"

void executeCustomScript() {
    ProcessManager pm;
    
    // Discover available scripts
    auto scripts = pm.discoverScripts("../scripts");
    
    // Start first script in pane 0
    if (!scripts.empty()) {
        if (pm.startScript(scripts[0], 0)) {
            LOG_INFO("Script started successfully");
        } else {
            LOG_ERROR("Failed to start script");
        }
    }
}
```

### UI Update Example
```cpp
#include "UIManager.h"

void updateInterface(UIManager& ui) {
    std::vector<std::string> scripts = {"script1.py", "script2.sh"};
    
    // Update script list
    ui.updateScriptList(scripts);
    
    // Add output to pane 0
    ui.updateOutput(0, "Process output line 1\n");
    ui.updateOutput(0, "Process output line 2\n");
    
    // Update status
    ui.updateStatus("2 scripts running");
    
    // Refresh display
    ui.refresh();
}
```

---

This API reference provides comprehensive documentation for all public interfaces in the Script Runner application. All classes follow RAII principles and provide strong exception safety guarantees.
