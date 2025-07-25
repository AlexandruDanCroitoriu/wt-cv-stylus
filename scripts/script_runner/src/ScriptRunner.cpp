/**
 * @file ScriptRunner.cpp
 * @brief Implementation of the main application controller
 * 
 * This file implements the ScriptRunner class which coordinates all
 * application subsystems and manages the main event loop.
 * 
 * @author GitHub Copilot
 * @date 2025-07-19
 * @version 1.0.0
 */

#include "ScriptRunner.h"
#include <ncurses.h>
#include <signal.h>
#include <unistd.h>
#include <algorithm>
#include <iomanip>
#include <set>
#include <sstream>
#include <thread>
#include <thread>
#include <sys/stat.h>

// Global reference for signal handler
ScriptRunner* g_scriptRunner = nullptr;

/**
 * @brief Signal handler for graceful shutdown
 * 
 * @param signum Signal number received
 */
void signalHandler(int signum) {
    if (g_scriptRunner) {
        g_scriptRunner->shutdown();
    }
}

ScriptRunner::ScriptRunner()
    : m_running(false)
    , m_scriptsDirectory("examples/sample_scripts")
    , m_startTime(std::chrono::steady_clock::now())
    , m_lastUpdate(std::chrono::steady_clock::now()) {
    
    Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Initializing application");
    
    // Set up signal handlers
    g_scriptRunner = this;
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    initializeSubsystems();
    
    Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Initialization complete");
}

ScriptRunner::~ScriptRunner() {
    Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Shutting down application");
    
    // Clear global reference
    g_scriptRunner = nullptr;
    
    // Cleanup is handled by unique_ptr destructors
    Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Shutdown complete");
}

int ScriptRunner::run() {
    try {
        Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Starting main event loop");
        
        // Initialize UI
        m_uiManager->initialize();
        
        // Disable console logging to prevent UI corruption
        Logger::getInstance().setConsoleLogging(false);
        
        // Refresh script list
        refreshScriptList();
        
        // Start running
        m_running = true;
        
        // Main event loop
        eventLoop();
        
        // Re-enable console logging after UI shutdown
        Logger::getInstance().setConsoleLogging(true);
        
        // Clean shutdown
        Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Event loop exited normally");
        return 0;
        
    } catch (const UIException& e) {
        Logger::getInstance().log(Logger::Level::ERROR, "ScriptRunner: UI error: " + std::string(e.what()));
        return 1;
    } catch (const ProcessException& e) {
        Logger::getInstance().log(Logger::Level::ERROR, "ScriptRunner: Process error: " + std::string(e.what()));
        return 2;
    } catch (const std::exception& e) {
        Logger::getInstance().log(Logger::Level::ERROR, "ScriptRunner: Unexpected error: " + std::string(e.what()));
        return 3;
    }
}

void ScriptRunner::shutdown() {
    Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Shutdown requested");
    m_running = false;
    
    // Terminate any running scripts
    if (m_processManager) {
        terminateScript(0);
        terminateScript(1);
    }
}

