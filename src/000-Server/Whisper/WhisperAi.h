#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <future>
#include <atomic>

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
    
    // Async transcription methods (non-blocking)
    std::future<std::string> transcribeFileAsync(const std::string& audio_file_path);
    std::future<std::string> transcribeAudioDataAsync(const std::vector<float> audio_data);
    
    // Get queue status
    size_t getQueueSize() const;
    
    // Check if initialized properly - thread-safe
    bool isInitialized() const;
    
    // Error handling - thread-safe
    std::string getLastError() const;

private:
    WhisperAi();
    ~WhisperAi();

    // Task structure for async processing
    struct TranscriptionTask {
        enum Type { FILE, AUDIO_DATA };
        Type type;
        std::string file_path;              // For FILE type
        std::vector<float> audio_data;      // For AUDIO_DATA type
        std::promise<std::string> result_promise;
        std::string task_id;
        
        TranscriptionTask(Type t, const std::string& path) 
            : type(t), file_path(path), task_id(generateTaskId()) {}
        TranscriptionTask(Type t, std::vector<float> data) 
            : type(t), audio_data(std::move(data)), task_id(generateTaskId()) {}
        
        // Move constructor and assignment
        TranscriptionTask(TranscriptionTask&& other) noexcept 
            : type(other.type), file_path(std::move(other.file_path)), 
              audio_data(std::move(other.audio_data)), 
              result_promise(std::move(other.result_promise)),
              task_id(std::move(other.task_id)) {}
        
        TranscriptionTask& operator=(TranscriptionTask&& other) noexcept {
            if (this != &other) {
                type = other.type;
                file_path = std::move(other.file_path);
                audio_data = std::move(other.audio_data);
                result_promise = std::move(other.result_promise);
                task_id = std::move(other.task_id);
            }
            return *this;
        }
        
        // Delete copy constructor and assignment
        TranscriptionTask(const TranscriptionTask&) = delete;
        TranscriptionTask& operator=(const TranscriptionTask&) = delete;
        
    private:
        static std::string generateTaskId();
    };

    whisper_context* context_;
    std::string last_error_;
    
    // Thread safety for context operations
    mutable std::mutex context_mutex_;
    
    // Async processing components
    std::queue<TranscriptionTask> task_queue_;
    mutable std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::thread worker_thread_;
    std::atomic<bool> shutdown_;
    std::atomic<bool> worker_running_;
    
    // Helper methods
    bool loadAudioFile(const std::string& file_path, std::vector<float>& audio_data);
    void setError(const std::string& error);
    
    // Internal transcription method (assumes context_mutex is already locked)
    std::string transcribeAudioDataInternal(const std::vector<float>& audio_data);
    
    // Worker thread methods
    void startWorkerThread();
    void stopWorkerThread();
    void workerLoop();
    
    // Process a single task (called by worker thread)
    std::string processTask(const TranscriptionTask& task);
};
