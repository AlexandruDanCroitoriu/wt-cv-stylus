/**
 * @file main.cpp
 * @brief Entry point for the terminal script runner application
 * 
 * @author GitHub Copilot
 * @date 2025-07-19
 * @version 1.0.0
 */

#include <iostream>
#include <cstdlib>
#include "ScriptRunner.h"
#include "Logger.h"

/**
 * @brief Main entry point for the script runner application
 * 
 * Initializes the application, runs the main event loop, and handles
 * any errors that occur during execution.
 * 
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return int Exit code (0 for success, non-zero for error)
 */
int main(int argc, char* argv[]) {
    // Initialize logger first
    Logger& logger = Logger::getInstance();
    logger.log(Logger::Level::INFO, "Starting terminal script runner application v1.0.0");
    
    // Handle command line arguments
    if (argc > 1) {
        std::string arg1(argv[1]);
        if (arg1 == "--version") {
            std::cout << "C++ Terminal Script Runner v1.0.0\n";
            return 0;
        } else if (arg1 == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --version    Show version information\n";
            std::cout << "  --help       Show this help message\n";
            std::cout << "\nControls:\n";
            std::cout << "  Arrow keys / hjkl  Navigate script list\n";
            std::cout << "  Enter             Execute selected script\n";
            std::cout << "  Space             Execute in alternating pane\n";
            std::cout << "  Tab / Left/Right  Switch between output panes\n";
            std::cout << "  Ctrl+C / t        Terminate active process\n";
            std::cout << "  r / F5            Refresh script list\n";
            std::cout << "  Ctrl+L            Clear active pane\n";
            std::cout << "  Ctrl+Left/Right   Resize script list panel\n";
            std::cout << "  F1 / ?            Show help\n";
            std::cout << "  Ctrl+Q / Esc      Quit application\n";
            return 0;
        }
    }
    
    try {
        // Create and run the main application
        ScriptRunner app;
        int exitCode = app.run();
        
        if (exitCode == 0) {
            logger.log(Logger::Level::INFO, "Application completed successfully");
        } else {
            logger.log(Logger::Level::ERROR, "Application exited with code: " + std::to_string(exitCode));
        }
        
        return exitCode;
        
    } catch (const std::exception& e) {
        logger.log(Logger::Level::ERROR, "Fatal application error: " + std::string(e.what()));
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        logger.log(Logger::Level::ERROR, "Unknown fatal error occurred");
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 2;
    }
}