void ScriptRunner::executeScript(const std::string& scriptPath, int paneIndex) {
    if (paneIndex < 0 || paneIndex > 1) {
        throw std::invalid_argument("Invalid pane index: " + std::to_string(paneIndex));
    }
    
    Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Executing script " + scriptPath + " in pane " + std::to_string(paneIndex) + " (async)");
    
    // Show immediate feedback that execution is starting
    std::string scriptName = scriptPath.substr(scriptPath.find_last_of('/') + 1);
    std::string separator = "\n" + std::string(60, '=') + "\n";
    separator += "📜 EXECUTING: " + scriptName + "\n";
    separator += "⏰ Time: " + getCurrentTimeString() + "\n";
    separator += "🔧 Pane: " + std::to_string(paneIndex + 1) + "\n";
    separator += "⏳ Status: Starting execution (UI responsive)...\n";
    separator += std::string(60, '=') + "\n";
    m_uiManager->updateOutput(paneIndex, separator);
    
    // Update status immediately
    m_uiManager->updateStatus("Starting: " + scriptName, "UI remains responsive", "");
    
    // Launch execution in a separate thread to keep UI responsive
    std::thread([this, scriptPath, paneIndex, scriptName]() {
        try {
            auto startTime = std::chrono::steady_clock::now();
            
            // Stop any existing process in the target pane
            if (m_processManager->isRunning(paneIndex)) {
                auto terminateStartTime = std::chrono::steady_clock::now();
                m_uiManager->updateOutput(paneIndex, "🛑 Terminating existing process (in background)...\n");
                
                terminateScript(paneIndex);
                
                auto terminateEndTime = std::chrono::steady_clock::now();
                auto terminateMs = std::chrono::duration_cast<std::chrono::milliseconds>(terminateEndTime - terminateStartTime).count();
                m_uiManager->updateOutput(paneIndex, "✅ Process terminated (took " + std::to_string(terminateMs) + "ms)\n");
                
                // Give it a moment to clean up only if we actually terminated a running process
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            } else {
                m_uiManager->updateOutput(paneIndex, "✨ No existing process to terminate\n");
            }
            
            // Add startup timing
            auto startupTime = std::chrono::steady_clock::now();
            m_uiManager->updateOutput(paneIndex, "🚀 Starting new process (background)...\n");
            
            // Start the new process
            bool success = m_processManager->startScript(scriptPath, paneIndex);
            
            auto endTime = std::chrono::steady_clock::now();
            auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            auto startupMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startupTime).count();
            
            if (success) {
                m_uiManager->updateOutput(paneIndex, "✅ Process started (startup: " + std::to_string(startupMs) + "ms, total: " + std::to_string(totalMs) + "ms)\n");
                m_uiManager->updateOutput(paneIndex, std::string(60, '-') + "\n");
                
                // Update UI status
                std::string status = "Started: " + scriptName;
                m_uiManager->updateStatus(status, "", "");
            } else {
                m_uiManager->updateOutput(paneIndex, "❌ Failed to start process\n");
                m_uiManager->updateStatus("Error", "Failed to start: " + scriptName, "");
            }
            
        } catch (const ProcessException& e) {
            std::string error = "Failed to execute " + scriptPath + ": " + e.what();
            Logger::getInstance().log(Logger::Level::ERROR, error);
            m_uiManager->updateOutput(paneIndex, "❌ ERROR: " + error + "\n");
            m_uiManager->updateStatus("Error", error, "");
        } catch (const std::exception& e) {
            std::string error = "Unexpected error executing " + scriptPath + ": " + e.what();
            Logger::getInstance().log(Logger::Level::ERROR, error);
            m_uiManager->updateOutput(paneIndex, "❌ UNEXPECTED ERROR: " + error + "\n");
            m_uiManager->updateStatus("Error", error, "");
        }
    }).detach(); // Detach the thread so it runs independently
}

void ScriptRunner::terminateScript(int paneIndex) {
    if (paneIndex < 0 || paneIndex > 1) {
        throw std::invalid_argument("Invalid pane index: " + std::to_string(paneIndex));
    }
    
    if (m_processManager && m_processManager->isRunning(paneIndex)) {
        Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Terminating script in pane " + std::to_string(paneIndex));
        
        m_processManager->terminateScript(paneIndex);
        
        std::string status = "Terminated process in pane " + std::to_string(paneIndex);
        m_uiManager->updateStatus(status, "", "");
    }
}

