/**
 * @file UIManager.cpp
 * @brief Implementation of the UIManager class
 * 
 * This file contains the implementation of the terminal user interface
 * management functionality using ncurses.
 * 
 * @author GitHub Copilot
 * @date 2025-07-19
 * @version 1.0.0
 */

#include "../include/UIManager.h"
#include "../include/Logger.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <regex>
#include <chrono>

UIManager::UIManager()
    : m_mainWindow(nullptr)
    , m_scriptListPane(nullptr)
    , m_outputPane1(nullptr)
    , m_outputPane2(nullptr)
    , m_statusBar(nullptr)
    , m_helpWindow(nullptr)
    , m_terminalWidth(0)
    , m_terminalHeight(0)
    , m_scriptListWidth(0)
    , m_outputPaneWidth(0)
    , m_outputPane1Width(0)
    , m_outputPane2Width(0)
    , m_statusBarHeight(STATUS_BAR_HEIGHT)
    , m_currentScriptListWidthRatio(DEFAULT_SCRIPT_LIST_WIDTH_RATIO)
    , m_outputPane1WidthRatio(DEFAULT_OUTPUT_PANE1_RATIO)
    , m_selectedScript(-1)
    , m_activePane(1)  // Start with output pane 1 selected
    , m_helpVisible(false)
    , m_initialized(false)
    , m_scriptListScroll(0)
    , m_lastClickTime(std::chrono::steady_clock::now())
    , m_lastClickX(-1)
    , m_lastClickY(-1)
    , m_lastClickedScript(-1) {
    
    // Initialize scroll positions
    m_scrollPosition[0] = 0;
    m_scrollPosition[1] = 0;
    
    LOG_DEBUG("UIManager constructed");
}

UIManager::~UIManager() {
    cleanup();
    LOG_DEBUG("UIManager destroyed");
}

// Helper function to strip ANSI escape sequences
std::string UIManager::stripAnsiCodes(const std::string& input) const {
    static const std::regex ansiRegex(R"(\x1b\[[0-9;]*[mK])");
    return std::regex_replace(input, ansiRegex, "");
}

// Helper function to render text with ANSI color codes
void UIManager::renderAnsiText(WINDOW* window, const std::string& text, int y, int x, int maxWidth) {
    if (!window) return;
    
    int currentX = x;
    int currentColorPair = static_cast<int>(ColorPair::DEFAULT);
    bool boldActive = false;
    
    // First, clear the line to ensure no leftover characters
    wmove(window, y, x);
    for (int i = 0; i < maxWidth; i++) {
        waddch(window, ' ');
    }
    
    // Regular expression to match ANSI escape sequences
    static const std::regex ansiRegex(R"(\x1b\[([0-9;]*)m)");
    
    auto it = std::sregex_iterator(text.begin(), text.end(), ansiRegex);
    auto end = std::sregex_iterator();
    
    size_t lastPos = 0;
    
    for (auto match = it; match != end; ++match) {
        // Print text before this escape sequence
        std::string textPart = text.substr(lastPos, match->position() - lastPos);
        for (char c : textPart) {
            if (currentX >= x + maxWidth) break;
            // Only render printable characters
            if (c >= 32 && c <= 126) {
                if (has_colors()) {
                    wattron(window, COLOR_PAIR(currentColorPair));
                    if (boldActive) wattron(window, A_BOLD);
                }
                mvwaddch(window, y, currentX, c);
                if (has_colors()) {
                    if (boldActive) wattroff(window, A_BOLD);
                    wattroff(window, COLOR_PAIR(currentColorPair));
                }
            }
            currentX++;
        }
        
        if (currentX >= x + maxWidth) break;
        
        // Parse the escape sequence
        std::string codes = match->str(1);
        if (codes.empty()) {
            // Reset to default
            currentColorPair = static_cast<int>(ColorPair::DEFAULT);
            boldActive = false;
        } else {
            // Parse color codes
            std::istringstream ss(codes);
            std::string code;
            while (std::getline(ss, code, ';')) {
                if (code.empty()) continue;
                try {
                    int codeNum = std::stoi(code);
                    switch (codeNum) {
                        case 0:  // Reset
                            currentColorPair = static_cast<int>(ColorPair::DEFAULT);
                            boldActive = false;
                            break;
                        case 1:  // Bold
                            boldActive = true;
                            break;
                        case 22: // Normal intensity
                            boldActive = false;
                            break;
                        case 30: currentColorPair = static_cast<int>(ColorPair::ANSI_BLACK); break;
                        case 31: currentColorPair = static_cast<int>(ColorPair::ANSI_RED); break;
                        case 32: currentColorPair = static_cast<int>(ColorPair::ANSI_GREEN); break;
                        case 33: currentColorPair = static_cast<int>(ColorPair::ANSI_YELLOW); break;
                        case 34: currentColorPair = static_cast<int>(ColorPair::ANSI_BLUE); break;
                        case 35: currentColorPair = static_cast<int>(ColorPair::ANSI_MAGENTA); break;
                        case 36: currentColorPair = static_cast<int>(ColorPair::ANSI_CYAN); break;
                        case 37: currentColorPair = static_cast<int>(ColorPair::ANSI_WHITE); break;
                        case 90: currentColorPair = static_cast<int>(ColorPair::ANSI_BRIGHT_BLACK); break;
                        case 91: currentColorPair = static_cast<int>(ColorPair::ANSI_BRIGHT_RED); break;
                        case 92: currentColorPair = static_cast<int>(ColorPair::ANSI_BRIGHT_GREEN); break;
                        case 93: currentColorPair = static_cast<int>(ColorPair::ANSI_BRIGHT_YELLOW); break;
                        case 94: currentColorPair = static_cast<int>(ColorPair::ANSI_BRIGHT_BLUE); break;
                        case 95: currentColorPair = static_cast<int>(ColorPair::ANSI_BRIGHT_MAGENTA); break;
                        case 96: currentColorPair = static_cast<int>(ColorPair::ANSI_BRIGHT_CYAN); break;
                        case 97: currentColorPair = static_cast<int>(ColorPair::ANSI_BRIGHT_WHITE); break;
                        // Add more codes as needed (background colors, etc.)
                    }
                } catch (const std::exception&) {
                    // Ignore invalid color codes
                }
            }
        }
        
        lastPos = match->position() + match->length();
    }
    
    // Print remaining text after last escape sequence
    std::string remainingText = text.substr(lastPos);
    for (char c : remainingText) {
        if (currentX >= x + maxWidth) break;
        // Only render printable characters
        if (c >= 32 && c <= 126) {
            if (has_colors()) {
                wattron(window, COLOR_PAIR(currentColorPair));
                if (boldActive) wattron(window, A_BOLD);
            }
            mvwaddch(window, y, currentX, c);
            if (has_colors()) {
                if (boldActive) wattroff(window, A_BOLD);
                wattroff(window, COLOR_PAIR(currentColorPair));
            }
        }
        currentX++;
    }
}

