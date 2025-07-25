#include "WhisperAi.h"
#include "whisper.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <thread>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <future>
#include <queue>
#include <atomic>

// Helper function to get current timestamp
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "[%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
    return ss.str();
}

// Singleton implementation
WhisperAi& WhisperAi::getInstance() {
    static WhisperAi instance;
    return instance;
}

WhisperAi::WhisperAi() 
    : context_(nullptr), shutdown_(false), worker_running_(false) {
    std::cout << getCurrentTimestamp() << "WhisperAi singleton instance created" << std::endl;
}

WhisperAi::~WhisperAi() {
    std::cout << getCurrentTimestamp() << "WhisperAi singleton destructor called" << std::endl;
    
    // Stop worker thread first
    stopWorkerThread();
    
    // Clean up context
    std::lock_guard<std::mutex> lock(context_mutex_);
    if (context_) {
        whisper_free(context_);
        context_ = nullptr;
        std::cout << getCurrentTimestamp() << "WhisperAi singleton destroyed and context freed" << std::endl;
    }
}

bool WhisperAi::initialize() {
    std::lock_guard<std::mutex> lock(context_mutex_);
    
    // Check if already initialized
    if (context_ != nullptr) {
        std::cout << getCurrentTimestamp() << "Whisper already initialized (singleton), reusing existing context" << std::endl;
        return true;
    }
    
    std::cout << getCurrentTimestamp() << "Initializing Whisper singleton..." << std::endl;
    

    std::string model_path;
    std::string full_path = "/apps/cv/models/ggml-base.en.bin";
    if (std::ifstream(full_path).good()) {
        model_path = full_path;
        std::cout << getCurrentTimestamp() << "Found Whisper model: " << full_path << std::endl;
    }
    
    if (model_path.empty()) {
        setError("Model file ggml-base.en.bin not found in any models/ directory. "
                 "Please download from https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.en.bin");
        return false;
    }

    // Initialize whisper context with optimized parameters
    struct whisper_context_params cparams = whisper_context_default_params();
    cparams.use_gpu = false; // Disable GPU for stability and consistency
    
    context_ = whisper_init_from_file_with_params(model_path.c_str(), cparams);
    
    if (!context_) {
        setError("Failed to initialize whisper context from model: " + model_path);
        return false;
    }
    
    std::cout << getCurrentTimestamp() << "Whisper singleton initialized successfully with model: " << model_path << std::endl;
    std::cout << getCurrentTimestamp() << "Available threads: " << std::thread::hardware_concurrency() << std::endl;
    
    // Start the worker thread
    startWorkerThread();
    
    return true;
}

std::string WhisperAi::transcribeFile(const std::string& audio_file_path) {
    std::lock_guard<std::mutex> lock(context_mutex_);
    
    if (context_ == nullptr) {
        setError("Whisper not initialized");
        return "";
    }
    
    // Browser now provides WAV files in the correct format (16kHz, mono, 16-bit PCM)
    std::cout << getCurrentTimestamp() << "Loading audio file: " << audio_file_path << std::endl;
    
    // Load audio data directly
    std::vector<float> audio_data;
    if (!loadAudioFile(audio_file_path, audio_data)) {
        return "";
    }
    
    // Call internal method that doesn't acquire mutex
    return transcribeAudioDataInternal(audio_data);
}

std::string WhisperAi::transcribeAudioData(const std::vector<float>& audio_data) {
    std::lock_guard<std::mutex> lock(context_mutex_);
    return transcribeAudioDataInternal(audio_data);
}

std::string WhisperAi::transcribeAudioDataInternal(const std::vector<float>& audio_data) {
    // Assumes mutex is already locked by caller
    if (context_ == nullptr) {
        setError("Whisper not initialized");
        return "";
    }
    
    if (audio_data.empty()) {
        setError("Audio data is empty");
        return "";
    }
    
    // Set up whisper parameters with performance optimizations
    std::cout << getCurrentTimestamp() << "Starting transcription of " << audio_data.size() << " audio samples" << std::endl;
    struct whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    
    // Performance optimizations - these are critical!
    wparams.print_realtime   = false;
    wparams.print_progress   = false;
    wparams.print_timestamps = false;
    wparams.print_special    = false;
    wparams.translate        = false;
    wparams.single_segment   = false;
    wparams.max_tokens       = 0;
    wparams.offset_ms        = 0;
    wparams.duration_ms      = 0;
    
    // Key performance settings
    wparams.n_threads        = std::min(4, (int)std::thread::hardware_concurrency());
    wparams.speed_up         = false;  // Disable speed up for better accuracy
    wparams.temperature      = 0.0f;   // Deterministic output
    wparams.temperature_inc  = 0.0f;
    wparams.entropy_thold    = 2.4f;   // Entropy threshold for early stopping
    wparams.logprob_thold    = -1.0f;  // Log probability threshold
    wparams.no_speech_thold  = 0.6f;   // No speech threshold
    
    // Set language to English (en) for ggml-base.en.bin model
    wparams.language = "en";
    
    // Run inference
    int result = whisper_full(context_, wparams, audio_data.data(), audio_data.size());
    
    if (result != 0) {
        setError("Whisper transcription failed with error code: " + std::to_string(result));
        return "";
    }
    
    // Extract transcribed text
    std::string transcription;
    const int n_segments = whisper_full_n_segments(context_);
    
    for (int i = 0; i < n_segments; ++i) {
        const char* text = whisper_full_get_segment_text(context_, i);
        if (text) {
            transcription += text;
        }
    }
    
    // Trim whitespace
    size_t start = transcription.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = transcription.find_last_not_of(" \t\n\r");
    transcription = transcription.substr(start, end - start + 1);
    
    std::cout << getCurrentTimestamp() << "Transcription completed: " << transcription.length() << " characters" << std::endl;
    
    return transcription;
}