void ScriptRunner::refreshScriptList() {
    Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Refreshing script list from multiple directories");
    
    try {
        // Combine scripts from all directories with separators
        std::vector<std::string> allScripts;
        std::set<std::string> uniqueScriptNames; // To avoid duplicates by filename
        
        for (size_t dirIndex = 0; dirIndex < m_scriptDirectories.size(); dirIndex++) {
            const auto& directory = m_scriptDirectories[dirIndex];
            Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: Searching directory: " + directory);
            
            try {
                auto scriptsInDir = m_processManager->discoverScripts(directory);
                
                // Add separator if this is not the first directory and we have scripts to show
                if (dirIndex > 0 && !scriptsInDir.empty() && !allScripts.empty()) {
                    std::string separator = "--- ";
                    if (directory == "..") {
                        separator += "Parent Directory Scripts";
                    } else if (directory == "../scripts" || directory == "scripts") {
                        separator += "Project Scripts";
                    } else if (directory.find("examples") != std::string::npos) {
                        separator += "Example Scripts";
                    } else {
                        separator += "Scripts from " + directory;
                    }
                    separator += " ---";
                    allScripts.push_back(separator);
                }
                
                for (const auto& scriptPath : scriptsInDir) {
                    // Extract filename for duplicate checking
                    std::string filename = scriptPath.substr(scriptPath.find_last_of('/') + 1);
                    
                    // Only add if we haven't seen this filename before
                    if (uniqueScriptNames.find(filename) == uniqueScriptNames.end()) {
                        allScripts.push_back(scriptPath);
                        uniqueScriptNames.insert(filename);
                        Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: Added script: " + scriptPath);
                    } else {
                        Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: Skipped duplicate: " + scriptPath);
                    }
                }
                
            } catch (const ProcessException& e) {
                Logger::getInstance().log(Logger::Level::WARNING, "ScriptRunner: Failed to scan directory '" + directory + "': " + e.what());
            }
        }
        
        m_availableScripts = allScripts;
        m_uiManager->updateScriptList(m_availableScripts);
        
        std::string status = "Found " + std::to_string(m_availableScripts.size()) + " scripts from " + std::to_string(m_scriptDirectories.size()) + " directories";
        m_uiManager->updateStatus(status, "", formatRightStatus());
        
        Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Found " + std::to_string(m_availableScripts.size()) + " total scripts");
        
    } catch (const std::exception& e) {
        std::string error = "Failed to refresh script list: " + std::string(e.what());
        Logger::getInstance().log(Logger::Level::ERROR, error);
        m_uiManager->updateStatus("Error", error, "");
    }
}

void ScriptRunner::handleKeyPress(int key) {
    // Log every key press with detailed information
    std::string keyDesc;
    switch (key) {
        case KEY_UP: keyDesc = "KEY_UP"; break;
        case KEY_DOWN: keyDesc = "KEY_DOWN"; break;
        case KEY_LEFT: keyDesc = "KEY_LEFT"; break;
        case KEY_RIGHT: keyDesc = "KEY_RIGHT"; break;
        case KEY_SLEFT: keyDesc = "SHIFT+LEFT"; break;
        case KEY_SRIGHT: keyDesc = "SHIFT+RIGHT"; break;
        case 545: keyDesc = "CTRL+LEFT(545)"; break;  
        case 546: keyDesc = "CTRL+LEFT(546)"; break;
        case 560: keyDesc = "CTRL+RIGHT(560)"; break; 
        case 561: keyDesc = "CTRL+RIGHT(561)"; break;
        case '\n':
        case '\r': keyDesc = "ENTER"; break;
        case ' ': keyDesc = "SPACE"; break;
        case '\t': keyDesc = "TAB"; break;
        case 3: keyDesc = "CTRL+C"; break;
        case 12: keyDesc = "CTRL+L"; break;
        case 17: keyDesc = "CTRL+Q"; break;
        case KEY_F(1): keyDesc = "F1"; break;
        case KEY_PPAGE: keyDesc = "PAGE_UP"; break;
        case KEY_NPAGE: keyDesc = "PAGE_DOWN"; break;
        case 'k': keyDesc = "k"; break;
        case 'j': keyDesc = "j"; break;
        case 'h': keyDesc = "h"; break;
        case 'l': keyDesc = "l"; break;
        case 'r': keyDesc = "r"; break;
        case 't': keyDesc = "t"; break;
        case '?': keyDesc = "?"; break;
        case '[': keyDesc = "[ (resize narrower)"; break;
        case ']': keyDesc = "] (resize wider)"; break;
        default: 
            if (key >= 32 && key <= 126) {
                keyDesc = "'" + std::string(1, static_cast<char>(key)) + "'";
            } else {
                keyDesc = "UNKNOWN(" + std::to_string(key) + ")";
            }
            break;
    }
    
    // Log key press with current state
    int selectedScript = m_uiManager ? m_uiManager->getSelectedScriptIndex() : -1;
    int activePane = m_uiManager ? m_uiManager->getActivePaneIndex() : -1;
    size_t scriptCount = m_availableScripts.size();
    
    Logger::getInstance().log(Logger::Level::DEBUG, 
        "ScriptRunner: Key pressed: " + keyDesc + " (code=" + std::to_string(key) + 
        ") | State: selectedScript=" + std::to_string(selectedScript) + 
        ", activePane=" + std::to_string(activePane) + 
        ", scriptCount=" + std::to_string(scriptCount));
    
    // Handle different categories of keys
    if (handleNavigationKey(key)) {
        Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: Key handled by navigation handler");
        return;
    }
    if (handleExecutionKey(key)) {
        Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: Key handled by execution handler");
        return;
    }
    if (handleViewKey(key)) {
        Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: Key handled by view handler");
        return;
    }
    if (handleApplicationKey(key)) {
        Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: Key handled by application handler");
        return;
    }
    
    // Unknown key - log for debugging
    Logger::getInstance().log(Logger::Level::WARNING, "ScriptRunner: Unhandled key: " + keyDesc + " (" + std::to_string(key) + ")");
}