// Helper function to check if a script list item is a separator
bool UIManager::isSeparator(const std::string& item) {
    return item.find("---") == 0;
}

// Helper function to wrap text to fit in given width
std::vector<std::string> UIManager::wrapText(const std::string& text, int maxWidth) const {
    std::vector<std::string> lines;
    if (text.empty() || maxWidth <= 0) {
        return lines;
    }
    
    // Strip ANSI codes for length calculation
    std::string cleanText = stripAnsiCodes(text);
    
    if (static_cast<int>(cleanText.length()) <= maxWidth) {
        lines.push_back(text); // Keep original with ANSI codes if it fits
        return lines;
    }
    
    // For now, if text needs wrapping, strip ANSI codes to avoid complex parsing
    // TODO: Implement proper ANSI-aware wrapping that preserves color codes
    std::istringstream words(cleanText);
    std::string word;
    std::string currentLine;
    
    while (words >> word) {
        if (currentLine.empty()) {
            currentLine = word;
        } else if (static_cast<int>(currentLine.length() + 1 + word.length()) <= maxWidth) {
            currentLine += " " + word;
        } else {
            lines.push_back(currentLine);
            currentLine = word;
        }
    }
    
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    
    return lines;
}

bool UIManager::initialize() {
    LOG_INFO("Initializing UI subsystem");
    
    try {
        // Initialize ncurses
        m_mainWindow = initscr();
        if (!m_mainWindow) {
            throw UIException("Failed to initialize ncurses");
        }
        
        // Get terminal dimensions
        getmaxyx(stdscr, m_terminalHeight, m_terminalWidth);
        
        // Check minimum terminal size
        if (m_terminalWidth < MIN_TERMINAL_WIDTH || m_terminalHeight < MIN_TERMINAL_HEIGHT) {
            cleanup();
            throw UIException("Terminal too small (minimum " + 
                            std::to_string(MIN_TERMINAL_WIDTH) + "x" + 
                            std::to_string(MIN_TERMINAL_HEIGHT) + " required)");
        }
        
        // Configure ncurses
        noecho();           // Don't echo pressed keys
        cbreak();           // Disable line buffering
        keypad(stdscr, TRUE); // Enable special keys
        nodelay(stdscr, TRUE); // Make getch() non-blocking
        curs_set(0);        // Hide cursor
        
        // Enable mouse support
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
        mouseinterval(0);   // No click delay
        
        // Initialize colors if supported
        if (has_colors()) {
            start_color();
            initializeColors();
        }
        
        // Create layout
        calculateLayout();
        createLayout();
        
        m_initialized = true;
        LOG_INFO("UI initialization completed successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("UI initialization failed: " + std::string(e.what()));
        cleanup();
        return false;
    }
}

void UIManager::cleanup() {
    if (!m_initialized) {
        return;
    }
    
    LOG_DEBUG("Cleaning up UI resources");
    
    // Delete all windows
    safeDeleteWindow(m_helpWindow);
    safeDeleteWindow(m_statusBar);
    safeDeleteWindow(m_outputPane2);
    safeDeleteWindow(m_outputPane1);
    safeDeleteWindow(m_scriptListPane);
    
    // End ncurses
    if (m_mainWindow) {
        endwin();
        m_mainWindow = nullptr;
    }
    
    m_initialized = false;
    LOG_DEBUG("UI cleanup completed");
}

void UIManager::createLayout() {
    LOG_DEBUG("Creating UI layout");
    
    // Delete existing windows
    safeDeleteWindow(m_scriptListPane);
    safeDeleteWindow(m_outputPane1);
    safeDeleteWindow(m_outputPane2);
    safeDeleteWindow(m_statusBar);
    
    // Calculate layout
    calculateLayout();
    
    // Create script list pane
    m_scriptListPane = newwin(
        m_terminalHeight - m_statusBarHeight,  // height
        m_scriptListWidth,                     // width
        0,                                     // start y
        0                                      // start x
    );
    
    // Create output pane 1
    m_outputPane1 = newwin(
        m_terminalHeight - m_statusBarHeight,  // height
        m_outputPane1Width,                    // width
        0,                                     // start y
        m_scriptListWidth                      // start x
    );
    
    // Create output pane 2
    m_outputPane2 = newwin(
        m_terminalHeight - m_statusBarHeight,  // height
        m_outputPane2Width,                    // width
        0,                                     // start y
        m_scriptListWidth + m_outputPane1Width // start x
    );
    
    // Create status bar
    m_statusBar = newwin(
        m_statusBarHeight,                     // height
        m_terminalWidth,                       // width
        m_terminalHeight - m_statusBarHeight,  // start y
        0                                      // start x
    );
    
    // Check window creation
    if (!m_scriptListPane || !m_outputPane1 || !m_outputPane2 || !m_statusBar) {
        throw UIException("Failed to create windows");
    }
    
    // Enable scrolling for output panes
    scrollok(m_outputPane1, TRUE);
    scrollok(m_outputPane2, TRUE);
    
    // Draw borders and headers
    drawBorders();
    drawHeaders();
    updateActivePaneBorder();
    
    // Initial refresh
    refresh();
    
    LOG_DEBUG("UI layout created successfully");
}

void UIManager::handleResize() {
    LOG_DEBUG("Handling terminal resize");
    
    // Get new terminal dimensions
    getmaxyx(stdscr, m_terminalHeight, m_terminalWidth);
    
    // Check minimum size
    if (m_terminalWidth < MIN_TERMINAL_WIDTH || m_terminalHeight < MIN_TERMINAL_HEIGHT) {
        LOG_WARNING("Terminal resized below minimum dimensions");
        return;
    }
    
    // Recreate layout
    createLayout();
    
    // Redraw content
    drawScriptList();
    drawOutputPane(0);
    drawOutputPane(1);
    updateStatus("Terminal resized", "", "");
    
    refresh();
}

void UIManager::updateScriptList(const std::vector<std::string>& scripts) {
    m_scriptList = scripts;
    
    // Adjust selection if necessary
    if (m_selectedScript >= static_cast<int>(scripts.size())) {
        m_selectedScript = scripts.empty() ? -1 : static_cast<int>(scripts.size()) - 1;
    }
    
    // If no script is selected but scripts are available, select the first valid one
    if (m_selectedScript < 0 && !scripts.empty()) {
        m_selectedScript = 0;
        // Find first non-separator script
        for (int i = 0; i < static_cast<int>(scripts.size()); i++) {
            if (!isSeparator(scripts[i])) {
                m_selectedScript = i;
                break;
            }
        }
    }
    
    // If current selection is a separator, find next valid script
    if (m_selectedScript >= 0 && m_selectedScript < static_cast<int>(scripts.size()) && 
        isSeparator(scripts[m_selectedScript])) {
        // Find next valid script
        for (int i = m_selectedScript + 1; i < static_cast<int>(scripts.size()); i++) {
            if (!isSeparator(scripts[i])) {
                m_selectedScript = i;
                break;
            }
        }
        // If none found after, search before
        if (isSeparator(scripts[m_selectedScript])) {
            for (int i = m_selectedScript - 1; i >= 0; i--) {
                if (!isSeparator(scripts[i])) {
                    m_selectedScript = i;
                    break;
                }
            }
        }
    }
    
    drawScriptList();
    if (m_scriptListPane) wrefresh(m_scriptListPane);
}

void UIManager::updateOutput(int paneIndex, const std::string& output) {
    validatePaneIndex(paneIndex);
    
    if (output.empty()) {
        return;
    }

    // Check if user is currently at or near the bottom before adding new content
    int visibleLines = m_terminalHeight - 4; // Account for borders and header
    int totalWrappedLines = calculateWrappedLineCount(paneIndex);
    int maxScroll = std::max(0, totalWrappedLines - visibleLines);
    bool wasAtBottom = (m_scrollPosition[paneIndex] >= maxScroll - 2); // Allow 2 lines tolerance
    
    // Split output into lines
    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        m_outputBuffer[paneIndex].push_back(line);
        
        // Limit buffer size
        if (m_outputBuffer[paneIndex].size() > MAX_OUTPUT_LINES) {
            m_outputBuffer[paneIndex].erase(m_outputBuffer[paneIndex].begin());
            // Adjust scroll position when removing from top
            if (m_scrollPosition[paneIndex] > 0) {
                m_scrollPosition[paneIndex]--;
            }
        }
    }
    
    // If user was at bottom, auto-scroll to keep the latest content visible
    if (wasAtBottom) {
        int newTotalWrappedLines = calculateWrappedLineCount(paneIndex);
        int newMaxScroll = std::max(0, newTotalWrappedLines - visibleLines);
        m_scrollPosition[paneIndex] = newMaxScroll;
    }
    
    // Only redraw and refresh this specific pane
    drawOutputPane(paneIndex);
    WINDOW* pane = (paneIndex == 0) ? m_outputPane1 : m_outputPane2;
    if (pane) wrefresh(pane);
}

