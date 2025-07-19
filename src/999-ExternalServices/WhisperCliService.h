#pragma once
#include <string>
#include <future>
#include <thread>
#include <atomic>
#include <functional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @brief WhisperCliService - Command Line Interface service for Whisper transcription
 * 
 * This class provides a simple interface to call an external whisper_service executable
 * for audio transcription. It consolidates both client and service functionality.
 */
class WhisperCliService {
public:
    WhisperCliService();
    ~WhisperCliService();
    
    /**
     * @brief Initialize the service with paths to the executable and model
     * @param whisper_executable_path Path to the whisper_service executable
     * @param model_path Path to the Whisper model file (e.g., ggml-base.en.bin)
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const std::string& whisper_executable_path, const std::string& model_path);
    
    /**
     * @brief Synchronously transcribe an audio file
     * @param audio_file_path Path to the audio file to transcribe
     * @return Transcribed text or error message starting with "ERROR:"
     */
    std::string transcribeFile(const std::string& audio_file_path);
    
    /**
     * @brief Asynchronously transcribe an audio file
     * @param audio_file_path Path to the audio file to transcribe
     * @return Future that will contain the transcribed text
     */
    /**
     * Transcribe an audio file asynchronously
     * @param audio_file_path Path to the audio file to transcribe
     * @param callback Callback function to be called with the transcription result
     */
    void transcribeFileAsync(const std::string& audio_file_path,
                             std::function<void(const std::string&)> callback);
    
    /**
     * @brief Check if the service is properly initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const;
    
    /**
     * @brief Get the last error message
     * @return Last error message
     */
    std::string getLastError() const;

private:
    /**
     * @brief Execute the whisper service with the given audio file
     * @param audio_file_path Path to the audio file
     * @return Transcribed text or error message
     */
    std::string executeWhisperService(const std::string& audio_file_path);
    
    /**
     * @brief Set error message
     * @param error Error message to set
     */
    void setError(const std::string& error) const;

    bool initialized_;
    std::string whisper_executable_path_;
    std::string model_path_;
    mutable std::string last_error_;
};