void ScriptRunner::handleResize() {
    Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: Terminal resize detected");
    
    if (m_uiManager) {
        m_uiManager->handleResize();
        updateUI(); // Force immediate update
    }
}

void ScriptRunner::initializeSubsystems() {
    Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Initializing subsystems");
    
    // Initialize ProcessManager
    m_processManager = std::make_unique<ProcessManager>();
    
    // Initialize UIManager
    m_uiManager = std::make_unique<UIManager>();
    
    // Build list of script directories to search
    std::vector<std::string> searchDirectories = {
        "examples/sample_scripts",    // Project examples
        "..",                        // Parent directory  
        "../examples/sample_scripts", // Parent directory examples
        "scripts",                   // Scripts subdirectory
        "."                          // Current directory
    };
    
    // Filter to only existing directories
    m_scriptDirectories.clear();
    struct stat st;
    
    for (const auto& dir : searchDirectories) {
        if (stat(dir.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            m_scriptDirectories.push_back(dir);
            Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Added script directory: " + dir);
        }
    }
    
    if (m_scriptDirectories.empty()) {
        Logger::getInstance().log(Logger::Level::WARNING, "ScriptRunner: No script directories found, using current directory");
        m_scriptDirectories.push_back(".");
    }
    
    // Set primary directory for compatibility (first found directory)
    m_scriptsDirectory = m_scriptDirectories[0];
}

void ScriptRunner::eventLoop() {
    Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Entering event loop");
    
    while (m_running) {
        // Handle input (non-blocking)
        int key = getch();
        if (key != ERR) {
            if (key == KEY_RESIZE) {
                handleResize();
            } else if (key == KEY_MOUSE) {
                // Handle mouse events through UIManager
                if (m_uiManager && m_uiManager->handleMouseEvent(key)) {
                    Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: Mouse event handled by UIManager");
                    
                    // Check if there was a double-click on a script
                    std::string doubleClickedScript = m_uiManager->getDoubleClickedScript();
                    if (!doubleClickedScript.empty()) {
                        Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Double-click detected, executing script: " + doubleClickedScript);
                        
                        // Execute in the currently active output pane
                        int activePane = m_uiManager->getActivePaneIndex();
                        int targetPane = (activePane >= 1 && activePane <= 2) ? (activePane - 1) : 0; // Convert UI pane to output pane index
                        executeScript(doubleClickedScript, targetPane);
                    }
                } else {
                    Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: Mouse event not handled");
                }
            } else {
                handleKeyPress(key);
            }
        }
        
        // Update UI if enough time has passed
        if (shouldUpdateUI()) {
            updateUI();
            m_lastUpdate = std::chrono::steady_clock::now();
        }
        
        // Small delay to prevent excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(INPUT_POLL_INTERVAL_MS));
    }
    
    Logger::getInstance().log(Logger::Level::INFO, "ScriptRunner: Exiting event loop");
}

void ScriptRunner::updateUI() {
    if (!m_uiManager) return;
    
    // Clean up any finished processes first
    if (m_processManager) {
        int cleaned = m_processManager->cleanupFinishedProcesses();
        if (cleaned > 0) {
            Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: Auto-cleaned " + std::to_string(cleaned) + " finished processes");
        }
    }
    
    // Get output from both processes
    std::string output0, output1;
    if (m_processManager) {
        output0 = m_processManager->getNewOutput(0);
        output1 = m_processManager->getNewOutput(1);
    }
    
    // Update output panes
    m_uiManager->updateOutput(0, output0);
    m_uiManager->updateOutput(1, output1);
    
    // Update status bar
    std::string leftStatus = formatLeftStatus();
    std::string centerStatus = formatCenterStatus();
    std::string rightStatus = formatRightStatus();
    m_uiManager->updateStatus(leftStatus, centerStatus, rightStatus);
    
    // Refresh the display
    m_uiManager->refresh();
}

bool ScriptRunner::handleNavigationKey(int key) {
    Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: Entering navigation handler for key: " + std::to_string(key));
    
    switch (key) {
        case KEY_UP:
        case 'k': {
            int currentIndex = m_uiManager->getSelectedScriptIndex();
            Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: UP navigation - currentIndex: " + std::to_string(currentIndex));
            
            int nextIndex = findNextValidScript(currentIndex, -1);
            Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: UP navigation - nextIndex: " + std::to_string(nextIndex));
            
            if (nextIndex != currentIndex) {
                Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: UP navigation - calling highlightScript(" + std::to_string(nextIndex) + ")");
                m_uiManager->highlightScript(nextIndex);
                Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: UP navigation - highlightScript completed");
            } else {
                Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: UP navigation - no change needed");
            }
            return true;
        }
        
        case KEY_DOWN:
        case 'j': {
            int currentIndex = m_uiManager->getSelectedScriptIndex();
            Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: DOWN navigation - currentIndex: " + std::to_string(currentIndex));
            
            int nextIndex = findNextValidScript(currentIndex, 1);
            Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: DOWN navigation - nextIndex: " + std::to_string(nextIndex));
            
            if (nextIndex != currentIndex) {
                Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: DOWN navigation - calling highlightScript(" + std::to_string(nextIndex) + ")");
                m_uiManager->highlightScript(nextIndex);
                Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: DOWN navigation - highlightScript completed");
            } else {
                Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: DOWN navigation - no change needed");
            }
            return true;
        }
            
        // Arrow keys removed - they should not change active pane for script execution
            
        case KEY_HOME:
        case 'g':
            m_uiManager->highlightScript(0);
            return true;
            
        case KEY_END:
        case 'G':
            m_uiManager->highlightScript(m_availableScripts.size() - 1);
            return true;
            
        case '\t': { // Tab key - cycle only between output panes 1 and 2
            int currentPane = m_uiManager->getActivePaneIndex();
            if (currentPane == 1) {
                m_uiManager->switchPane(2); // From pane 1 to pane 2
            } else {
                m_uiManager->switchPane(1); // From any other pane (including 0) to pane 1
            }
            return true;
        }
        
        default:
            return false;
    }
}

bool ScriptRunner::handleExecutionKey(int key) {
    switch (key) {
        case '\n':
        case '\r':
        case KEY_ENTER: {
            // Execute selected script in currently selected output pane
            int selectedScript = m_uiManager->getSelectedScriptIndex();
            if (selectedScript >= 0 && selectedScript < static_cast<int>(m_availableScripts.size())) {
                std::string scriptPath = m_availableScripts[selectedScript];
                
                // Don't execute separators
                if (isSeparator(scriptPath)) {
                    return true;
                }
                
                // Get the currently active pane, default to pane 0 if script list is selected
                int activePane = m_uiManager->getActivePaneIndex();
                int targetPane = (activePane >= 1 && activePane <= 2) ? (activePane - 1) : 0; // Convert UI pane to output pane index
                
                executeScript(scriptPath, targetPane);
            }
            return true;
        }
        
        case ' ': {
            // Execute in specific pane (alternating)
            int selectedScript = m_uiManager->getSelectedScriptIndex();
            if (selectedScript >= 0 && selectedScript < static_cast<int>(m_availableScripts.size())) {
                std::string scriptPath = m_availableScripts[selectedScript];
                
                // Don't execute separators
                if (isSeparator(scriptPath)) {
                    return true;
                }
                
                static int lastPane = 1; // Start with pane 0 next time
                lastPane = (lastPane + 1) % 2;
                
                executeScript(scriptPath, lastPane);
            }
            return true;
        }
        
        case 3: // Ctrl+C
        case 't': {
            // Terminate process in active pane
            int activePane = m_uiManager->getActivePaneIndex();
            if (activePane >= 0 && activePane <= 1) {
                terminateScript(activePane);
            }
            return true;
        }
        
        default:
            return false;
    }
}

bool ScriptRunner::handleViewKey(int key) {
    int activePane = m_uiManager->getActivePaneIndex();
    
    switch (key) {
        case KEY_PPAGE: // Page Up
            if (activePane >= 1 && activePane <= 2) {
                m_uiManager->scrollUp(activePane - 1, 10); // Convert UI pane to output pane index (1,2 -> 0,1)
            }
            return true;
            
        case KEY_NPAGE: // Page Down
            if (activePane >= 1 && activePane <= 2) {
                m_uiManager->scrollDown(activePane - 1, 10); // Convert UI pane to output pane index (1,2 -> 0,1)
            }
            return true;
            
        case 12: // Ctrl+L
            if (activePane >= 1 && activePane <= 2) {
                m_uiManager->clearOutputPane(activePane - 1); // Convert UI pane to output pane index (1,2 -> 0,1)
            }
            return true;
            
        case 'r':
        case KEY_F(5): // F5
            refreshScriptList();
            return true;
            
        case '\t': // Tab - cycle between output panes only
            {
                int currentPane = m_uiManager->getActivePaneIndex();
                if (currentPane == 1) {
                    m_uiManager->switchPane(2); // From pane 1 to pane 2
                } else {
                    m_uiManager->switchPane(1); // From any other pane to pane 1
                }
            }
            return true;
            
        // Directional resize keys - Only Ctrl+Left/Right for horizontal resizing
        case 545:      // Ctrl+Left Arrow (expand active pane left)
        case 546:      // Alternative Ctrl+Left 
            if (m_uiManager->resizeActivePaneLeft()) {
                Logger::getInstance().log(Logger::Level::INFO, "Active pane expanded left");
            }
            return true;
            
        case 560:       // Ctrl+Right Arrow (expand active pane right)
        case 561:       // Alternative Ctrl+Right
            if (m_uiManager->resizeActivePaneRight()) {
                Logger::getInstance().log(Logger::Level::INFO, "Active pane expanded right");
            }
            return true;
            
        // Legacy resize keys (kept for compatibility)
        case KEY_SLEFT: // Shift+Left (backup option)
        case '[':       // Simple alternative: [ key for narrower
            if (m_uiManager->resizeScriptListNarrower()) {
                Logger::getInstance().log(Logger::Level::INFO, "Script list resized narrower");
            }
            return true;
            
        case KEY_SRIGHT: // Shift+Right (backup option)
        case ']':        // Simple alternative: ] key for wider
            if (m_uiManager->resizeScriptListWider()) {
                Logger::getInstance().log(Logger::Level::INFO, "Script list resized wider");
            }
            return true;
            
        // Output pane 1 resize keys (legacy)
        case ',':  // Comma - make output pane 1 narrower
            if (m_uiManager->resizeOutputPane1Narrower()) {
                Logger::getInstance().log(Logger::Level::INFO, "Output pane 1 resized narrower");
            }
            return true;
            
        case '.':  // Period - make output pane 1 wider
            if (m_uiManager->resizeOutputPane1Wider()) {
                Logger::getInstance().log(Logger::Level::INFO, "Output pane 1 resized wider");
            }
            return true;
            
        default:
            return false;
    }
}

bool ScriptRunner::handleApplicationKey(int key) {
    switch (key) {
        case 17: // Ctrl+Q
        case 27: // Escape
            shutdown();
            return true;
            
        case KEY_F(1): // F1
        case '?':
            m_uiManager->showHelp();
            return true;
            
        default:
            return false;
    }
}

int ScriptRunner::getAvailablePane() const {
    if (!m_processManager) return 0;
    
    // Check pane 0 first
    if (!m_processManager->isRunning(0)) {
        return 0;
    }
    
    // Check pane 1
    if (!m_processManager->isRunning(1)) {
        return 1;
    }
    
    // Both panes are busy
    return -1;
}

std::string ScriptRunner::formatLeftStatus() const {
    std::string status = "Scripts: " + std::to_string(m_availableScripts.size());
    
    if (m_processManager) {
        int runningCount = 0;
        if (m_processManager->isRunning(0)) runningCount++;
        if (m_processManager->isRunning(1)) runningCount++;
        
        if (runningCount > 0) {
            status += " | Running: " + std::to_string(runningCount);
        }
    }
    
    return status;
}

std::string ScriptRunner::formatCenterStatus() const {
    int selectedScript = m_uiManager->getSelectedScriptIndex();
    if (selectedScript >= 0 && selectedScript < static_cast<int>(m_availableScripts.size())) {
        return "Selected: " + m_availableScripts[selectedScript];
    }
    return "";
}

std::string ScriptRunner::formatRightStatus() const {
    auto uptime = getUptime();
    return "Uptime: " + formatDuration(uptime);
}

bool ScriptRunner::shouldUpdateUI() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastUpdate);
    return elapsed.count() >= UI_UPDATE_INTERVAL_MS;
}