void UIManager::updateStatus(const std::string& leftStatus, 
                           const std::string& centerStatus,
                           const std::string& rightStatus) {
    if (!m_statusBar) return;
    
    // Clear status bar
    werase(m_statusBar);
    
    // Set status bar color
    if (has_colors()) {
        wattron(m_statusBar, COLOR_PAIR(static_cast<int>(ColorPair::STATUS)));
    }
    
    // Draw left status
    mvwprintw(m_statusBar, 0, 1, "%s", leftStatus.c_str());
    
    // Draw center status
    if (!centerStatus.empty()) {
        int centerX = (m_terminalWidth - static_cast<int>(centerStatus.length())) / 2;
        if (centerX > 0) {
            mvwprintw(m_statusBar, 0, centerX, "%s", centerStatus.c_str());
        }
    }
    
    // Draw right status
    if (!rightStatus.empty()) {
        int rightX = m_terminalWidth - static_cast<int>(rightStatus.length()) - 1;
        if (rightX > 0) {
            mvwprintw(m_statusBar, 0, rightX, "%s", rightStatus.c_str());
        }
    }
    
    if (has_colors()) {
        wattroff(m_statusBar, COLOR_PAIR(static_cast<int>(ColorPair::STATUS)));
    }
    
    wrefresh(m_statusBar);
}

int UIManager::getInput() {
    return getch();
}

