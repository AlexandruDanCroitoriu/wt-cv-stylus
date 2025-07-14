#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>

// Forward declaration to avoid including whisper.h in header
struct whisper_context;

class WhisperWrapper {
public:
    WhisperWrapper();
    ~WhisperWrapper();

    // Initialize with default model (ggml-base.en.bin)
    bool initialize();
    
    // Transcribe audio file
    std::string transcribeFile(const std::string& audio_file_path);
    
    // Transcribe raw audio data (16kHz, mono, float32)
    std::string transcribeAudioData(const std::vector<float>& audio_data);
    
    // Convert audio file to required format (WAV 16kHz mono)
    static bool convertAudioToWav(const std::string& input_file, const std::string& output_file);
    
    // Convert and save audio file to WAV format in same directory
    static std::string convertAndSaveAudioToWav(const std::string& input_file);
    

    
    // Check if initialized properly
    bool isInitialized() const { return context_ != nullptr; }
    
    // Error handling
    std::string getLastError() const { return last_error_; }

private:
    whisper_context* context_;
    std::string last_error_;
    
    // Helper methods
    bool loadAudioFile(const std::string& file_path, std::vector<float>& audio_data);
    void setError(const std::string& error);
};
