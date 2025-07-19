#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <thread>
#include <cstring>
#include <vector>
#include <chrono>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include "whisper.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class WhisperService {
private:
    whisper_context* context_;
    std::string model_path_;
    
public:
    WhisperService() : context_(nullptr) {}
    
    ~WhisperService() {
        if (context_) {
            whisper_free(context_);
        }
    }
    
    bool initialize(const std::string& model_path, json& init_info) {
        model_path_ = model_path;
        init_info["model_path"] = model_path;
        
        if (!std::filesystem::exists(model_path)) {
            init_info["error"] = "Model file not found: " + model_path;
            return false;
        }
        
        // Initialize with default parameters for better compatibility
        whisper_context_params ctx_params = whisper_context_default_params();
        
        // Note: We'll capture whisper output using a different approach in the future
        // For now, we'll let whisper output go to stderr and document it in our JSON
        init_info["note"] = "Whisper library outputs initialization details to stderr";
        
        context_ = whisper_init_from_file_with_params(model_path.c_str(), ctx_params);
        
        if (!context_) {
            init_info["error"] = "Failed to load model from " + model_path;
            return false;
        }
        
        init_info["success"] = true;
        init_info["model_loaded"] = true;
        return true;
    }
    
    std::string transcribeFile(const std::string& audio_file_path) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        json response;
        response["success"] = false;
        response["audio_file"] = audio_file_path;
        response["model_path"] = model_path_;
        response["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        
        if (!context_) {
            response["error"] = "Whisper not initialized";
            return response.dump();
        }
        
        if (!std::filesystem::exists(audio_file_path)) {
            response["error"] = "Audio file not found: " + audio_file_path;
            return response.dump();
        }
        
        // Load audio file
        json audio_info;
        std::vector<float> audio_data;
        if (!loadAudioFile(audio_file_path, audio_data, audio_info)) {
            response["error"] = "Failed to load audio file: " + audio_file_path;
            response["audio_info"] = audio_info;
            return response.dump();
        }
        
        // Add audio info to response
        response["audio_info"] = audio_info;
        
        // Prepare parameters
        whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
        params.print_realtime = false;
        params.print_progress = false;
        params.print_timestamps = false;
        params.print_special = false;
        params.translate = false;
        params.language = "en";
        params.n_threads = std::min(4, (int)std::thread::hardware_concurrency());
        
        // Add processing info
        response["processing_info"] = {
            {"language", "en"},
            {"threads", params.n_threads},
            {"model_type", "base"}
        };
        
        // Perform transcription
        auto transcription_start = std::chrono::high_resolution_clock::now();
        int result = whisper_full(context_, params, audio_data.data(), audio_data.size());
        auto transcription_end = std::chrono::high_resolution_clock::now();
        
        if (result != 0) {
            response["error"] = "Transcription failed with code: " + std::to_string(result);
            return response.dump();
        }
        
        // Extract transcribed text and segments
        std::string transcription;
        json segments = json::array();
        const int n_segments = whisper_full_n_segments(context_);
        
        for (int i = 0; i < n_segments; ++i) {
            const char* text = whisper_full_get_segment_text(context_, i);
            if (text) {
                transcription += text;
                
                // Add segment info
                json segment;
                segment["id"] = i;
                segment["text"] = text;
                segment["start_time"] = whisper_full_get_segment_t0(context_, i) * 0.01; // Convert to seconds
                segment["end_time"] = whisper_full_get_segment_t1(context_, i) * 0.01;   // Convert to seconds
                segments.push_back(segment);
            }
        }
        
        // Trim whitespace from full transcription
        transcription.erase(0, transcription.find_first_not_of(" \t\n\r"));
        transcription.erase(transcription.find_last_not_of(" \t\n\r") + 1);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        auto transcription_duration = std::chrono::duration_cast<std::chrono::milliseconds>(transcription_end - transcription_start);
        
        // Build successful response
        response["success"] = true;
        response["transcription"] = transcription;
        response["segments"] = segments;
        response["timing"] = {
            {"total_processing_ms", total_duration.count()},
            {"transcription_ms", transcription_duration.count()},
            {"real_time_factor", (audio_data.size() / 16000.0) / (total_duration.count() / 1000.0)}
        };
        
        return response.dump();
    }
    