bool UIManager::handleMouseEvent(int key) {
    if (key != KEY_MOUSE) {
        return false;
    }
    
    MEVENT event;
    if (getmouse(&event) != OK) {
        LOG_DEBUG("Failed to get mouse event");
        return false;
    }
    
    LOG_DEBUG("Mouse event: x=" + std::to_string(event.x) + ", y=" + std::to_string(event.y) + 
             ", bstate=0x" + std::to_string(event.bstate));
    
    // Clear any previous double-click script
    m_doubleClickedScript.clear();
    
    // Check if mouse is in output pane 1
    if (event.x >= m_scriptListWidth && event.x < m_scriptListWidth + m_outputPane1Width &&
        event.y >= 0 && event.y < m_terminalHeight - m_statusBarHeight) {
        
        // Mouse wheel up in output pane 1
        if (event.bstate & BUTTON4_PRESSED) {
            LOG_DEBUG("Mouse wheel up in output pane 1");
            scrollUp(0, 3); // Scroll up by 3 lines
            return true;
        }
        // Mouse wheel down in output pane 1
        else if (event.bstate & BUTTON5_PRESSED) {
            LOG_DEBUG("Mouse wheel down in output pane 1");
            scrollDown(0, 3); // Scroll down by 3 lines
            return true;
        }
        // Click to focus pane 1
        else if (event.bstate & BUTTON1_PRESSED) {
            LOG_DEBUG("Click in output pane 1 - switching focus");
            switchPane(1);
            return true;
        }
    }
    // Check if mouse is in output pane 2
    else if (event.x >= m_scriptListWidth + m_outputPane1Width && event.x < m_terminalWidth &&
             event.y >= 0 && event.y < m_terminalHeight - m_statusBarHeight) {
        
        // Mouse wheel up in output pane 2
        if (event.bstate & BUTTON4_PRESSED) {
            LOG_DEBUG("Mouse wheel up in output pane 2");
            scrollUp(1, 3); // Scroll up by 3 lines
            return true;
        }
        // Mouse wheel down in output pane 2
        else if (event.bstate & BUTTON5_PRESSED) {
            LOG_DEBUG("Mouse wheel down in output pane 2");
            scrollDown(1, 3); // Scroll down by 3 lines
            return true;
        }
        // Click to focus pane 2
        else if (event.bstate & BUTTON1_PRESSED) {
            LOG_DEBUG("Click in output pane 2 - switching focus");
            switchPane(2);
            return true;
        }
    }
    // Check if mouse is in script list pane
    else if (event.x >= 0 && event.x < m_scriptListWidth &&
             event.y >= 0 && event.y < m_terminalHeight - m_statusBarHeight) {
        
        // Handle clicks in script list
        if (event.bstate & BUTTON1_PRESSED) {
            // Calculate clicked script index
            int clickedLine = event.y - 1; // Adjust for border
            int scriptIndex = m_scriptListScroll + clickedLine;
            
            if (scriptIndex >= 0 && scriptIndex < static_cast<int>(m_scriptList.size()) &&
                !isSeparator(m_scriptList[scriptIndex])) {
                
                // Check for double-click
                auto currentTime = std::chrono::steady_clock::now();
                auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_lastClickTime).count();
                
                // Double-click detection: same script, within 500ms, similar position
                if (scriptIndex == m_lastClickedScript && 
                    timeDiff < 500 && 
                    abs(event.x - m_lastClickX) < 3 && 
                    abs(event.y - m_lastClickY) < 3) {
                    
                    LOG_DEBUG("Double-click detected on script: " + m_scriptList[scriptIndex]);
                    m_doubleClickedScript = m_scriptList[scriptIndex];
                    
                    // Don't change active pane for script list clicks - keep current selection
                    highlightScript(scriptIndex);
                    return true;
                } else {
                    // Single click - just select the script, don't change active pane
                    LOG_DEBUG("Single click on script: " + m_scriptList[scriptIndex]);
                    highlightScript(scriptIndex);
                    
                    // Update click tracking
                    m_lastClickTime = currentTime;
                    m_lastClickX = event.x;
                    m_lastClickY = event.y;
                    m_lastClickedScript = scriptIndex;
                    return true;
                }
            }
        }
    }
    
    return false;
}

std::string UIManager::getDoubleClickedScript() {
    std::string script = m_doubleClickedScript;
    m_doubleClickedScript.clear(); // Clear after retrieval
    return script;
}

void UIManager::highlightScript(int index) {
    LOG_DEBUG("UIManager::highlightScript called with index: " + std::to_string(index) + 
             ", scriptList size: " + std::to_string(m_scriptList.size()) + 
             ", current selection: " + std::to_string(m_selectedScript));
    
    if (index >= 0 && index < static_cast<int>(m_scriptList.size())) {
        LOG_DEBUG("UIManager::highlightScript - index valid, setting selection to " + std::to_string(index));
        m_selectedScript = index;
        
        LOG_DEBUG("UIManager::highlightScript - calling ensureSelectionVisible");
        ensureSelectionVisible();
        LOG_DEBUG("UIManager::highlightScript - ensureSelectionVisible completed");
    } else {
        LOG_WARNING("UIManager::highlightScript - invalid index " + std::to_string(index) + 
                   " (size: " + std::to_string(m_scriptList.size()) + "), setting to -1");
        m_selectedScript = -1;
    }
    
    LOG_DEBUG("UIManager::highlightScript - calling drawScriptList");
    drawScriptList();
    LOG_DEBUG("UIManager::highlightScript - drawScriptList completed");
    
    if (m_scriptListPane) {
        LOG_DEBUG("UIManager::highlightScript - refreshing script list pane");
        wrefresh(m_scriptListPane);
        LOG_DEBUG("UIManager::highlightScript - refresh completed");
    } else {
        LOG_ERROR("UIManager::highlightScript - m_scriptListPane is null!");
    }
    
    LOG_DEBUG("UIManager::highlightScript - completed successfully");
}

void UIManager::switchPane(int paneIndex) {
    if (paneIndex < 0 || paneIndex > 2) {
        throw std::invalid_argument("Invalid pane index: " + std::to_string(paneIndex));
    }
    
    if (!m_initialized) {
        LOG_WARNING("Cannot switch panes - UI not initialized");
        return;
    }
    
    m_activePane = paneIndex;
    updateActivePaneBorder();
    
    // Refresh only the affected windows with null checks
    if (m_scriptListPane) wrefresh(m_scriptListPane);
    if (m_outputPane1) wrefresh(m_outputPane1);
    if (m_outputPane2) wrefresh(m_outputPane2);
}

void UIManager::refresh() {
    // Redraw everything without clearing screen
    drawBorders();
    drawHeaders();
    drawScriptList();
    drawOutputPane(0);
    drawOutputPane(1);
    updateActivePaneBorder();
    
    // Refresh all windows in proper order
    if (m_scriptListPane) wrefresh(m_scriptListPane);
    if (m_outputPane1) wrefresh(m_outputPane1);
    if (m_outputPane2) wrefresh(m_outputPane2);
    if (m_statusBar) wrefresh(m_statusBar);
    
    if (m_helpVisible && m_helpWindow) {
        drawHelpOverlay();
        wrefresh(m_helpWindow);
    }
}

void UIManager::showHelp() {
    m_helpVisible = !m_helpVisible;
    
    if (m_helpVisible) {
        drawHelpOverlay();
        wrefresh(m_helpWindow);
    } else {
        safeDeleteWindow(m_helpWindow);
        refresh();
    }
}

void UIManager::clearOutputPane(int paneIndex) {
    validatePaneIndex(paneIndex);
    
    m_outputBuffer[paneIndex].clear();
    m_scrollPosition[paneIndex] = 0;
    
    // Only redraw and refresh this specific pane
    drawOutputPane(paneIndex);
    WINDOW* pane = (paneIndex == 0) ? m_outputPane1 : m_outputPane2;
    if (pane) wrefresh(pane);
}

