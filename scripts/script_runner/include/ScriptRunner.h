/**
 * @file ScriptRunner.h
 * @brief Main application controller for the terminal script runner
 * 
 * This file defines the ScriptRunner class which serves as the central
 * coordinator for all application subsystems, managing the lifecycle
 * of the UI, process management, and logging components.
 * 
 * @author GitHub Copilot
 * @date 2025-07-19
 * @version 1.0.0
 */

#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include "UIManager.h"
#include "ProcessManager.h"
#include "Logger.h"
#include "Exceptions.h"
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <atomic>

/**
 * @brief Main application controller coordinating all subsystems
 * 
 * The ScriptRunner class serves as the central coordinator for the entire
 * application, implementing the main event loop and managing communication
 * between the UI, process management, and logging subsystems.
 * 
 * Key responsibilities:
 * - Application lifecycle management (initialization, main loop, shutdown)
 * - Event routing between UI and process management
 * - Keyboard input handling and command processing
 * - Real-time UI updates with process output
 * - Error handling and recovery
 * - Signal handling for graceful shutdown
 * 
 * Architecture:
 * - Follows MVC pattern with ScriptRunner as Controller
 * - UIManager handles View layer (terminal interface)
 * - ProcessManager handles Model layer (script execution)
 * - Logger provides cross-cutting logging functionality
 */
class ScriptRunner {
public:
    /**
     * @brief Construct a new Script Runner object
     * 
     * Initializes all subsystems and prepares the application for execution.
     * 
     * @throws ScriptRunnerException If subsystem initialization fails
     */
    ScriptRunner();

    /**
     * @brief Destroy the Script Runner object
     * 
     * Ensures clean shutdown of all subsystems and resource cleanup.
     */
    ~ScriptRunner();

    // Delete copy constructor and assignment (resource management)
    ScriptRunner(const ScriptRunner&) = delete;
    ScriptRunner& operator=(const ScriptRunner&) = delete;

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
     * Sets the running flag to false to exit the main loop.
     */
    void shutdown();

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
     * @throws std::invalid_argument If paneIndex is invalid
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
     * @throws std::invalid_argument If paneIndex is invalid
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
     */
    void refreshScriptList();

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

    /**
     * @brief Get the current running state
     * 
     * @return true If application should continue running
     * @return false If application should exit
     */
    bool isRunning() const { return m_running; }

private:
    // Subsystem components
    std::unique_ptr<UIManager> m_uiManager;         ///< UI management subsystem
    std::unique_ptr<ProcessManager> m_processManager; ///< Process execution subsystem

    // Application state
    std::atomic<bool> m_running;                    ///< Application running state
    std::vector<std::string> m_availableScripts;   ///< Cached script list
    std::string m_scriptsDirectory;                 ///< Primary script discovery directory
    std::vector<std::string> m_scriptDirectories;  ///< All script directories to search

    // Timing for UI updates
    std::chrono::steady_clock::time_point m_lastUpdate; ///< Last UI update time
    std::chrono::steady_clock::time_point m_startTime;  ///< Application start time

    // Configuration
    static constexpr int UI_UPDATE_INTERVAL_MS = 50;    ///< UI update interval (20 FPS)
    static constexpr int INPUT_POLL_INTERVAL_MS = 10;   ///< Input polling interval

    /**
     * @brief Initialize all application subsystems
     * 
     * Sets up UI, process management, and discovers initial script list.
     * 
     * @throws ScriptRunnerException If any subsystem initialization fails
     */
    void initializeSubsystems();

    /**
     * @brief Main event loop implementation
     * 
     * Handles input events, updates UI, and manages process output.
     */
    void eventLoop();

    /**
     * @brief Update UI with current application state
     * 
     * Refreshes process output, status information, and other dynamic content.
     */
    void updateUI();

    /**
     * @brief Handle navigation keys (arrows, hjkl)
     * 
     * @param key Key code to process
     * @return true If key was handled
     * @return false If key was not a navigation key
     */
    bool handleNavigationKey(int key);

    /**
     * @brief Handle execution control keys (Enter, Space, Ctrl+C)
     * 
     * @param key Key code to process
     * @return true If key was handled
     * @return false If key was not an execution key
     */
    bool handleExecutionKey(int key);

    /**
     * @brief Handle view control keys (PageUp/Down, Ctrl+L, etc.)
     * 
     * @param key Key code to process
     * @return true If key was handled
     * @return false If key was not a view control key
     */
    bool handleViewKey(int key);

    /**
     * @brief Handle application control keys (Ctrl+Q, F1, etc.)
     * 
     * @param key Key code to process
     * @return true If key was handled
     * @return false If key was not an application control key
     */
    bool handleApplicationKey(int key);

    /**
     * @brief Check if a script list item is a separator
     * 
     * @param item Script list item to check
     * @return bool True if item is a separator
     */
    bool isSeparator(const std::string& item) const;

    /**
     * @brief Find next valid script index (skipping separators)
     * 
     * @param currentIndex Current index
     * @param direction Direction to search (+1 for down, -1 for up)
     * @return int Next valid script index, or currentIndex if none found
     */
    int findNextValidScript(int currentIndex, int direction) const;

    /**
     * @brief Get next available output pane
     * 
     * @return int Available pane index (0 or 1), or -1 if both are busy
     */
    int getAvailablePane() const;

    /**
     * @brief Format status bar information
     * 
     * @return std::string Formatted status for left section
     */
    std::string formatLeftStatus() const;

    /**
     * @brief Format center status information
     * 
     * @return std::string Formatted status for center section
     */
    std::string formatCenterStatus() const;

    /**
     * @brief Format right status information (resources, uptime)
     * 
     * @return std::string Formatted status for right section
     */
    std::string formatRightStatus() const;

    /**
     * @brief Check if enough time has passed for UI update
     * 
     * @return true If UI should be updated
     * @return false If update should be skipped
     */
    bool shouldUpdateUI() const;

    /**
     * @brief Get current application uptime
     * 
     * @return std::chrono::seconds Uptime duration
     */
    std::chrono::seconds getUptime() const;

    /**
     * @brief Format duration as human-readable string
     * 
     * @param duration Duration to format
     * @return std::string Formatted duration (e.g., "2h15m30s")
     */
    std::string formatDuration(std::chrono::seconds duration) const;

    /**
     * @brief Get current time as formatted string
     * 
     * @return std::string Current time (e.g., "14:35:22")
     */
    std::string getCurrentTimeString() const;
};

#endif // SCRIPTRUNNER_H
