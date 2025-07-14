#include "WhisperWrapper.h"
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

WhisperWrapper::WhisperWrapper() 
    : context_(nullptr), language_("auto") {
}

WhisperWrapper::~WhisperWrapper() {
    if (context_) {
        whisper_free(context_);
        context_ = nullptr;
    }
}

bool WhisperWrapper::initialize(const std::string& model_path) {
    // Check if model file exists
    std::ifstream file(model_path);
    if (!file.good()) {
        setError("Model file not found: " + model_path);
        return false;
    }
    file.close();

    // Initialize whisper context with optimized parameters
    struct whisper_context_params cparams = whisper_context_default_params();
    cparams.use_gpu = false; // Disable GPU for stability and consistency
    
    context_ = whisper_init_from_file_with_params(model_path.c_str(), cparams);
    
    if (!context_) {
        setError("Failed to initialize whisper context from model: " + model_path);
        return false;
    }
    
    std::cout << "Whisper initialized successfully with model: " << model_path << std::endl;
    std::cout << "Available threads: " << std::thread::hardware_concurrency() << std::endl;
    
    return true;
}

std::string WhisperWrapper::transcribeFile(const std::string& audio_file_path) {
    if (!isInitialized()) {
        setError("Whisper not initialized");
        return "";
    }
    
    std::string wav_file = audio_file_path;
    std::string converted_file = "";
    
    // Check if file is already WAV format, if not convert it
    if (audio_file_path.substr(audio_file_path.find_last_of(".") + 1) != "wav") {
        converted_file = convertAndSaveAudioToWav(audio_file_path);
        if (converted_file.empty()) {
            setError("Failed to convert audio file to WAV format");
            return "";
        }
        wav_file = converted_file;
        std::cout << "Using converted WAV file: " << wav_file << std::endl;
    }
    
    // Load audio data
    std::vector<float> audio_data;
    if (!loadAudioFile(wav_file, audio_data)) {
        return "";
    }
    
    return transcribeAudioData(audio_data);
}

std::string WhisperWrapper::transcribeAudioData(const std::vector<float>& audio_data) {
    if (!isInitialized()) {
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
    
    // Set language if specified
    if (language_ != "auto") {
        wparams.language = language_.c_str();
    }
    
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

bool WhisperWrapper::convertAudioToWav(const std::string& input_file, const std::string& output_file) {
    // Use ffmpeg to convert audio to 16kHz mono WAV
    std::string command = "ffmpeg -i \"" + input_file + "\" -ar 16000 -ac 1 -c:a pcm_s16le \"" + output_file + "\" -y 2>/dev/null";
    
    int result = std::system(command.c_str());
    
    if (result != 0) {
        std::cerr << "Failed to convert audio file. Make sure ffmpeg is installed." << std::endl;
        return false;
    }
    
    return true;
}

std::vector<std::string> WhisperWrapper::getSupportedLanguages() {
    return {
        "auto", "en", "zh", "de", "es", "ru", "ko", "fr", "ja", "pt", "tr", "pl", "ca", "nl", 
        "ar", "sv", "it", "id", "hi", "fi", "vi", "he", "uk", "el", "ms", "cs", "ro", "da",
        "hu", "ta", "no", "th", "ur", "hr", "bg", "lt", "la", "mi", "ml", "cy", "sk", "te",
        "fa", "lv", "bn", "sr", "az", "sl", "kn", "et", "mk", "br", "eu", "is", "hy", "ne",
        "mn", "bs", "kk", "sq", "sw", "gl", "mr", "pa", "si", "km", "sn", "yo", "so", "af",
        "oc", "ka", "be", "tg", "sd", "gu", "am", "yi", "lo", "uz", "fo", "ht", "ps", "tk",
        "nn", "mt", "sa", "lb", "my", "bo", "tl", "mg", "as", "tt", "haw", "ln", "ha", "ba",
        "jw", "su"
    };
}

void WhisperWrapper::setLanguage(const std::string& language) {
    language_ = language;
}

bool WhisperWrapper::loadAudioFile(const std::string& file_path, std::vector<float>& audio_data) {
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
        std::cout << "Warning: Sample rate is " << sample_rate << "Hz, but Whisper expects 16kHz. "
                  << "Consider re-encoding the audio file." << std::endl;
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

void WhisperWrapper::setError(const std::string& error) {
    last_error_ = error;
    std::cerr << "WhisperWrapper Error: " << error << std::endl;
}

std::string WhisperWrapper::convertAndSaveAudioToWav(const std::string& input_file)
{
    // Generate output filename by replacing extension with .wav
    std::string output_file = input_file;
    size_t lastDot = output_file.find_last_of('.');
    if (lastDot != std::string::npos) {
        output_file = output_file.substr(0, lastDot) + "_converted.wav";
    } else {
        output_file += "_converted.wav";
    }
    
    // Convert to WAV format
    if (convertAudioToWav(input_file, output_file)) {
        std::cout << getCurrentTimestamp() << "Converted WAV file saved: " << output_file << std::endl;
        return output_file;
    } else {
        std::cerr << "Failed to convert audio file: " << input_file << std::endl;
        return "";
    }
}