void UIManager::scrollUp(int paneIndex, int lines) {
    validatePaneIndex(paneIndex);
    
    m_scrollPosition[paneIndex] = std::max(0, m_scrollPosition[paneIndex] - lines);
    
    // Only redraw and refresh this specific pane
    drawOutputPane(paneIndex);
    WINDOW* pane = (paneIndex == 0) ? m_outputPane1 : m_outputPane2;
    if (pane) wrefresh(pane);
}

void UIManager::scrollDown(int paneIndex, int lines) {
    validatePaneIndex(paneIndex);
    
    int visibleLines = m_terminalHeight - 4; // Account for borders and header
    int totalWrappedLines = calculateWrappedLineCount(paneIndex);
    int maxScroll = std::max(0, totalWrappedLines - visibleLines);
    
    if (maxScroll > 0) {
        m_scrollPosition[paneIndex] = std::min(maxScroll, m_scrollPosition[paneIndex] + lines);
        
        // Only redraw and refresh this specific pane
        drawOutputPane(paneIndex);
        WINDOW* pane = (paneIndex == 0) ? m_outputPane1 : m_outputPane2;
        if (pane) wrefresh(pane);
    }
}

// Private method implementations

void UIManager::initializeColors() {
    init_pair(static_cast<int>(ColorPair::DEFAULT), COLOR_WHITE, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::HEADER), COLOR_CYAN, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::SELECTED), COLOR_BLACK, COLOR_YELLOW);
    init_pair(static_cast<int>(ColorPair::RUNNING), COLOR_GREEN, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ERROR), COLOR_RED, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::SUCCESS), COLOR_GREEN, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::STATUS), COLOR_WHITE, COLOR_BLUE);
    init_pair(static_cast<int>(ColorPair::BORDER), COLOR_WHITE, COLOR_BLACK);
    
    // ANSI color pairs (standard colors)
    init_pair(static_cast<int>(ColorPair::ANSI_BLACK), COLOR_BLACK, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ANSI_RED), COLOR_RED, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ANSI_GREEN), COLOR_GREEN, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ANSI_YELLOW), COLOR_YELLOW, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ANSI_BLUE), COLOR_BLUE, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ANSI_MAGENTA), COLOR_MAGENTA, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ANSI_CYAN), COLOR_CYAN, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ANSI_WHITE), COLOR_WHITE, COLOR_BLACK);
    
    // ANSI bright colors (if terminal supports them, use high intensity)
    init_pair(static_cast<int>(ColorPair::ANSI_BRIGHT_BLACK), COLOR_BLACK, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ANSI_BRIGHT_RED), COLOR_RED, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ANSI_BRIGHT_GREEN), COLOR_GREEN, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ANSI_BRIGHT_YELLOW), COLOR_YELLOW, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ANSI_BRIGHT_BLUE), COLOR_BLUE, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ANSI_BRIGHT_MAGENTA), COLOR_MAGENTA, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ANSI_BRIGHT_CYAN), COLOR_CYAN, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::ANSI_BRIGHT_WHITE), COLOR_WHITE, COLOR_BLACK);
}

void UIManager::calculateLayout() {
    // Calculate script list width using current ratio
    m_scriptListWidth = (m_terminalWidth * m_currentScriptListWidthRatio) / 100;
    
    // Calculate remaining width for output panes
    int remainingWidth = m_terminalWidth - m_scriptListWidth;
    
    // Calculate individual output pane widths based on ratios
    m_outputPane1Width = (remainingWidth * m_outputPane1WidthRatio) / 100;
    m_outputPane2Width = remainingWidth - m_outputPane1Width;
    
    // Keep the old m_outputPaneWidth for compatibility (use pane1 width)
    m_outputPaneWidth = m_outputPane1Width;
    
    LOG_DEBUG("Layout calculated: terminal=" + std::to_string(m_terminalWidth) + "x" + 
              std::to_string(m_terminalHeight) + ", script_list=" + 
              std::to_string(m_scriptListWidth) + " (ratio=" + 
              std::to_string(m_currentScriptListWidthRatio) + "%), output_pane1=" + 
              std::to_string(m_outputPane1Width) + ", output_pane2=" + 
              std::to_string(m_outputPane2Width) + " (pane1_ratio=" + 
              std::to_string(m_outputPane1WidthRatio) + "%)");
}

void UIManager::drawBorders() {
    // Draw borders for all panes
    if (m_scriptListPane) box(m_scriptListPane, 0, 0);
    if (m_outputPane1) box(m_outputPane1, 0, 0);
    if (m_outputPane2) box(m_outputPane2, 0, 0);
}

void UIManager::drawHeaders() {
    // Script list header
    if (m_scriptListPane && has_colors()) {
        wattron(m_scriptListPane, COLOR_PAIR(static_cast<int>(ColorPair::HEADER)));
        mvwprintw(m_scriptListPane, 0, 2, " Scripts ");
        wattroff(m_scriptListPane, COLOR_PAIR(static_cast<int>(ColorPair::HEADER)));
    }
    
    // Output pane 1 header
    if (m_outputPane1 && has_colors()) {
        wattron(m_outputPane1, COLOR_PAIR(static_cast<int>(ColorPair::HEADER)));
        mvwprintw(m_outputPane1, 0, 2, " Output 1 ");
        wattroff(m_outputPane1, COLOR_PAIR(static_cast<int>(ColorPair::HEADER)));
    }
    
    // Output pane 2 header
    if (m_outputPane2 && has_colors()) {
        wattron(m_outputPane2, COLOR_PAIR(static_cast<int>(ColorPair::HEADER)));
        mvwprintw(m_outputPane2, 0, 2, " Output 2 ");
        wattroff(m_outputPane2, COLOR_PAIR(static_cast<int>(ColorPair::HEADER)));
    }
}

void UIManager::validatePaneIndex(int paneIndex) const {
    if (paneIndex < 0 || paneIndex > 1) {
        throw std::invalid_argument("Invalid pane index: " + std::to_string(paneIndex) + 
                                   " (must be 0 or 1)");
    }
}

