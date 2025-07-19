#include "WhisperCliService.h"
#include <iostream>
#include <array>
#include <sstream>
#include <cstdio>
#include <algorithm>
#include <thread>
#include <chrono>
#include <mutex>
#include <sys/file.h>

// Global mutex to prevent concurrent transcriptions
static std::mutex transcription_mutex;

WhisperCliService::WhisperCliService()
    : initialized_(false)
    , whisper_executable_path_()
    , model_path_()
    , last_error_()
{
}

WhisperCliService::~WhisperCliService() {
    // No cleanup needed for CLI service
}

bool WhisperCliService::initialize(const std::string& whisper_executable_path, const std::string& model_path) {
    whisper_executable_path_ = whisper_executable_path;
    model_path_ = model_path;
    initialized_ = true;
    return true;
}

bool WhisperCliService::isInitialized() const {
    return initialized_;
}

std::string WhisperCliService::transcribeFile(const std::string& audio_file_path) {
    if (!initialized_) {
        setError("WhisperCliService not initialized");
        return "ERROR: Service not initialized";
    }

    // Use mutex to serialize transcriptions for stability
    std::lock_guard<std::mutex> lock(transcription_mutex);
    std::cout << "Starting transcription for: " << audio_file_path << std::endl;
    
    auto result = executeWhisperService(audio_file_path);
    
    std::cout << "Completed transcription for: " << audio_file_path << std::endl;
              
    return result;
}

void WhisperCliService::transcribeFileAsync(const std::string& audio_file_path,
                                            std::function<void(const std::string&)> callback) {
    // For now, implement as synchronous call
    // In the future, this could be implemented with std::async or threading
    std::string result = transcribeFile(audio_file_path);
    if (callback) {
        callback(result);
    }
}

std::string WhisperCliService::getLastError() const {
    return last_error_;
}

std::string WhisperCliService::executeWhisperService(const std::string& audio_file_path) {
    // Use file locking to prevent concurrent transcriptions
    // This ensures only one transcription runs at a time for stability
    std::ostringstream command_stream;
    command_stream << "flock /tmp/whisper.lock timeout 60s \"" << whisper_executable_path_ << "\" \"" 
                   << model_path_ << "\" \"" << audio_file_path << "\" 2>/dev/null";
    std::string command = command_stream.str();
    
    std::cout << "Executing (serialized): " << command << std::endl;
    
    // Execute command with robust error handling
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        setError("Failed to execute whisper service");
        return "ERROR: Failed to execute whisper service";
    }
    
    // Read output with buffering to prevent blocking
    std::array<char, 4096> buffer;
    std::string result;
    result.reserve(8192); // Reserve space to reduce allocations
    
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
        
        // Prevent excessive memory usage
        if (result.size() > 1024 * 1024) { // 1MB limit
            setError("Output too large, possible infinite loop");
            pclose(pipe);
            return "ERROR: Output too large";
        }
    }
    
    int exit_code = pclose(pipe);
    
    // Additional cleanup - ensure no zombie processes
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    if (exit_code == 0) {
        // Success - parse JSON and extract transcription
        try {
            // Remove trailing newline if present
            if (!result.empty() && result.back() == '\n') {
                result.pop_back();
            }
            
            json response = json::parse(result);
            
            if (response.contains("success") && response["success"].get<bool>()) {
                if (response.contains("transcription")) {
                    std::string transcription = response["transcription"].get<std::string>();
                    
                    // Log detailed info for debugging
                    if (response.contains("timing")) {
                        auto timing = response["timing"];
                        std::cout << "Transcription completed in " 
                                  << timing["total_processing_ms"].get<int>() << "ms" << std::endl;
                    }
                    
                    return transcription;
                } else {
                    setError("JSON response missing transcription field");
                    return "ERROR: Invalid response format";
                }
            } else {
                std::string error_msg = "Transcription failed";
                if (response.contains("error")) {
                    error_msg = response["error"].get<std::string>();
                }
                setError(error_msg);
                return "ERROR: " + error_msg;
            }
        } catch (const json::exception& e) {
            setError("Failed to parse JSON response: " + std::string(e.what()));
            return "ERROR: Invalid JSON response: " + result;
        }
    } else {
        setError("Whisper service failed with exit code " + std::to_string(exit_code / 256) + ". Output: " + result);
        return "ERROR: Transcription failed: " + result;
    }
}

void WhisperCliService::setError(const std::string& error) const {
    last_error_ = error;
    std::cerr << "WhisperCliService Error: " << error << std::endl;
}
