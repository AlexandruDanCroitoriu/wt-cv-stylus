/**
 * @file test_process_manager.cpp
 * @brief Test program for ProcessManager functionality
 * 
 * This test program verifies that the ProcessManager can correctly
 * discover and execute scripts.
 */

#include "../include/Logger.h"
#include "../include/ProcessManager.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        // Initialize logging
        auto& logger = Logger::getInstance();
        logger.setLevel(Logger::Level::DEBUG);
        
        LOG_INFO("ProcessManager Test Starting");
        
        // Create ProcessManager
        ProcessManager pm;
        
        // Discover scripts
        auto scripts = pm.discoverScripts("../");
        LOG_INFO("Found " + std::to_string(scripts.size()) + " scripts");
        
        // Find our example script
        std::string targetScript;
        for (const auto& script : scripts) {
            if (script.find("example_task.py") != std::string::npos) {
                targetScript = script;
                break;
            }
        }
        
        if (targetScript.empty()) {
            LOG_ERROR("Could not find example_task.py");
            return 1;
        }
        
        LOG_INFO("Testing script execution: " + targetScript);
        
        // Start the script
        if (!pm.startScript(targetScript, 0)) {
            LOG_ERROR("Failed to start script");
            return 1;
        }
        
        LOG_INFO("Script started successfully");
        
        // Monitor execution for 15 seconds
        for (int i = 0; i < 150; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Get new output
            std::string output = pm.getNewOutput(0);
            if (!output.empty()) {
                std::cout << output;
            }
            
            // Check status
            if (!pm.isRunning(0)) {
                LOG_INFO("Script completed");
                break;
            }
            
            // Status update every 2 seconds
            if (i % 20 == 0) {
                auto runtime = pm.getRuntime(0);
                LOG_DEBUG("Script still running, runtime: " + std::to_string(runtime.count()) + "s");
            }
        }
        
        // Get final output
        std::string finalOutput = pm.getNewOutput(0);
        if (!finalOutput.empty()) {
            std::cout << finalOutput;
        }
        
        // Check final status
        ProcessStatus status = pm.getStatus(0);
        LOG_INFO("Final status: " + std::to_string(static_cast<int>(status)));
        
        LOG_INFO("ProcessManager Test Completed");
        return 0;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Test failed: " + std::string(e.what()));
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