std::string UIManager::getBaseName(const std::string& fullPath) const {
    size_t lastSlash = fullPath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        return fullPath.substr(lastSlash + 1);
    }
    return fullPath;
}

void UIManager::ensureSelectionVisible() {
    LOG_DEBUG("UIManager::ensureSelectionVisible called - selectedScript: " + std::to_string(m_selectedScript) + 
             ", scriptList size: " + std::to_string(m_scriptList.size()) + 
             ", current scroll: " + std::to_string(m_scriptListScroll));
    
    if (m_selectedScript < 0 || m_scriptList.empty()) {
        LOG_DEBUG("UIManager::ensureSelectionVisible - early return (invalid selection or empty list)");
        return;
    }
    
    int visibleLines = m_terminalHeight - 4; // Account for borders and header
    LOG_DEBUG("UIManager::ensureSelectionVisible - visibleLines: " + std::to_string(visibleLines));
    
    // Scroll up if selection is above visible area
    if (m_selectedScript < m_scriptListScroll) {
        LOG_DEBUG("UIManager::ensureSelectionVisible - scrolling up from " + std::to_string(m_scriptListScroll) + " to " + std::to_string(m_selectedScript));
        m_scriptListScroll = m_selectedScript;
    }
    
    // Scroll down if selection is below visible area
    if (m_selectedScript >= m_scriptListScroll + visibleLines) {
        int newScroll = m_selectedScript - visibleLines + 1;
        LOG_DEBUG("UIManager::ensureSelectionVisible - scrolling down from " + std::to_string(m_scriptListScroll) + " to " + std::to_string(newScroll));
        m_scriptListScroll = newScroll;
    }
    
    // Ensure scroll position is valid
    m_scriptListScroll = std::max(0, m_scriptListScroll);
    m_scriptListScroll = std::min(m_scriptListScroll, 
                                 static_cast<int>(m_scriptList.size()) - visibleLines);
    if (m_scriptListScroll < 0) m_scriptListScroll = 0;
}

void UIManager::drawScriptList() {
    if (!m_scriptListPane) return;
    
    // Clear content area with default attributes (preserve borders)
    if (has_colors()) {
        wattron(m_scriptListPane, COLOR_PAIR(static_cast<int>(ColorPair::DEFAULT)));
    }
    for (int y = 1; y < m_terminalHeight - m_statusBarHeight - 1; y++) {
        wmove(m_scriptListPane, y, 1);
        for (int x = 1; x < m_scriptListWidth - 1; x++) {
            waddch(m_scriptListPane, ' ');
        }
    }
    if (has_colors()) {
        wattroff(m_scriptListPane, COLOR_PAIR(static_cast<int>(ColorPair::DEFAULT)));
    }
    
    int visibleLines = m_terminalHeight - 4;
    int startIndex = m_scriptListScroll;
    int endIndex = std::min(startIndex + visibleLines, static_cast<int>(m_scriptList.size()));
    
    for (int i = startIndex; i < endIndex; i++) {
        int y = i - startIndex + 1; // +1 for border
        
        // Handle separators differently
        if (isSeparator(m_scriptList[i])) {
            std::string separator = m_scriptList[i];
            
            // Truncate if too long
            int maxWidth = m_scriptListWidth - 2; // Account for borders
            if (static_cast<int>(separator.length()) > maxWidth) {
                separator = separator.substr(0, maxWidth - 3) + "...";
            }
            
            // Draw separator in different color/style
            if (has_colors()) {
                wattron(m_scriptListPane, COLOR_PAIR(static_cast<int>(ColorPair::HEADER)));
            }
            mvwprintw(m_scriptListPane, y, 1, "%s", separator.c_str());
            if (has_colors()) {
                wattroff(m_scriptListPane, COLOR_PAIR(static_cast<int>(ColorPair::HEADER)));
            }
        } else {
            // Regular script entry
            std::string displayName = getBaseName(m_scriptList[i]);
            
            // Truncate if too long
            int maxWidth = m_scriptListWidth - 4; // Account for borders and numbering
            if (static_cast<int>(displayName.length()) > maxWidth) {
                displayName = displayName.substr(0, maxWidth - 3) + "...";
            }
            
            // Highlight selection (but not separators)
            if (i == m_selectedScript && has_colors()) {
                wattron(m_scriptListPane, COLOR_PAIR(static_cast<int>(ColorPair::SELECTED)));
            }
            
            mvwprintw(m_scriptListPane, y, 1, "[%d] %s", i + 1, displayName.c_str());
            
            if (i == m_selectedScript && has_colors()) {
                wattroff(m_scriptListPane, COLOR_PAIR(static_cast<int>(ColorPair::SELECTED)));
            }
        }
    }
    
    // Draw scroll indicators
    if (m_scriptListScroll > 0) {
        mvwaddch(m_scriptListPane, 1, m_scriptListWidth - 2, '^');
    }
    if (endIndex < static_cast<int>(m_scriptList.size())) {
        mvwaddch(m_scriptListPane, m_terminalHeight - m_statusBarHeight - 2, 
                m_scriptListWidth - 2, 'v');
    }
}

void UIManager::drawOutputPane(int paneIndex) {
    WINDOW* pane = (paneIndex == 0) ? m_outputPane1 : m_outputPane2;
    if (!pane) return;
    
    // Get the correct width for this pane
    int paneWidth = (paneIndex == 0) ? m_outputPane1Width : m_outputPane2Width;
    
    // Clear content area with default attributes (preserve borders)
    if (has_colors()) {
        wattron(pane, COLOR_PAIR(static_cast<int>(ColorPair::DEFAULT)));
    }
    for (int y = 1; y < m_terminalHeight - m_statusBarHeight - 1; y++) {
        wmove(pane, y, 1);
        for (int x = 1; x < paneWidth - 1; x++) {
            waddch(pane, ' ');
        }
    }
    if (has_colors()) {
        wattroff(pane, COLOR_PAIR(static_cast<int>(ColorPair::DEFAULT)));
    }
    
    const auto& buffer = m_outputBuffer[paneIndex];
    int visibleLines = m_terminalHeight - 4;
    int maxWidth = paneWidth - 2; // Account for borders
    
    // Create wrapped buffer if needed
    std::vector<std::string> wrappedLines;
    for (const auto& line : buffer) {
        auto wrapped = wrapText(line, maxWidth);
        wrappedLines.insert(wrappedLines.end(), wrapped.begin(), wrapped.end());
    }
    
    // Apply scrolling to wrapped content
    int startLine = m_scrollPosition[paneIndex];
    int endLine = std::min(startLine + visibleLines, static_cast<int>(wrappedLines.size()));
    
    for (int i = startLine; i < endLine; i++) {
        int y = i - startLine + 1; // +1 for border
        const std::string& line = wrappedLines[i];
        
        // Render line with ANSI colors preserved
        renderAnsiText(pane, line, y, 1, maxWidth);
    }
    
    // Draw scroll indicators based on wrapped content
    if (m_scrollPosition[paneIndex] > 0) {
        mvwaddch(pane, 1, paneWidth - 2, '^');
    }
    if (endLine < static_cast<int>(wrappedLines.size())) {
        mvwaddch(pane, m_terminalHeight - m_statusBarHeight - 2, 
                paneWidth - 2, 'v');
    }
}