private:
    bool loadAudioFile(const std::string& file_path, std::vector<float>& audio_data, json& audio_info) {
        // Simple WAV file loader for 16kHz mono format
        std::ifstream file(file_path, std::ios::binary);
        if (!file) {
            audio_info["error"] = "Cannot open audio file: " + file_path;
            return false;
        }
        
        // Read WAV header (44 bytes)
        char header[44];
        file.read(header, 44);
        if (!file || file.gcount() != 44) {
            audio_info["error"] = "Invalid WAV header";
            return false;
        }
        
        // Check if it's a valid WAV file
        if (std::memcmp(header, "RIFF", 4) != 0 || std::memcmp(header + 8, "WAVE", 4) != 0) {
            audio_info["error"] = "Not a valid WAV file";
            return false;
        }
        
        // Extract format information
        uint16_t audio_format = *reinterpret_cast<uint16_t*>(header + 20);
        uint16_t num_channels = *reinterpret_cast<uint16_t*>(header + 22);
        uint32_t sample_rate = *reinterpret_cast<uint32_t*>(header + 24);
        uint16_t bits_per_sample = *reinterpret_cast<uint16_t*>(header + 34);
        
        // Store audio format info
        audio_info["format"] = {
            {"audio_format", audio_format},
            {"channels", num_channels},
            {"sample_rate", sample_rate},
            {"bits_per_sample", bits_per_sample}
        };
        
        // Verify format (expecting 16-bit PCM)
        if (audio_format != 1) {
            audio_info["error"] = "Only PCM format supported";
            return false;
        }
        
        if (bits_per_sample != 16) {
            audio_info["error"] = "Only 16-bit audio supported";
            return false;
        }
        
        // Read audio data
        std::vector<int16_t> raw_data;
        int16_t sample;
        while (file.read(reinterpret_cast<char*>(&sample), sizeof(sample))) {
            raw_data.push_back(sample);
        }
        
        if (raw_data.empty()) {
            audio_info["error"] = "No audio data found";
            return false;
        }
        
        // Convert to float and handle multi-channel by taking average
        audio_data.clear();
        audio_data.reserve(raw_data.size() / num_channels);
        
        for (size_t i = 0; i < raw_data.size(); i += num_channels) {
            float sample_sum = 0.0f;
            for (uint16_t ch = 0; ch < num_channels && i + ch < raw_data.size(); ++ch) {
                sample_sum += static_cast<float>(raw_data[i + ch]) / 32768.0f;
            }
            audio_data.push_back(sample_sum / num_channels);
        }
        
        audio_info["samples"] = audio_data.size();
        audio_info["duration_seconds"] = audio_data.size() / 16000.0;
        audio_info["raw_samples"] = raw_data.size();
        
        return true;
    }
};

void printUsage(const char* program_name) {
    json usage_response;
    usage_response["success"] = false;
    usage_response["error"] = "Usage: " + std::string(program_name) + " <model_path> <audio_file_path>";
    usage_response["example"] = std::string(program_name) + " models/ggml-base.en.bin audio.wav";
    std::cout << usage_response.dump() << std::endl;
}

int main(int argc, char* argv[]) {
    // Redirect stderr to /dev/null to suppress whisper debug output
    freopen("/dev/null", "w", stderr);
    
    if (argc != 3) {
        json error_response;
        error_response["success"] = false;
        error_response["error"] = "Usage: " + std::string(argv[0]) + " <model_path> <audio_file_path>";
        std::cout << error_response.dump() << std::endl;
        return 1;
    }
    
    std::string model_path = argv[1];
    std::string audio_file_path = argv[2];
    
    WhisperService service;
    
    // Initialize with model and capture initialization info
    json init_info;
    if (!service.initialize(model_path, init_info)) {
        json error_response;
        error_response["success"] = false;
        error_response["error"] = "Failed to initialize Whisper service";
        error_response["initialization"] = init_info;
        std::cout << error_response.dump() << std::endl;
        return 1;
    }
    
    // Transcribe audio file
    std::string result = service.transcribeFile(audio_file_path);
    
    // Parse the result to add initialization info
    json final_response = json::parse(result);
    final_response["initialization"] = init_info;
    
    // Output result to stdout (main app will read this)
    std::cout << final_response.dump() << std::endl;
    
    return 0;
}
