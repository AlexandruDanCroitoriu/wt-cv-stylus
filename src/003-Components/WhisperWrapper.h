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
    
    // Transcribe audio file (expects 16kHz mono WAV format from browser)
    std::string transcribeFile(const std::string& audio_file_path);
    
    // Transcribe raw audio data (16kHz, mono, float32)
    std::string transcribeAudioData(const std::vector<float>& audio_data);
    
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