void UIManager::drawHelpOverlay() {
    // Calculate help window size
    int helpWidth = std::min(60, m_terminalWidth - 4);
    int helpHeight = std::min(20, m_terminalHeight - 4);
    int startY = (m_terminalHeight - helpHeight) / 2;
    int startX = (m_terminalWidth - helpWidth) / 2;
    
    // Create help window
    safeDeleteWindow(m_helpWindow);
    m_helpWindow = newwin(helpHeight, helpWidth, startY, startX);
    
    if (!m_helpWindow) return;
    
    // Draw border and background
    box(m_helpWindow, 0, 0);
    if (has_colors()) {
        wbkgd(m_helpWindow, COLOR_PAIR(static_cast<int>(ColorPair::DEFAULT)));
    }
    
    // Draw title
    if (has_colors()) {
        wattron(m_helpWindow, COLOR_PAIR(static_cast<int>(ColorPair::HEADER)));
    }
    mvwprintw(m_helpWindow, 0, 2, " Help - Keyboard Shortcuts ");
    if (has_colors()) {
        wattroff(m_helpWindow, COLOR_PAIR(static_cast<int>(ColorPair::HEADER)));
    }
    
    // Draw help content
    const char* helpText[] = {
        "Navigation:",
        "  ↑/k        Move up in script list",
        "  ↓/j        Move down in script list",
        "  Tab        Cycle between output panes",
        "",
        "Execution:",
        "  Enter      Execute script in selected pane",
        "  Space      Execute in available pane",
        "  Double-click Execute script in selected pane",
        "  Ctrl+C     Terminate active script",
        "  r          Refresh script list",
        "",
        "View:",
        "  PageUp     Scroll output up",
        "  PageDown   Scroll output down",
        "  Ctrl+L     Clear active output",
        "  Mouse      Click panes to focus, wheel to scroll",
        "",
        "Resize (Active Panel):",
        "  Ctrl+←     Expand active panel left",
        "  Ctrl+→     Expand active panel right",
        "  [/]        Legacy script list resize",
        "  F1/?       Toggle this help",
        "  Ctrl+Q     Quit application"
    };
    
    int numLines = sizeof(helpText) / sizeof(helpText[0]);
    for (int i = 0; i < numLines && i < helpHeight - 3; i++) {
        mvwprintw(m_helpWindow, i + 2, 2, "%s", helpText[i]);
    }
}

void UIManager::updateActivePaneBorder() {
    if (!m_initialized) {
        return;
    }
    
    // Reset all borders with null checks
    if (m_scriptListPane) box(m_scriptListPane, 0, 0);
    if (m_outputPane1) box(m_outputPane1, 0, 0);
    if (m_outputPane2) box(m_outputPane2, 0, 0);
    
    // Highlight active pane border
    WINDOW* activePaneWindow = nullptr;
    switch (m_activePane) {
        case 0: activePaneWindow = m_scriptListPane; break;
        case 1: activePaneWindow = m_outputPane1; break;
        case 2: activePaneWindow = m_outputPane2; break;
    }
    
    if (activePaneWindow && has_colors()) {
        wattron(activePaneWindow, COLOR_PAIR(static_cast<int>(ColorPair::SELECTED)));
        box(activePaneWindow, 0, 0);
        wattroff(activePaneWindow, COLOR_PAIR(static_cast<int>(ColorPair::SELECTED)));
    }
    
    // Redraw headers to ensure they're visible
    drawHeaders();
}

bool UIManager::resizeScriptListWider() {
    if (!m_initialized) {
        LOG_WARNING("Cannot resize - UI not initialized");
        return false;
    }
    
    // Check if we can increase the width
    if (m_currentScriptListWidthRatio >= MAX_SCRIPT_LIST_WIDTH) {
        LOG_DEBUG("Script list already at maximum width (" + 
                 std::to_string(MAX_SCRIPT_LIST_WIDTH) + "%)");
        return false;
    }
    
    // Increase width by RESIZE_STEP
    int newRatio = std::min(m_currentScriptListWidthRatio + RESIZE_STEP, 
                           MAX_SCRIPT_LIST_WIDTH);
    
    LOG_INFO("Resizing script list wider: " + 
             std::to_string(m_currentScriptListWidthRatio) + "% -> " + 
             std::to_string(newRatio) + "%");
    
    m_currentScriptListWidthRatio = newRatio;
    
    // Recreate layout with new dimensions
    createLayout();
    
    // Redraw all content
    drawScriptList();
    drawOutputPane(0);
    drawOutputPane(1);
    
    refresh();
    
    return true;
}

bool UIManager::resizeScriptListNarrower() {
    if (!m_initialized) {
        LOG_WARNING("Cannot resize - UI not initialized");
        return false;
    }
    
    // Check if we can decrease the width
    if (m_currentScriptListWidthRatio <= MIN_SCRIPT_LIST_WIDTH) {
        LOG_DEBUG("Script list already at minimum width (" + 
                 std::to_string(MIN_SCRIPT_LIST_WIDTH) + "%)");
        return false;
    }
    
    // Decrease width by RESIZE_STEP
    int newRatio = std::max(m_currentScriptListWidthRatio - RESIZE_STEP, 
                           MIN_SCRIPT_LIST_WIDTH);
    
    LOG_INFO("Resizing script list narrower: " + 
             std::to_string(m_currentScriptListWidthRatio) + "% -> " + 
             std::to_string(newRatio) + "%");
    
    m_currentScriptListWidthRatio = newRatio;
    
    // Recreate layout with new dimensions
    createLayout();
    
    // Redraw all content
    drawScriptList();
    drawOutputPane(0);
    drawOutputPane(1);
    
    refresh();
    
    return true;
}

