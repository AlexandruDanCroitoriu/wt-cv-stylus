# Integration Guide

This document provides comprehensive guidance on integrating whisper.cpp into various types of applications and systems.

## C/C++ Integration

### Direct Library Integration

#### Basic Setup

```cpp
// whisper_wrapper.h
#ifndef WHISPER_WRAPPER_H
#define WHISPER_WRAPPER_H

#include "whisper.h"
#include <string>
#include <vector>

class WhisperWrapper {
public:
    WhisperWrapper();
    ~WhisperWrapper();
    
    bool loadModel(const std::string& model_path);
    std::string transcribeAudio(const std::vector<float>& audio_data, int sample_rate = 16000);
    std::string transcribeFile(const std::string& audio_file);
    
    // Configuration
    void setThreads(int threads);
    void setLanguage(const std::string& language);
    void setTranslate(bool translate);
    
    // Advanced features
    struct TranscriptionResult {
        std::string text;
        std::string language;
        std::vector<float> timestamps;
        float confidence;
    };
    
    TranscriptionResult transcribeWithTimestamps(const std::vector<float>& audio_data);
    
private:
    struct whisper_context* ctx;
    struct whisper_full_params params;
    bool model_loaded;
    
    void initializeParams();
};

#endif // WHISPER_WRAPPER_H
```

```cpp
// whisper_wrapper.cpp
#include "whisper_wrapper.h"
#include <iostream>
#include <fstream>

WhisperWrapper::WhisperWrapper() : ctx(nullptr), model_loaded(false) {
    initializeParams();
}

WhisperWrapper::~WhisperWrapper() {
    if (ctx) {
        whisper_free(ctx);
    }
}

void WhisperWrapper::initializeParams() {
    params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.n_threads = 4;
    params.translate = false;
    params.language = "en";
    params.print_progress = false;
    params.print_timestamps = true;
}

bool WhisperWrapper::loadModel(const std::string& model_path) {
    ctx = whisper_init_from_file(model_path.c_str());
    
    if (!ctx) {
        std::cerr << "Failed to load model: " << model_path << std::endl;
        return false;
    }
    
    model_loaded = true;
    return true;
}

std::string WhisperWrapper::transcribeAudio(const std::vector<float>& audio_data, int sample_rate) {
    if (!model_loaded) {
        return "Error: Model not loaded";
    }
    
    // Resample if needed (whisper expects 16kHz)
    std::vector<float> resampled_audio;
    if (sample_rate != 16000) {
        // Simple resampling (for production, use proper resampling library)
        float ratio = static_cast<float>(sample_rate) / 16000.0f;
        int new_size = static_cast<int>(audio_data.size() / ratio);
        resampled_audio.resize(new_size);
        
        for (int i = 0; i < new_size; ++i) {
            int src_idx = static_cast<int>(i * ratio);
            resampled_audio[i] = audio_data[src_idx];
        }
    } else {
        resampled_audio = audio_data;
    }
    
    // Run inference
    if (whisper_full(ctx, params, resampled_audio.data(), resampled_audio.size()) != 0) {
        return "Error: Transcription failed";
    }
    
    // Extract text
    std::string result;
    const int n_segments = whisper_full_n_segments(ctx);
    
    for (int i = 0; i < n_segments; ++i) {
        const char* text = whisper_full_get_segment_text(ctx, i);
        result += text;
    }
    
    return result;
}

std::string WhisperWrapper::transcribeFile(const std::string& audio_file) {
    // For simplicity, use external tool to convert to raw audio
    // In production, use audio library like libsndfile or FFmpeg
    std::string cmd = "ffmpeg -i " + audio_file + " -ar 16000 -ac 1 -f f32le /tmp/audio.raw 2>/dev/null";
    system(cmd.c_str());
    
    // Read raw audio data
    std::ifstream file("/tmp/audio.raw", std::ios::binary);
    if (!file) {
        return "Error: Could not read audio file";
    }
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg() / sizeof(float);
    file.seekg(0, std::ios::beg);
    
    std::vector<float> audio_data(size);
    file.read(reinterpret_cast<char*>(audio_data.data()), size * sizeof(float));
    
    return transcribeAudio(audio_data);
}

void WhisperWrapper::setThreads(int threads) {
    params.n_threads = threads;
}

void WhisperWrapper::setLanguage(const std::string& language) {
    params.language = language.c_str();
}

void WhisperWrapper::setTranslate(bool translate) {
    params.translate = translate;
}

WhisperWrapper::TranscriptionResult WhisperWrapper::transcribeWithTimestamps(const std::vector<float>& audio_data) {
    TranscriptionResult result;
    
    if (!model_loaded) {
        result.text = "Error: Model not loaded";
        return result;
    }
    
    // Enable timestamp output
    params.print_timestamps = true;
    
    if (whisper_full(ctx, params, audio_data.data(), audio_data.size()) != 0) {
        result.text = "Error: Transcription failed";
        return result;
    }
    
    const int n_segments = whisper_full_n_segments(ctx);
    
    for (int i = 0; i < n_segments; ++i) {
        const char* text = whisper_full_get_segment_text(ctx, i);
        result.text += text;
        
        // Get timestamps
        int64_t t0 = whisper_full_get_segment_t0(ctx, i);
        int64_t t1 = whisper_full_get_segment_t1(ctx, i);
        
        result.timestamps.push_back(t0 * 0.01f);  // Convert to seconds
        result.timestamps.push_back(t1 * 0.01f);
    }
    
    // Get detected language
    int lang_id = whisper_full_lang_id(ctx);
    result.language = whisper_lang_str(lang_id);
    
    return result;
}
```

