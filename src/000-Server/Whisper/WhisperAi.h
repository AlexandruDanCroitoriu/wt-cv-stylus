#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>

// Forward declaration to avoid including whisper.h in header
struct whisper_context;

class WhisperAi {
public:
    // Singleton access
    static WhisperAi& getInstance();
    
    // Delete copy constructor and assignment operator
    WhisperAi(const WhisperAi&) = delete;
    WhisperAi& operator=(const WhisperAi&) = delete;

    // Initialize with default model (ggml-base.en.bin) - thread-safe
    bool initialize();
    
    // Transcribe audio file (expects 16kHz mono WAV format from browser) - thread-safe
    std::string transcribeFile(const std::string& audio_file_path);
    
    // Transcribe raw audio data (16kHz, mono, float32) - thread-safe
    std::string transcribeAudioData(const std::vector<float>& audio_data);
    
    // Check if initialized properly - thread-safe
    bool isInitialized() const;
    
    // Error handling - thread-safe
    std::string getLastError() const;

private:
    WhisperAi();
    ~WhisperAi();

    whisper_context* context_;
    std::string last_error_;
    
    // Thread safety
    mutable std::mutex mutex_;
    
    // Helper methods
    bool loadAudioFile(const std::string& file_path, std::vector<float>& audio_data);
    void setError(const std::string& error);
    
    // Internal transcription method (assumes mutex is already locked)
    std::string transcribeAudioDataInternal(const std::vector<float>& audio_data);
};