bool WhisperAi::loadAudioFile(const std::string& file_path, std::vector<float>& audio_data) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        setError("Cannot open audio file: " + file_path);
        return false;
    }
    
    // Read WAV header (more robust implementation)
    char header[44];
    file.read(header, 44);
    
    if (std::string(header, 4) != "RIFF" || std::string(header + 8, 4) != "WAVE") {
        setError("Invalid WAV file format: " + file_path);
        return false;
    }
    
    // Extract key parameters from WAV header
    uint32_t sample_rate = *reinterpret_cast<uint32_t*>(header + 24);
    uint16_t channels = *reinterpret_cast<uint16_t*>(header + 22);
    uint16_t bits_per_sample = *reinterpret_cast<uint16_t*>(header + 34);
    
    std::cout << getCurrentTimestamp() << "WAV file info: " << sample_rate << "Hz, " << channels << " channel(s), " 
              << bits_per_sample << " bits" << std::endl;
    
    // Check if audio format is supported
    if (bits_per_sample != 16) {
        setError("Unsupported bits per sample: " + std::to_string(bits_per_sample) + ". Expected 16-bit PCM.");
        return false;
    }
    
    if (sample_rate != 16000) {
        std::cout << "Info: Sample rate is " << sample_rate << "Hz. Converting to 16kHz for optimal Whisper performance." << std::endl;
    } else {
        std::cout << getCurrentTimestamp() << "Perfect! Audio is already in optimal format for Whisper (16kHz, mono, 16-bit)" << std::endl;
    }
    
    // Find data chunk
    file.seekg(36, std::ios::beg);
    char chunk_header[8];
    file.read(chunk_header, 8);
    
    // Skip to data if we're not already there
    while (std::string(chunk_header, 4) != "data" && file.good()) {
        uint32_t chunk_size = *reinterpret_cast<uint32_t*>(chunk_header + 4);
        file.seekg(chunk_size, std::ios::cur);
        file.read(chunk_header, 8);
    }
    
    if (std::string(chunk_header, 4) != "data") {
        setError("No data chunk found in WAV file");
        return false;
    }
    
    uint32_t data_size = *reinterpret_cast<uint32_t*>(chunk_header + 4);
    uint32_t num_samples = data_size / (bits_per_sample / 8) / channels;
    
    std::cout << getCurrentTimestamp() << "Loading " << num_samples << " samples (" 
              << (float)num_samples / sample_rate << " seconds)" << std::endl;
    
    // Optimize memory allocation
    audio_data.clear();
    audio_data.reserve(num_samples);
    
    // Read audio data efficiently
    std::vector<int16_t> raw_data(num_samples * channels);
    file.read(reinterpret_cast<char*>(raw_data.data()), data_size);
    file.close();
    
    // Convert to float32 and normalize - optimize for channels
    if (channels == 1) {
        // Mono - direct conversion
        audio_data.resize(num_samples);
        for (size_t i = 0; i < num_samples; ++i) {
            audio_data[i] = static_cast<float>(raw_data[i]) / 32768.0f;
        }
    } else {
        // Multi-channel - convert to mono by averaging
        audio_data.resize(num_samples);
        for (size_t i = 0; i < num_samples; ++i) {
            float sample = 0.0f;
            for (int ch = 0; ch < channels; ++ch) {
                sample += static_cast<float>(raw_data[i * channels + ch]);
            }
            audio_data[i] = (sample / channels) / 32768.0f;
        }
        std::cout << "Converted " << channels << " channels to mono" << std::endl;
    }
    
    std::cout << getCurrentTimestamp() << "Audio loaded successfully: " << audio_data.size() << " samples" << std::endl;
    return true;
}