bool UIManager::resizeOutputPane1Wider() {
    if (!m_initialized) {
        LOG_WARNING("Cannot resize - UI not initialized");
        return false;
    }
    
    // Check if we can increase output pane 1 width
    if (m_outputPane1WidthRatio >= MAX_OUTPUT_PANE_RATIO) {
        LOG_DEBUG("Output pane 1 already at maximum width (" + 
                 std::to_string(MAX_OUTPUT_PANE_RATIO) + "%)");
        return false;
    }
    
    // Increase output pane 1 width by RESIZE_STEP
    int newRatio = std::min(m_outputPane1WidthRatio + RESIZE_STEP, 
                           MAX_OUTPUT_PANE_RATIO);
    
    LOG_INFO("Resizing output pane 1 wider: " + 
             std::to_string(m_outputPane1WidthRatio) + "% -> " + 
             std::to_string(newRatio) + "%");
    
    m_outputPane1WidthRatio = newRatio;
    
    // Recreate layout with new dimensions
    createLayout();
    
    // Redraw all content
    drawScriptList();
    drawOutputPane(0);
    drawOutputPane(1);
    
    refresh();
    
    return true;
}

bool UIManager::resizeOutputPane1Narrower() {
    if (!m_initialized) {
        LOG_WARNING("Cannot resize - UI not initialized");
        return false;
    }
    
    // Check if we can decrease output pane 1 width
    if (m_outputPane1WidthRatio <= MIN_OUTPUT_PANE_RATIO) {
        LOG_DEBUG("Output pane 1 already at minimum width (" + 
                 std::to_string(MIN_OUTPUT_PANE_RATIO) + "%)");
        return false;
    }
    
    // Decrease output pane 1 width by RESIZE_STEP
    int newRatio = std::max(m_outputPane1WidthRatio - RESIZE_STEP, 
                           MIN_OUTPUT_PANE_RATIO);
    
    LOG_INFO("Resizing output pane 1 narrower: " + 
             std::to_string(m_outputPane1WidthRatio) + "% -> " + 
             std::to_string(newRatio) + "%");
    
    m_outputPane1WidthRatio = newRatio;
    
    // Recreate layout with new dimensions
    createLayout();
    
    // Redraw all content
    drawScriptList();
    drawOutputPane(0);
    drawOutputPane(1);
    
    refresh();
    
    return true;
}

bool UIManager::resizeActivePaneUp() {
    if (!m_initialized) {
        LOG_WARNING("Cannot resize - UI not initialized");
        return false;
    }
    
    switch (m_activePane) {
        case 0: // Script list - expand in both directions (script list gets bigger, output panes smaller)
            return resizeScriptListWider();
            
        case 1: // Output pane 1 - expand in both directions (pane 1 gets bigger, pane 2 smaller)
            return resizeOutputPane1Wider();
            
        case 2: // Output pane 2 - expand in both directions (pane 2 gets bigger, pane 1 smaller)
            return resizeOutputPane1Narrower(); // This makes pane 2 bigger
            
        default:
            return false;
    }
}

bool UIManager::resizeActivePaneDown() {
    if (!m_initialized) {
        LOG_WARNING("Cannot resize - UI not initialized");
        return false;
    }
    
    switch (m_activePane) {
        case 0: // Script list - shrink from both sides (script list gets smaller, output panes bigger)
            return resizeScriptListNarrower();
            
        case 1: // Output pane 1 - shrink from both sides (pane 1 gets smaller, pane 2 bigger)
            return resizeOutputPane1Narrower();
            
        case 2: // Output pane 2 - shrink from both sides (pane 2 gets smaller, pane 1 bigger)
            return resizeOutputPane1Wider(); // This makes pane 2 smaller
            
        default:
            return false;
    }
}

bool UIManager::resizeActivePaneLeft() {
    if (!m_initialized) {
        LOG_WARNING("Cannot resize - UI not initialized");
        return false;
    }
    
    switch (m_activePane) {
        case 0: // Script list - can't expand left (already at leftmost position)
            LOG_DEBUG("Script list cannot expand left - already at leftmost position");
            return false;
            
        case 1: // Output pane 1 - expand left by taking space from script list
            return resizeScriptListNarrower(); // Makes script list smaller, output panes bigger
            
        case 2: // Output pane 2 - expand left by taking space from output pane 1
            return resizeOutputPane1Narrower(); // Makes pane 1 smaller, pane 2 bigger
            
        default:
            return false;
    }
}

bool UIManager::resizeActivePaneRight() {
    if (!m_initialized) {
        LOG_WARNING("Cannot resize - UI not initialized");
        return false;
    }
    
    switch (m_activePane) {
        case 0: // Script list - expand right by taking space from output panes
            return resizeScriptListWider();
            
        case 1: // Output pane 1 - expand right by taking space from output pane 2
            return resizeOutputPane1Wider(); // Makes pane 1 bigger, pane 2 smaller
            
        case 2: // Output pane 2 - can't expand right (already at rightmost position)
            LOG_DEBUG("Output pane 2 cannot expand right - already at rightmost position");
            return false;
            
        default:
            return false;
    }
}

int UIManager::calculateWrappedLineCount(int paneIndex) const {
    if (paneIndex < 0 || paneIndex > 1) {
        return 0;
    }
    
    const auto& buffer = m_outputBuffer[paneIndex];
    if (buffer.empty()) {
        return 0;
    }
    
    // Get the correct width for this pane
    int paneWidth = (paneIndex == 0) ? m_outputPane1Width : m_outputPane2Width;
    int maxWidth = paneWidth - 2; // Account for borders
    
    int totalLines = 0;
    for (const auto& line : buffer) {
        auto wrapped = wrapText(line, maxWidth);
        totalLines += static_cast<int>(wrapped.size());
    }
    
    return totalLines;
}

void UIManager::safeDeleteWindow(WINDOW*& window) {
    if (window) {
        delwin(window);
        window = nullptr;
    }
}