std::chrono::seconds ScriptRunner::getUptime() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime);
}

std::string ScriptRunner::formatDuration(std::chrono::seconds duration) const {
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration % std::chrono::hours(1));
    auto seconds = duration % std::chrono::minutes(1);
    
    std::ostringstream oss;
    if (hours.count() > 0) {
        oss << hours.count() << "h";
    }
    if (minutes.count() > 0 || hours.count() > 0) {
        oss << minutes.count() << "m";
    }
    oss << seconds.count() << "s";
    
    return oss.str();
}

std::string ScriptRunner::getCurrentTimeString() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto tm_now = *std::localtime(&time_t_now);
    
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << tm_now.tm_hour << ":"
        << std::setfill('0') << std::setw(2) << tm_now.tm_min << ":"
        << std::setfill('0') << std::setw(2) << tm_now.tm_sec;
    
    return oss.str();
}

bool ScriptRunner::isSeparator(const std::string& item) const {
    return item.find("---") == 0;
}

int ScriptRunner::findNextValidScript(int currentIndex, int direction) const {
    Logger::getInstance().log(Logger::Level::DEBUG, 
        "ScriptRunner: findNextValidScript called - currentIndex: " + std::to_string(currentIndex) + 
        ", direction: " + std::to_string(direction) + ", scriptCount: " + std::to_string(m_availableScripts.size()));
    
    if (m_availableScripts.empty()) {
        Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: findNextValidScript - empty script list, returning -1");
        return -1;
    }
    
    int size = static_cast<int>(m_availableScripts.size());
    int nextIndex = currentIndex;
    
    Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: findNextValidScript - starting search from index " + std::to_string(nextIndex));
    
    // Search for next valid script
    for (int i = 0; i < size; i++) {
        nextIndex += direction;
        
        // Handle wrapping
        if (nextIndex >= size) {
            nextIndex = 0;
            Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: findNextValidScript - wrapped to beginning: " + std::to_string(nextIndex));
        } else if (nextIndex < 0) {
            nextIndex = size - 1;
            Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: findNextValidScript - wrapped to end: " + std::to_string(nextIndex));
        }
        
        Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: findNextValidScript - checking index " + std::to_string(nextIndex) + 
            " (item: '" + (nextIndex < static_cast<int>(m_availableScripts.size()) ? m_availableScripts[nextIndex] : "OUT_OF_RANGE") + "')");
        
        // Check bounds to prevent crashes
        if (nextIndex < 0 || nextIndex >= static_cast<int>(m_availableScripts.size())) {
            Logger::getInstance().log(Logger::Level::ERROR, "ScriptRunner: findNextValidScript - INDEX OUT OF BOUNDS: " + std::to_string(nextIndex) + 
                " (size: " + std::to_string(m_availableScripts.size()) + ")");
            return currentIndex;
        }
        
        // Check if this is a valid script (not a separator)
        bool isSep = isSeparator(m_availableScripts[nextIndex]);
        Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: findNextValidScript - isSeparator(" + std::to_string(nextIndex) + "): " + (isSep ? "true" : "false"));
        
        if (!isSep) {
            Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: findNextValidScript - found valid script at index " + std::to_string(nextIndex));
            return nextIndex;
        }
        
        // If we've come full circle, return current index
        if (nextIndex == currentIndex) {
            Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: findNextValidScript - full circle reached, returning current index");
            break;
        }
    }
    
    Logger::getInstance().log(Logger::Level::DEBUG, "ScriptRunner: findNextValidScript - no valid script found, returning current index " + std::to_string(currentIndex));
    return currentIndex; // No valid script found, stay where we are
}