void WhisperAi::setError(const std::string& error) {
    last_error_ = error;
    std::cerr << "WhisperAi Error: " << error << std::endl;
}

bool WhisperAi::isInitialized() const {
    std::lock_guard<std::mutex> lock(context_mutex_);
    return context_ != nullptr;
}

std::string WhisperAi::getLastError() const {
    std::lock_guard<std::mutex> lock(context_mutex_);
    return last_error_;
}

// New async methods implementation
std::future<std::string> WhisperAi::transcribeFileAsync(const std::string& audio_file_path) {
    TranscriptionTask task(TranscriptionTask::FILE, audio_file_path);
    auto future = task.result_promise.get_future();
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_queue_.push(std::move(task));
        std::cout << getCurrentTimestamp() << "Queued transcription task for file: " << audio_file_path 
                  << " (queue size: " << task_queue_.size() << ")" << std::endl;
    }
    queue_cv_.notify_one();
    
    return future;
}

std::future<std::string> WhisperAi::transcribeAudioDataAsync(std::vector<float> audio_data) {
    TranscriptionTask task(TranscriptionTask::AUDIO_DATA, std::move(audio_data));
    auto future = task.result_promise.get_future();
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_queue_.push(std::move(task));
        std::cout << getCurrentTimestamp() << "Queued transcription task for audio data"
                  << " (queue size: " << task_queue_.size() << ")" << std::endl;
    }
    queue_cv_.notify_one();
    
    return future;
}

size_t WhisperAi::getQueueSize() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return task_queue_.size();
}

// Worker thread management
void WhisperAi::startWorkerThread() {
    if (worker_running_.exchange(true)) {
        std::cout << getCurrentTimestamp() << "Worker thread already running" << std::endl;
        return;
    }
    
    shutdown_ = false;
    worker_thread_ = std::thread(&WhisperAi::workerLoop, this);
    std::cout << getCurrentTimestamp() << "Worker thread started" << std::endl;
}

void WhisperAi::stopWorkerThread() {
    if (!worker_running_.exchange(false)) {
        return; // Already stopped
    }
    
    std::cout << getCurrentTimestamp() << "Stopping worker thread..." << std::endl;
    
    // Signal shutdown
    shutdown_ = true;
    queue_cv_.notify_all();
    
    // Wait for thread to finish
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    
    // Clear any remaining tasks
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!task_queue_.empty()) {
        auto& task = task_queue_.front();
        task.result_promise.set_value(""); // Set empty result for cancelled tasks
        task_queue_.pop();
    }
    
    std::cout << getCurrentTimestamp() << "Worker thread stopped" << std::endl;
}

void WhisperAi::workerLoop() {
    std::cout << getCurrentTimestamp() << "Worker thread loop started" << std::endl;
    
    while (!shutdown_) {
        TranscriptionTask task(TranscriptionTask::FILE, ""); // Temporary initialization
        
        // Wait for work or shutdown signal
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return !task_queue_.empty() || shutdown_; });
            
            if (shutdown_) {
                break;
            }
            
            if (!task_queue_.empty()) {
                task = std::move(task_queue_.front());
                task_queue_.pop();
            } else {
                continue; // Spurious wakeup
            }
        }
        
        // Process the task
        try {
            std::cout << getCurrentTimestamp() << "Processing transcription task: " << task.task_id << std::endl;
            
            std::string result = processTask(task);
            task.result_promise.set_value(result);
            
            std::cout << getCurrentTimestamp() << "Completed transcription task: " << task.task_id << std::endl;
        } catch (const std::exception& e) {
            std::cout << getCurrentTimestamp() << "Error processing task: " << e.what() << std::endl;
            task.result_promise.set_value(""); // Set empty result on error
        }
    }
    
    std::cout << getCurrentTimestamp() << "Worker thread loop ended" << std::endl;
}

std::string WhisperAi::processTask(const TranscriptionTask& task) {
    switch (task.type) {
        case TranscriptionTask::FILE: {
            // Load audio file first
            std::vector<float> audio_data;
            if (!loadAudioFile(task.file_path, audio_data)) {
                return ""; // Error already set by loadAudioFile
            }
            
            // Process with context lock
            std::lock_guard<std::mutex> lock(context_mutex_);
            return transcribeAudioDataInternal(audio_data);
        }
        
        case TranscriptionTask::AUDIO_DATA: {
            // Process with context lock
            std::lock_guard<std::mutex> lock(context_mutex_);
            return transcribeAudioDataInternal(task.audio_data);
        }
        
        default:
            setError("Unknown task type");
            return "";
    }
}

// Static method for generating task IDs
std::string WhisperAi::TranscriptionTask::generateTaskId() {
    static std::atomic<int> counter{0};
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << "task_" << time_t << "_" << counter.fetch_add(1);
    return ss.str();
}
