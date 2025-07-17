# Getting Started with Whisper.cpp in WT-CV-Stylus Project

This guide will help you get started with whisper.cpp examples within your WT (C++) web application project.

## Prerequisites

### System Requirements
- Modern CPU (x86_64 or ARM64)
- Minimum 4GB RAM (8GB+ recommended for larger models)
- C++17 compatible compiler (your project uses C++17)
- CMake 3.13 or higher (as specified in your CMakeLists.txt)

### Dependencies

**Required for all examples:**
- No external dependencies for basic CLI usage (whisper.cpp is integrated via CMake)

**Required for microphone examples (stream, command):**
```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev

# Fedora/RHEL
sudo dnf install SDL2-devel

# macOS
brew install sdl2
```

**Optional for advanced features:**
- CUDA for GPU acceleration
- OpenBLAS for optimized CPU computation
- Metal Performance Shaders (macOS)

## Building the Examples

### 1. Build Your Project with Whisper Examples
```bash
# Navigate to your project root
cd /home/alex/github-repos/wt-cv-stylus-copilot

# Build with whisper examples enabled
cmake -DCMAKE_BUILD_TYPE=Debug -DWHISPER_BUILD_EXAMPLES=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/debug

# Build the project
cd build/debug && make -j$(nproc)

# For release build
cd /home/alex/github-repos/wt-cv-stylus-copilot
cmake -DCMAKE_BUILD_TYPE=Release -DWHISPER_BUILD_EXAMPLES=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/release
cd build/release && make -j$(nproc)
```

### 2. Models (Already Available)
Your project already has models in the `./models/` directory:
```bash
# Check available models
ls -la ./models/
# You should see: ggml-base.en.bin and others
```

### 3. Verify Installation
```bash
# Navigate to build directory
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd

# Test with your available model
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/audio_20250716_215632_411.wav

# ✅ This works and outputs: "Hello, my name is Alex, and I'm a programmer."
```

## Your First Transcription

### 1. Basic File Transcription
```bash
# Navigate to build directory
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd

# Transcribe an audio file
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/your-audio.wav

# With output to file
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/your-audio.wav -otxt

# Multiple output formats
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/your-audio.wav -otxt -osrt -ovtt
```

### 2. Supported Audio Formats
- WAV (recommended)
- MP3
- FLAC
- OGG

**Converting audio to supported format:**
```bash
# Using ffmpeg
ffmpeg -i input.mp4 -ar 16000 -ac 1 output.wav
ffmpeg -i input.m4a -ar 16000 -ac 1 output.wav
```

### 3. Common Parameters
```bash
# Navigate to build directory first
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd

# Use multiple threads for faster processing
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/audio.wav -t 8

# Translate to English (for non-English audio)
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/audio.wav -tr

# Process only part of the audio (first 30 seconds)
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/audio.wav -d 30000

# Better quality settings (if you have ggml-small.en.bin)
./bin/main -m ../../models/ggml-small.en.bin -f ../../audio-files/audio.wav -bo 5 -bs 5
```

## Quick Examples Overview

### 1. Command Line Interface (CLI)
**Purpose**: Batch transcription of audio files
```bash
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && ./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/audio.wav -otxt
```

### 2. Real-time Streaming
**Purpose**: Live transcription from microphone
```bash
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && ./bin/stream -m ../../models/ggml-base.en.bin -t 8
```

### 3. HTTP Server
**Purpose**: Transcription service via REST API
```bash
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && ./bin/server -m ../../models/ggml-base.en.bin -p 8081
```

### 4. Voice Commands
**Purpose**: Recognize spoken commands
```bash
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && ./bin/command -m ../../models/ggml-base.en.bin -t 8
```

## Model Selection Guide