#### CMakeLists.txt Integration

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.12)
project(MyWhisperApp)

set(CMAKE_CXX_STANDARD 17)

# Find whisper.cpp
find_package(PkgConfig REQUIRED)

# Add whisper.cpp as subdirectory or find installed version
add_subdirectory(third_party/whisper.cpp)

# Your application
add_executable(my_app
    src/main.cpp
    src/whisper_wrapper.cpp
)

target_link_libraries(my_app
    whisper
    ${CMAKE_THREAD_LIBS_INIT}
)

# Include directories
target_include_directories(my_app PRIVATE
    third_party/whisper.cpp/include
    src/
)

# Copy models to build directory
file(GLOB MODEL_FILES "models/*.bin")
foreach(MODEL_FILE ${MODEL_FILES})
    configure_file(${MODEL_FILE} ${CMAKE_BINARY_DIR}/models/ COPYONLY)
endforeach()
```

#### Usage Example

```cpp
// main.cpp
#include "whisper_wrapper.h"
#include <iostream>
#include <vector>

int main() {
    WhisperWrapper whisper;
    
    // Load model
    if (!whisper.loadModel("models/ggml-base.en.bin")) {
        std::cerr << "Failed to load model" << std::endl;
        return 1;
    }
    
    // Configure
    whisper.setThreads(4);
    whisper.setLanguage("en");
    
    // Transcribe file
    std::string result = whisper.transcribeFile("audio.wav");
    std::cout << "Transcription: " << result << std::endl;
    
    // Transcribe audio data with timestamps
    std::vector<float> audio_data;  // Your audio data here
    auto detailed_result = whisper.transcribeWithTimestamps(audio_data);
    
    std::cout << "Text: " << detailed_result.text << std::endl;
    std::cout << "Language: " << detailed_result.language << std::endl;
    std::cout << "Timestamps: ";
    for (float timestamp : detailed_result.timestamps) {
        std::cout << timestamp << "s ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

## Python Integration Patterns

### Advanced Python Wrapper

```python
# advanced_whisper.py
import subprocess
import json
import tempfile
import os
import threading
import queue
import time
from typing import List, Dict, Optional, Callable
from dataclasses import dataclass
from pathlib import Path

@dataclass
class TranscriptionSegment:
    start: float
    end: float
    text: str
    confidence: float = 0.0

@dataclass
class TranscriptionResult:
    text: str
    language: str
    segments: List[TranscriptionSegment]
    processing_time: float

class AdvancedWhisperProcessor:
    def __init__(self, 
                 model_path: str,
                 executable_path: str = "./build/bin/whisper-cli",
                 default_threads: int = 4):
        self.model_path = Path(model_path)
        self.executable_path = Path(executable_path)
        self.default_threads = default_threads
        
        if not self.model_path.exists():
            raise FileNotFoundError(f"Model not found: {model_path}")
        
        if not self.executable_path.exists():
            raise FileNotFoundError(f"Whisper executable not found: {executable_path}")
    
    def transcribe(self,
                   audio_input,  # Can be file path or audio data
                   language: Optional[str] = None,
                   translate: bool = False,
                   word_timestamps: bool = False,
                   max_segment_length: int = 0,
                   threads: Optional[int] = None,
                   callback: Optional[Callable[[str], None]] = None) -> TranscriptionResult:
        """
        Transcribe audio with advanced options
        
        Args:
            audio_input: Path to audio file or numpy array of audio data
            language: Language code ('en', 'es', etc.) or 'auto'
            translate: Translate to English
            word_timestamps: Include word-level timestamps
            max_segment_length: Maximum characters per segment
            threads: Number of threads
            callback: Progress callback function
        """
        
        start_time = time.time()
        
        # Handle different input types
        if isinstance(audio_input, (str, Path)):
            audio_file = str(audio_input)
        else:
            # Assume numpy array or similar
            audio_file = self._save_audio_data(audio_input)
        
        try:
            result = self._run_transcription(
                audio_file=audio_file,
                language=language,
                translate=translate,
                word_timestamps=word_timestamps,
                max_segment_length=max_segment_length,
                threads=threads or self.default_threads,
                callback=callback
            )
            
            processing_time = time.time() - start_time
            result.processing_time = processing_time
            
            return result
            
        finally:
            # Cleanup temporary files
            if not isinstance(audio_input, (str, Path)):
                try:
                    os.unlink(audio_file)
                except OSError:
                    pass
    
    def transcribe_stream(self,
                         audio_generator,
                         chunk_duration: float = 5.0,
                         overlap: float = 1.0) -> queue.Queue:
        """
        Transcribe streaming audio
        
        Args:
            audio_generator: Generator yielding audio chunks
            chunk_duration: Duration of each chunk in seconds
            overlap: Overlap between chunks in seconds
        
        Returns:
            Queue containing TranscriptionResult objects
        """
        result_queue = queue.Queue()
        
        def process_stream():
            audio_buffer = []
            for audio_chunk in audio_generator:
                audio_buffer.extend(audio_chunk)
                
                # Process when buffer is large enough
                if len(audio_buffer) >= chunk_duration * 16000:  # Assuming 16kHz
                    chunk_data = audio_buffer[:int(chunk_duration * 16000)]
                    
                    try:
                        result = self.transcribe(chunk_data)
                        result_queue.put(result)
                    except Exception as e:
                        result_queue.put(e)
                    
                    # Keep overlap
                    overlap_samples = int(overlap * 16000)
                    audio_buffer = audio_buffer[-overlap_samples:]
        
        # Start processing in background thread
        threading.Thread(target=process_stream, daemon=True).start()
        
        return result_queue
    
    def batch_transcribe(self,
                        audio_files: List[str],
                        output_dir: str,
                        parallel_jobs: int = 1,
                        **kwargs) -> Dict[str, TranscriptionResult]:
        """
        Batch transcribe multiple files
        """
        from concurrent.futures import ThreadPoolExecutor, as_completed
        
        results = {}
        
        def transcribe_file(file_path):
            try:
                result = self.transcribe(file_path, **kwargs)
                
                # Save individual results
                output_file = Path(output_dir) / f"{Path(file_path).stem}.json"
                with open(output_file, 'w') as f:
                    json.dump({
                        'file': str(file_path),
                        'text': result.text,
                        'language': result.language,
                        'segments': [
                            {
                                'start': seg.start,
                                'end': seg.end,
                                'text': seg.text,
                                'confidence': seg.confidence
                            }
                            for seg in result.segments
                        ],
                        'processing_time': result.processing_time
                    }, f, indent=2)
                
                return file_path, result
                
            except Exception as e:
                return file_path, e
        
        # Create output directory
        Path(output_dir).mkdir(parents=True, exist_ok=True)
        
        # Process files in parallel
        with ThreadPoolExecutor(max_workers=parallel_jobs) as executor:
            future_to_file = {
                executor.submit(transcribe_file, file_path): file_path
                for file_path in audio_files
            }
            
            for future in as_completed(future_to_file):
                file_path = future_to_file[future]
                try:
                    file_path, result = future.result()
                    results[file_path] = result
                    print(f"✓ Completed: {file_path}")
                except Exception as e:
                    results[file_path] = e
                    print(f"✗ Failed: {file_path} - {e}")
        
        return results
    
    def _save_audio_data(self, audio_data) -> str:
        """Save audio data to temporary WAV file"""
        import numpy as np
        import wave
        
        # Convert to numpy array if needed
        if not isinstance(audio_data, np.ndarray):
            audio_data = np.array(audio_data)
        
        # Ensure correct format
        if audio_data.dtype != np.int16:
            audio_data = (audio_data * 32767).astype(np.int16)
        
        # Save to temporary file
        with tempfile.NamedTemporaryFile(suffix='.wav', delete=False) as tmp:
            with wave.open(tmp.name, 'wb') as wav:
                wav.setnchannels(1)
                wav.setsampwidth(2)
                wav.setframerate(16000)
                wav.writeframes(audio_data.tobytes())
            
            return tmp.name
    
    def _run_transcription(self, **kwargs) -> TranscriptionResult:
        """Internal method to run transcription command"""
        
        # Build command arguments
        cmd = [str(self.executable_path)]
        cmd.extend(['-m', str(self.model_path)])
        cmd.extend(['-f', kwargs['audio_file']])
        cmd.extend(['-t', str(kwargs['threads'])])
        cmd.extend(['-oj'])  # JSON output
        cmd.extend(['-np'])  # No prints
        
        if kwargs.get('language'):
            cmd.extend(['-l', kwargs['language']])
        
        if kwargs.get('translate'):
            cmd.append('-tr')
        
        if kwargs.get('word_timestamps'):
            cmd.append('-owts')
        
        if kwargs.get('max_segment_length'):
            cmd.extend(['-ml', str(kwargs['max_segment_length'])])
        
        # Create temporary output file
        with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as tmp:
            tmp_path = tmp.name
        
        cmd.extend(['-of', tmp_path])
        
        try:
            # Run command
            process = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                check=True
            )
            
            # Read JSON result
            with open(f"{tmp_path}.json", 'r') as f:
                json_data = json.load(f)
            
            # Parse results
            return self._parse_json_result(json_data)
            
        except subprocess.CalledProcessError as e:
            raise Exception(f"Transcription failed: {e.stderr}")
        
        finally:
            # Cleanup
            for ext in ['', '.json', '.txt']:
                try:
                    os.unlink(f"{tmp_path}{ext}")
                except OSError:
                    pass
    
    def _parse_json_result(self, json_data: dict) -> TranscriptionResult:
        """Parse JSON output to TranscriptionResult"""
        
        text = ""
        segments = []
        
        if "transcription" in json_data:
            for segment_data in json_data["transcription"]:
                segment_text = segment_data["text"].strip()
                text += segment_text
                
                # Parse timestamps
                start_time = segment_data["offsets"]["from"] / 1000.0
                end_time = segment_data["offsets"]["to"] / 1000.0
                
                segments.append(TranscriptionSegment(
                    start=start_time,
                    end=end_time,
                    text=segment_text,
                    confidence=1.0  # whisper.cpp doesn't provide confidence scores
                ))
        
        language = json_data.get("result", {}).get("language", "unknown")
        
        return TranscriptionResult(
            text=text.strip(),
            language=language,
            segments=segments,
            processing_time=0.0  # Will be set by caller
        )

# Usage examples
if __name__ == "__main__":
    # Initialize processor
    processor = AdvancedWhisperProcessor(
        model_path="./models/ggml-base.en.bin",
        default_threads=8
    )
    
    # Single file transcription
    result = processor.transcribe(
        "meeting.wav",
        language="en",
        word_timestamps=True
    )
    
    print(f"Transcription: {result.text}")
    print(f"Language: {result.language}")
    print(f"Processing time: {result.processing_time:.2f}s")
    
    for i, segment in enumerate(result.segments):
        print(f"Segment {i}: [{segment.start:.2f}s - {segment.end:.2f}s] {segment.text}")
    
    # Batch processing
    audio_files = ["file1.wav", "file2.wav", "file3.wav"]
    batch_results = processor.batch_transcribe(
        audio_files,
        output_dir="./transcriptions",
        parallel_jobs=2,
        language="auto",
        translate=False
    )
    
    for file_path, result in batch_results.items():
        if isinstance(result, Exception):
            print(f"Error processing {file_path}: {result}")
        else:
            print(f"Successfully processed {file_path}: {len(result.text)} characters")
```

## Web Application Integration

### REST API Server

```python
# whisper_api_server.py
from flask import Flask, request, jsonify, send_file
from werkzeug.utils import secure_filename
import tempfile
import os
from pathlib import Path
import logging
from advanced_whisper import AdvancedWhisperProcessor

app = Flask(__name__)
app.config['MAX_CONTENT_LENGTH'] = 100 * 1024 * 1024  # 100MB max file size

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Initialize Whisper processor
WHISPER_PROCESSOR = AdvancedWhisperProcessor(
    model_path="./models/ggml-base.en.bin",
    default_threads=4
)

ALLOWED_EXTENSIONS = {'wav', 'mp3', 'flac', 'ogg', 'm4a', 'mp4'}

def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

@app.route('/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({
        'status': 'healthy',
        'service': 'whisper-api',
        'version': '1.0.0'
    })

@app.route('/transcribe', methods=['POST'])
def transcribe_audio():
    """
    Transcribe uploaded audio file
    
    Form parameters:
    - file: Audio file (required)
    - language: Language code (optional, default: auto-detect)
    - translate: Boolean, translate to English (optional, default: false)
    - word_timestamps: Boolean, include word timestamps (optional, default: false)
    - format: Output format - json, text, srt, vtt (optional, default: json)
    """
    
    try:
        # Check if file is present
        if 'file' not in request.files:
            return jsonify({'error': 'No file provided'}), 400
        
        file = request.files['file']
        if file.filename == '':
            return jsonify({'error': 'No file selected'}), 400
        
        if not allowed_file(file.filename):
            return jsonify({
                'error': 'Unsupported file type',
                'supported_types': list(ALLOWED_EXTENSIONS)
            }), 400
        
        # Get parameters
        language = request.form.get('language', 'auto')
        translate = request.form.get('translate', 'false').lower() == 'true'
        word_timestamps = request.form.get('word_timestamps', 'false').lower() == 'true'
        output_format = request.form.get('format', 'json').lower()
        
        # Save uploaded file temporarily
        filename = secure_filename(file.filename)
        with tempfile.NamedTemporaryFile(suffix=Path(filename).suffix, delete=False) as tmp:
            file.save(tmp.name)
            temp_file_path = tmp.name
        
        try:
            # Transcribe
            logger.info(f"Transcribing file: {filename}")
            
            result = WHISPER_PROCESSOR.transcribe(
                audio_input=temp_file_path,
                language=language if language != 'auto' else None,
                translate=translate,
                word_timestamps=word_timestamps
            )
            
            logger.info(f"Transcription completed in {result.processing_time:.2f}s")
            
            # Format response based on requested format
            if output_format == 'json':
                return jsonify({
                    'text': result.text,
                    'language': result.language,
                    'processing_time': result.processing_time,
                    'segments': [
                        {
                            'start': seg.start,
                            'end': seg.end,
                            'text': seg.text,
                            'confidence': seg.confidence
                        }
                        for seg in result.segments
                    ] if word_timestamps else None
                })
            
            elif output_format == 'text':
                return result.text, 200, {'Content-Type': 'text/plain'}
            
            elif output_format == 'srt':
                srt_content = _convert_to_srt(result.segments)
                return srt_content, 200, {
                    'Content-Type': 'text/plain',
                    'Content-Disposition': f'attachment; filename="{Path(filename).stem}.srt"'
                }
            
            elif output_format == 'vtt':
                vtt_content = _convert_to_vtt(result.segments)
                return vtt_content, 200, {
                    'Content-Type': 'text/vtt',
                    'Content-Disposition': f'attachment; filename="{Path(filename).stem}.vtt"'
                }
            
            else:
                return jsonify({'error': f'Unsupported format: {output_format}'}), 400
        
        finally:
            # Cleanup
            try:
                os.unlink(temp_file_path)
            except OSError:
                pass
    
    except Exception as e:
        logger.error(f"Transcription error: {str(e)}")
        return jsonify({'error': str(e)}), 500

@app.route('/batch_transcribe', methods=['POST'])
def batch_transcribe():
    """
    Batch transcribe multiple files
    
    Expects multipart/form-data with multiple 'files' fields
    """
    
    try:
        files = request.files.getlist('files')
        
        if not files:
            return jsonify({'error': 'No files provided'}), 400
        
        # Validate all files
        for file in files:
            if not allowed_file(file.filename):
                return jsonify({
                    'error': f'Unsupported file type: {file.filename}',
                    'supported_types': list(ALLOWED_EXTENSIONS)
                }), 400
        
        # Save files temporarily
        temp_files = []
        try:
            for file in files:
                filename = secure_filename(file.filename)
                with tempfile.NamedTemporaryFile(suffix=Path(filename).suffix, delete=False) as tmp:
                    file.save(tmp.name)
                    temp_files.append((tmp.name, filename))
            
            # Get parameters
            language = request.form.get('language', 'auto')
            translate = request.form.get('translate', 'false').lower() == 'true'
            
            # Process files
            results = {}
            
            for temp_path, original_filename in temp_files:
                try:
                    result = WHISPER_PROCESSOR.transcribe(
                        audio_input=temp_path,
                        language=language if language != 'auto' else None,
                        translate=translate
                    )
                    
                    results[original_filename] = {
                        'text': result.text,
                        'language': result.language,
                        'processing_time': result.processing_time,
                        'status': 'success'
                    }
                    
                except Exception as e:
                    results[original_filename] = {
                        'error': str(e),
                        'status': 'error'
                    }
            
            return jsonify({'results': results})
        
        finally:
            # Cleanup all temp files
            for temp_path, _ in temp_files:
                try:
                    os.unlink(temp_path)
                except OSError:
                    pass
    
    except Exception as e:
        logger.error(f"Batch transcription error: {str(e)}")
        return jsonify({'error': str(e)}), 500

@app.route('/models', methods=['GET'])
def list_models():
    """List available models"""
    models_dir = Path('./models')
    models = []
    
    if models_dir.exists():
        for model_file in models_dir.glob('ggml-*.bin'):
            models.append({
                'name': model_file.name,
                'size': model_file.stat().st_size,
                'path': str(model_file)
            })
    
    return jsonify({'models': models})

def _convert_to_srt(segments):
    """Convert segments to SRT format"""
    srt_content = ""
    
    for i, segment in enumerate(segments, 1):
        start_time = _seconds_to_srt_time(segment.start)
        end_time = _seconds_to_srt_time(segment.end)
        
        srt_content += f"{i}\n"
        srt_content += f"{start_time} --> {end_time}\n"
        srt_content += f"{segment.text.strip()}\n\n"
    
    return srt_content

def _convert_to_vtt(segments):
    """Convert segments to WebVTT format"""
    vtt_content = "WEBVTT\n\n"
    
    for segment in segments:
        start_time = _seconds_to_vtt_time(segment.start)
        end_time = _seconds_to_vtt_time(segment.end)
        
        vtt_content += f"{start_time} --> {end_time}\n"
        vtt_content += f"{segment.text.strip()}\n\n"
    
    return vtt_content

def _seconds_to_srt_time(seconds):
    """Convert seconds to SRT time format (HH:MM:SS,mmm)"""
    hours = int(seconds // 3600)
    minutes = int((seconds % 3600) // 60)
    secs = int(seconds % 60)
    millisecs = int((seconds % 1) * 1000)
    
    return f"{hours:02d}:{minutes:02d}:{secs:02d},{millisecs:03d}"

def _seconds_to_vtt_time(seconds):
    """Convert seconds to WebVTT time format (HH:MM:SS.mmm)"""
    hours = int(seconds // 3600)
    minutes = int((seconds % 3600) // 60)
    secs = int(seconds % 60)
    millisecs = int((seconds % 1) * 1000)
    
    return f"{hours:02d}:{minutes:02d}:{secs:02d}.{millisecs:03d}"

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, debug=False)
```

### Frontend Integration

```html
<!-- whisper-client.html -->
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Whisper Transcription Service</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
        }
        
        .upload-area {
            border: 2px dashed #ccc;
            border-radius: 10px;
            padding: 40px;
            text-align: center;
            margin: 20px 0;
            transition: border-color 0.3s;
        }
        
        .upload-area.dragover {
            border-color: #007bff;
            background-color: #f8f9fa;
        }
        
        .options {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin: 20px 0;
        }
        
        .option-group {
            display: flex;
            flex-direction: column;
        }
        
        label {
            font-weight: bold;
            margin-bottom: 5px;
        }
        
        select, input {
            padding: 8px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        
        button {
            background-color: #007bff;
            color: white;
            padding: 12px 24px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
        }
        
        button:hover {
            background-color: #0056b3;
        }
        
        button:disabled {
            background-color: #6c757d;
            cursor: not-allowed;
        }
        
        .progress {
            display: none;
            margin: 20px 0;
        }
        
        .progress-bar {
            width: 100%;
            height: 20px;
            background-color: #f0f0f0;
            border-radius: 10px;
            overflow: hidden;
        }
        
        .progress-fill {
            height: 100%;
            background-color: #007bff;
            width: 0%;
            transition: width 0.3s;
        }
        
        .results {
            margin-top: 30px;
            padding: 20px;
            background-color: #f8f9fa;
            border-radius: 8px;
            display: none;
        }
        
        .segment {
            margin: 10px 0;
            padding: 10px;
            background-color: white;
            border-radius: 4px;
            border-left: 4px solid #007bff;
        }
        
        .timestamp {
            font-size: 12px;
            color: #666;
            margin-bottom: 5px;
        }
        
        .error {
            color: #dc3545;
            background-color: #f8d7da;
            border: 1px solid #f5c6cb;
            padding: 10px;
            border-radius: 4px;
            margin: 10px 0;
        }
    </style>
</head>
<body>
    <h1>🎤 Whisper Transcription Service</h1>
    
    <div class="upload-area" id="uploadArea">
        <p>📁 Drag and drop audio files here or click to select</p>
        <input type="file" id="fileInput" multiple accept="audio/*" style="display: none;">
        <button onclick="document.getElementById('fileInput').click()">Select Files</button>
    </div>
    
    <div class="options">
        <div class="option-group">
            <label for="language">Language:</label>
            <select id="language">
                <option value="auto">Auto-detect</option>
                <option value="en">English</option>
                <option value="es">Spanish</option>
                <option value="fr">French</option>
                <option value="de">German</option>
                <option value="it">Italian</option>
                <option value="pt">Portuguese</option>
                <option value="ru">Russian</option>
                <option value="ja">Japanese</option>
                <option value="ko">Korean</option>
                <option value="zh">Chinese</option>
            </select>
        </div>
        
        <div class="option-group">
            <label for="format">Output Format:</label>
            <select id="format">
                <option value="json">JSON (with metadata)</option>
                <option value="text">Plain Text</option>
                <option value="srt">SRT Subtitles</option>
                <option value="vtt">WebVTT Subtitles</option>
            </select>
        </div>
        
        <div class="option-group">
            <label>
                <input type="checkbox" id="translate"> Translate to English
            </label>
        </div>
        
        <div class="option-group">
            <label>
                <input type="checkbox" id="timestamps"> Include Word Timestamps
            </label>
        </div>
    </div>
    
    <button id="transcribeBtn" onclick="transcribeFiles()">🚀 Start Transcription</button>
    
    <div class="progress" id="progress">
        <div class="progress-bar">
            <div class="progress-fill" id="progressFill"></div>
        </div>
        <p id="progressText">Processing...</p>
    </div>
    
    <div class="results" id="results"></div>

    <script>
        let selectedFiles = [];
        
        // File upload handling
        const uploadArea = document.getElementById('uploadArea');
        const fileInput = document.getElementById('fileInput');
        
        uploadArea.addEventListener('dragover', (e) => {
            e.preventDefault();
            uploadArea.classList.add('dragover');
        });
        
        uploadArea.addEventListener('dragleave', () => {
            uploadArea.classList.remove('dragover');
        });
        
        uploadArea.addEventListener('drop', (e) => {
            e.preventDefault();
            uploadArea.classList.remove('dragover');
            
            const files = Array.from(e.dataTransfer.files);
            handleFiles(files);
        });
        
        fileInput.addEventListener('change', (e) => {
            const files = Array.from(e.target.files);
            handleFiles(files);
        });
        
        function handleFiles(files) {
            selectedFiles = files.filter(file => file.type.startsWith('audio/'));
            
            if (selectedFiles.length === 0) {
                showError('Please select valid audio files');
                return;
            }
            
            const fileNames = selectedFiles.map(f => f.name).join(', ');
            uploadArea.innerHTML = `
                <p>✅ Selected ${selectedFiles.length} file(s):</p>
                <p style="font-size: 14px; color: #666;">${fileNames}</p>
                <button onclick="document.getElementById('fileInput').click()">Change Selection</button>
            `;
        }
        
        async function transcribeFiles() {
            if (selectedFiles.length === 0) {
                showError('Please select audio files first');
                return;
            }
            
            const transcribeBtn = document.getElementById('transcribeBtn');
            const progress = document.getElementById('progress');
            const results = document.getElementById('results');
            
            transcribeBtn.disabled = true;
            progress.style.display = 'block';
            results.style.display = 'none';
            
            try {
                if (selectedFiles.length === 1) {
                    await transcribeSingleFile(selectedFiles[0]);
                } else {
                    await transcribeBatchFiles(selectedFiles);
                }
            } catch (error) {
                showError(`Transcription failed: ${error.message}`);
            } finally {
                transcribeBtn.disabled = false;
                progress.style.display = 'none';
            }
        }
        
        async function transcribeSingleFile(file) {
            const formData = new FormData();
            formData.append('file', file);
            formData.append('language', document.getElementById('language').value);
            formData.append('format', document.getElementById('format').value);
            formData.append('translate', document.getElementById('translate').checked);
            formData.append('word_timestamps', document.getElementById('timestamps').checked);
            
            updateProgress(0, `Processing ${file.name}...`);
            
            const response = await fetch('/transcribe', {
                method: 'POST',
                body: formData
            });
            
            if (!response.ok) {
                const error = await response.json();
                throw new Error(error.error || 'Transcription failed');
            }
            
            updateProgress(100, 'Complete!');
            
            const format = document.getElementById('format').value;
            
            if (format === 'json') {
                const result = await response.json();
                displayJsonResult(file.name, result);
            } else {
                const text = await response.text();
                displayTextResult(file.name, text, format);
            }
        }
        
        async function transcribeBatchFiles(files) {
            const formData = new FormData();
            
            files.forEach(file => {
                formData.append('files', file);
            });
            
            formData.append('language', document.getElementById('language').value);
            formData.append('translate', document.getElementById('translate').checked);
            
            updateProgress(0, `Processing ${files.length} files...`);
            
            const response = await fetch('/batch_transcribe', {
                method: 'POST',
                body: formData
            });
            
            if (!response.ok) {
                const error = await response.json();
                throw new Error(error.error || 'Batch transcription failed');
            }
            
            updateProgress(100, 'Complete!');
            
            const result = await response.json();
            displayBatchResults(result.results);
        }
        
        function updateProgress(percent, text) {
            document.getElementById('progressFill').style.width = `${percent}%`;
            document.getElementById('progressText').textContent = text;
        }
        
        function displayJsonResult(filename, result) {
            const results = document.getElementById('results');
            results.style.display = 'block';
            
            let html = `
                <h3>📝 Transcription Results: ${filename}</h3>
                <p><strong>Language:</strong> ${result.language}</p>
                <p><strong>Processing Time:</strong> ${result.processing_time.toFixed(2)}s</p>
                <div class="segment">
                    <h4>Full Text:</h4>
                    <p>${result.text}</p>
                </div>
            `;
            
            if (result.segments) {
                html += '<h4>Segments with Timestamps:</h4>';
                result.segments.forEach((segment, index) => {
                    html += `
                        <div class="segment">
                            <div class="timestamp">
                                ${formatTime(segment.start)} - ${formatTime(segment.end)}
                            </div>
                            <p>${segment.text}</p>
                        </div>
                    `;
                });
            }
            
            results.innerHTML = html;
        }
        
        function displayTextResult(filename, text, format) {
            const results = document.getElementById('results');
            results.style.display = 'block';
            
            const downloadBtn = `
                <button onclick="downloadText('${filename}', '${format}')">
                    💾 Download ${format.toUpperCase()}
                </button>
            `;
            
            results.innerHTML = `
                <h3>📝 Transcription Results: ${filename}</h3>
                ${downloadBtn}
                <div class="segment">
                    <pre style="white-space: pre-wrap; font-family: inherit;">${text}</pre>
                </div>
            `;
            
            // Store text for download
            window.lastTranscriptionText = text;
            window.lastTranscriptionFilename = filename;
        }
        
        function displayBatchResults(results) {
            const resultsDiv = document.getElementById('results');
            resultsDiv.style.display = 'block';
            
            let html = '<h3>📁 Batch Transcription Results</h3>';
            
            Object.entries(results).forEach(([filename, result]) => {
                if (result.status === 'success') {
                    html += `
                        <div class="segment">
                            <h4>✅ ${filename}</h4>
                            <p><strong>Language:</strong> ${result.language}</p>
                            <p><strong>Processing Time:</strong> ${result.processing_time.toFixed(2)}s</p>
                            <p><strong>Text:</strong> ${result.text.substring(0, 200)}${result.text.length > 200 ? '...' : ''}</p>
                        </div>
                    `;
                } else {
                    html += `
                        <div class="segment error">
                            <h4>❌ ${filename}</h4>
                            <p><strong>Error:</strong> ${result.error}</p>
                        </div>
                    `;
                }
            });
            
            resultsDiv.innerHTML = html;
        }
        
        function downloadText(filename, format) {
            const text = window.lastTranscriptionText;
            const blob = new Blob([text], { type: 'text/plain' });
            const url = URL.createObjectURL(blob);
            
            const a = document.createElement('a');
            a.href = url;
            a.download = `${filename.split('.')[0]}.${format}`;
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            URL.revokeObjectURL(url);
        }
        
        function formatTime(seconds) {
            const minutes = Math.floor(seconds / 60);
            const secs = (seconds % 60).toFixed(1);
            return `${minutes}:${secs.padStart(4, '0')}`;
        }
        
        function showError(message) {
            const results = document.getElementById('results');
            results.style.display = 'block';
            results.innerHTML = `<div class="error">${message}</div>`;
        }
    </script>
</body>
</html>
```

## Docker Deployment

### Multi-stage Dockerfile

```dockerfile
# Dockerfile.production
FROM ubuntu:22.04 as builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    python3 \
    python3-pip \
    libsdl2-dev \
    && rm -rf /var/lib/apt/lists/*

# Build whisper.cpp
WORKDIR /whisper
COPY . .

# Build with optimizations
RUN cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DWHISPER_SDL2=ON \
    -DCMAKE_C_FLAGS="-O3 -march=native" \
    -DCMAKE_CXX_FLAGS="-O3 -march=native"

RUN cmake --build build --config Release -j$(nproc)

# Production stage
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    ffmpeg \
    libsdl2-2.0-0 \
    && rm -rf /var/lib/apt/lists/*

# Install Python dependencies
COPY requirements.txt /tmp/
RUN pip3 install -r /tmp/requirements.txt

# Copy built binaries
COPY --from=builder /whisper/build/bin/ /usr/local/bin/

# Copy application code
COPY whisper_api_server.py /app/
COPY advanced_whisper.py /app/
WORKDIR /app

# Create models directory
RUN mkdir -p /app/models

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=30s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# Expose port
EXPOSE 8080

# Run as non-root user
RUN useradd -m -u 1000 whisper
USER whisper

# Start server
CMD ["python3", "whisper_api_server.py"]
```

### Docker Compose

```yaml
# docker-compose.yml
version: '3.8'

services:
  whisper-api:
    build:
      context: .
      dockerfile: Dockerfile.production
    ports:
      - "8080:8080"
    volumes:
      - ./models:/app/models:ro
      - ./uploads:/tmp/uploads
    environment:
      - FLASK_ENV=production
      - WHISPER_MODEL_PATH=/app/models/ggml-base.en.bin
      - WHISPER_THREADS=4
    restart: unless-stopped
    deploy:
      resources:
        limits:
          memory: 4G
          cpus: '2.0'
        reservations:
          memory: 2G
          cpus: '1.0'
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8080/health"]
      interval: 30s
      timeout: 10s
      retries: 3
      start_period: 30s

  nginx:
    image: nginx:alpine
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf:ro
      - ./ssl:/etc/nginx/ssl:ro
      - ./static:/usr/share/nginx/html:ro
    depends_on:
      - whisper-api
    restart: unless-stopped

  prometheus:
    image: prom/prometheus
    ports:
      - "9090:9090"
    volumes:
      - ./prometheus.yml:/etc/prometheus/prometheus.yml:ro
    restart: unless-stopped

  grafana:
    image: grafana/grafana
    ports:
      - "3000:3000"
    environment:
      - GF_SECURITY_ADMIN_PASSWORD=admin
    volumes:
      - grafana-data:/var/lib/grafana
    restart: unless-stopped

volumes:
  grafana-data:
```

### Kubernetes Deployment

```yaml
# k8s-deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: whisper-api
  labels:
    app: whisper-api
spec:
  replicas: 3
  selector:
    matchLabels:
      app: whisper-api
  template:
    metadata:
      labels:
        app: whisper-api
    spec:
      containers:
      - name: whisper-api
        image: whisper-api:latest
        ports:
        - containerPort: 8080
        env:
        - name: WHISPER_THREADS
          value: "4"
        - name: FLASK_ENV
          value: "production"
        resources:
          requests:
            memory: "2Gi"
            cpu: "1"
          limits:
            memory: "4Gi"
            cpu: "2"
        volumeMounts:
        - name: models
          mountPath: /app/models
          readOnly: true
        livenessProbe:
          httpGet:
            path: /health
            port: 8080
          initialDelaySeconds: 30
          periodSeconds: 10
        readinessProbe:
          httpGet:
            path: /health
            port: 8080
          initialDelaySeconds: 5
          periodSeconds: 5
      volumes:
      - name: models
        configMap:
          name: whisper-models
---
apiVersion: v1
kind: Service
metadata:
  name: whisper-api-service
spec:
  selector:
    app: whisper-api
  ports:
  - protocol: TCP
    port: 80
    targetPort: 8080
  type: LoadBalancer
---
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: whisper-api-ingress
  annotations:
    nginx.ingress.kubernetes.io/proxy-body-size: "100m"
    nginx.ingress.kubernetes.io/proxy-read-timeout: "300"
    nginx.ingress.kubernetes.io/proxy-send-timeout: "300"
spec:
  rules:
  - host: whisper-api.example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: whisper-api-service
            port:
              number: 80
```

## Next Steps

- **Performance Optimization**: Review [performance tuning](performance.md)
- **Advanced Features**: Explore [specialized examples](advanced-examples.md)
- **Language Bindings**: Check [mobile and other language examples](language-bindings.md)
- **Web Examples**: Try [WebAssembly integration](web-examples.md)