| Model | Size | Speed | Quality | Use Case |
|-------|------|-------|---------|----------|
| tiny.en | 39 MB | Fastest | Basic | Real-time, IoT devices |
| base.en | 142 MB | Fast | Good | General real-time use |
| small.en | 466 MB | Medium | Better | Balanced quality/speed |
| medium.en | 1.5 GB | Slow | High | High-quality transcription |
| large-v3 | 3.1 GB | Slowest | Best | Maximum accuracy |

**Recommendations:**
- **Real-time applications**: tiny, base
- **General transcription**: base, small
- **High-quality transcription**: small, medium
- **Maximum accuracy**: medium, large

## Output Formats

### Text Formats
- **Plain text** (`-otxt`): Simple transcription
- **VTT** (`-ovtt`): Web Video Text Tracks
- **SRT** (`-osrt`): SubRip subtitle format
- **LRC** (`-olrc`): Lyric file format

### Structured Formats
- **JSON** (`-oj`): Structured data with timestamps
- **CSV** (`-ocsv`): Comma-separated values

### Example Output
```bash
# Navigate to build directory first
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd

# Generate multiple formats
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/audio.wav -otxt -osrt -oj

# Creates (in the build/debug directory):
# ../../audio-files/audio.wav.txt - Plain text
# ../../audio-files/audio.wav.srt - Subtitle file
# ../../audio-files/audio.wav.json - JSON with timestamps
```

## Performance Optimization

### Threading
```bash
# Navigate to build directory first
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd

# Use all CPU cores
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/audio.wav -t $(nproc)

# Or specify exact number
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/audio.wav -t 8
```

### Memory Usage
```bash
# Navigate to build directory first
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd

# Reduce audio context for lower memory usage
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/audio.wav -ac 512

# Limit maximum context tokens
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/audio.wav -mc 1000
```

## Troubleshooting

### Common Issues

**1. Model not found**
```bash
# Error: Could not load model from '../../models/ggml-base.en.bin'
# Solution: Check if model exists in your project
ls -la /home/alex/github-repos/wt-cv-stylus-copilot/models/
# If missing, you need to download the model to your project's models directory
```

**2. Audio format not supported**
```bash
# Error: failed to read WAV file
# Solution: Convert to supported format and place in audio-files directory
ffmpeg -i input.mp4 -ar 16000 -ac 1 /home/alex/github-repos/wt-cv-stylus-copilot/audio-files/output.wav
```

**3. Microphone not working**
```bash
# Error: SDL_Init failed
# Solution: Install SDL2 and rebuild with examples enabled
sudo apt-get install libsdl2-dev
cd /home/alex/github-repos/wt-cv-stylus-copilot
cmake -DCMAKE_BUILD_TYPE=Debug -DWHISPER_BUILD_EXAMPLES=ON -S . -B build/debug
cd build/debug && make -j$(nproc)
```

**4. Low performance**
- Use smaller model (tiny, base)
- Reduce thread count if CPU limited
- Use GPU acceleration if available

### Performance Benchmarking
```bash
# Navigate to build directory first
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd

# Test your system performance
./bin/bench -m ../../models/ggml-base.en.bin -t 8

# Compare different models (if you have them)
./bin/bench -m ../../models/ggml-tiny.en.bin -t 8
./bin/bench -m ../../models/ggml-small.en.bin -t 8
```

## Next Steps

1. **Project Integration**: Learn about [project-specific commands](project-integration.md)
2. **Real-time Usage**: Explore [streaming examples](realtime-examples.md)
3. **Web Integration**: Check out [web examples](web-examples.md)
4. **Advanced Features**: Review [advanced examples](advanced-examples.md)
5. **Performance**: Read the [performance guide](performance.md)

## Additional Resources

- **Project Integration**: See [project-integration.md](project-integration.md) for complete project-specific guide
- **Performance Data**: Check the [benchmarking results](performance.md)
- **API Reference**: See language-specific [bindings](language-bindings.md)
- **Your Main App**: Integration with `src/000-Server/Whisper/WhisperAi.cpp`
