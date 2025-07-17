# Whisper.cpp Integration in WT-CV-Stylus Project

This document explains how to use whisper.cpp examples within this specific WT (C++) web application project.

## Project Structure Overview

```
wt-cv-stylus-copilot/
├── models/                          # Whisper models location
│   ├── ggml-base.en.bin
│   ├── ggml-tiny.en.bin
│   └── ...
├── build/
│   ├── debug/                       # Debug build directory (RUN FROM HERE)
│   │   ├── app                      # Main application
│   │   └── _deps/whisper-build/     # Whisper examples built here
│   └── release/                     # Release build directory (RUN FROM HERE)
│       ├── app                      # Main application  
│       └── _deps/whisper-build/     # Whisper examples built here
├── src/
│   └── 000-Server/Whisper/
│       └── WhisperAi.cpp            # Your whisper integration
└── docs/whispercpp/                 # This documentation
```

## ⚠️ CRITICAL EXECUTION RULES

**ALWAYS run whisper examples from the build directory using absolute paths:**

```bash
# Navigate to build directory with absolute path
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd

# Verify you're in the correct directory
pwd
# Should output: /home/alex/github-repos/wt-cv-stylus-copilot/build/debug
```

## Building the Project with Whisper Support

### Debug Build
```bash
cd /home/alex/github-repos/wt-cv-stylus-copilot && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/debug && cd build/debug && make -j$(nproc)
```

### Release Build  
```bash
cd /home/alex/github-repos/wt-cv-stylus-copilot && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/release && cd build/release && make -j$(nproc)
```

## Available Whisper Examples

After building, whisper.cpp examples are available in:
- `build/debug/bin/` (debug build)
- `build/release/bin/` (release build)

### List Available Examples
```bash
# From debug build directory
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && ls -la bin/

# Common examples you'll find:
# - main (CLI transcription)
# - stream (real-time microphone)
# - server (HTTP API)
# - bench (performance testing)
# - quantize (model compression)
```

### Building Examples (Required First Time)
```bash
# Build with examples enabled
cd /home/alex/github-repos/wt-cv-stylus-copilot && cmake -DCMAKE_BUILD_TYPE=Debug -DWHISPER_BUILD_EXAMPLES=ON -S . -B build/debug && cd build/debug && make -j$(nproc)
```

## ✅ Verified Working Example

The following command has been tested and works successfully:

```bash
# Navigate to build directory
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd

# Transcribe audio file (tested with real file)
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/audio_20250716_215632_411.wav

# Result: "Hello, my name is Alex, and I'm a programmer."
# Processing time: ~22 seconds for 4.2 seconds of audio
# Model: ggml-base.en.bin (147MB)
```

## Using Whisper Examples

### 1. Basic CLI Transcription

**⚠️ ALWAYS use absolute paths and run from build directory:**

```bash
# Navigate to build directory
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd

# Basic transcription
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/your-audio.wav

# With text output
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/your-audio.wav -otxt

# Multiple output formats
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/your-audio.wav -otxt -osrt -oj

# Example with real file (tested):
./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/audio_20250716_215632_411.wav
```

### 2. Real-time Microphone Transcription

```bash
# Navigate to build directory
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd

# Start real-time transcription (requires SDL2)
./bin/stream -m ../../models/ggml-base.en.bin -t 8

# With custom settings
./bin/stream -m ../../models/ggml-base.en.bin -t 4 --step 500 --length 5000
```

### 3. HTTP Server for Transcription API

```bash
# Navigate to build directory
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd

# Start whisper HTTP server
./bin/server -m ../../models/ggml-base.en.bin -p 8081

# Server will be available at http://localhost:8081
# Note: Your main app runs on port 9020, so use different port
```

### 4. Performance Benchmarking

```bash
# Navigate to build directory
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd

# Test performance with your models
./bin/bench -m ../../models/ggml-base.en.bin -t 8

# Compare different models
./bin/bench -m ../../models/ggml-tiny.en.bin -t 4
./bin/bench -m ../../models/ggml-small.en.bin -t 8
```

### 5. Model Quantization

```bash
# Navigate to build directory
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd

# Quantize a model to reduce size
./bin/quantize ../../models/ggml-base.en.bin ../../models/ggml-base.en-q8_0.bin q8_0
```

## Project-Specific Model Paths

Your models are located in `./models/` from the project root. When running from build directory, use:

```bash
# Model path patterns (from build/debug or build/release)
../../models/ggml-tiny.en.bin     # Fastest (39MB)
../../models/ggml-base.en.bin     # Balanced (142MB)  
../../models/ggml-small.en.bin    # Better quality (466MB)
../../models/ggml-medium.en.bin   # High quality (1.5GB)
../../models/ggml-large-v3.bin    # Best quality (3.1GB)
```

## Integration with Your Main Application

Your main app includes whisper integration in:
- `src/000-Server/Whisper/WhisperAi.cpp`
- `src/003-Components/VoiceRecorder.cpp`

### Running Your App with Whisper
```bash
# Run your main application (which includes whisper integration)
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd && make run

# Or manually:
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd && ./app --docroot ../../ -c ../../wt_config.xml --http-address 0.0.0.0 --http-port 9020
```

## Common Use Cases for Your Project

### 1. Test Whisper Performance
```bash
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd && ./bin/bench -m ../../models/ggml-base.en.bin -t 8
```

### 2. Transcribe Audio Files for Testing
```bash
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd && ./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/audio_20250716_215632_411.wav -otxt
```

### 3. Real-time Voice Testing
```bash
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd && ./bin/stream -m ../../models/ggml-base.en.bin -t 4
```

### 4. API Server for External Testing
```bash
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd && ./bin/server -m ../../models/ggml-base.en.bin -p 8081
```

## Troubleshooting

### Common Issues

**1. "Command not found" or "No such file"**
```bash
# Solution: Always verify you're in the build directory
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && pwd
# Should output: /home/alex/github-repos/wt-cv-stylus-copilot/build/debug

# Check if whisper binaries exist
ls -la bin/

# If no binaries found, rebuild with examples enabled:
cd /home/alex/github-repos/wt-cv-stylus-copilot && cmake -DCMAKE_BUILD_TYPE=Debug -DWHISPER_BUILD_EXAMPLES=ON -S . -B build/debug && cd build/debug && make -j$(nproc)
```

**2. "Model not found"**
```bash
# Solution: Check model path is correct from build directory
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && ls -la ../../models/
```

**3. "Library not found" errors**
```bash
# Solution: Use the make run target which sets up library paths
cd /home/alex/github-repos/wt-cv-stylus-copilot/build/debug && make run
```

**4. SDL2 errors for microphone examples**
```bash
# Install SDL2 development libraries
sudo apt-get install libsdl2-dev

# Rebuild project
cd /home/alex/github-repos/wt-cv-stylus-copilot && cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build/debug && cd build/debug && make -j$(nproc)
```

## Script Templates

### Testing Script
```bash
#!/bin/bash
# test-whisper.sh

PROJECT_ROOT="/home/alex/github-repos/wt-cv-stylus-copilot"
BUILD_DIR="$PROJECT_ROOT/build/debug"

echo "=== Whisper.cpp Testing ==="
echo "Project: $PROJECT_ROOT"
echo "Build: $BUILD_DIR"

# Navigate to build directory
cd "$BUILD_DIR" && pwd

echo "Available whisper binaries:"
ls -la bin/

echo "Available models:"
ls -la ../../models/

echo "Testing basic transcription..."
if [ -f "../../audio-files/audio_20250716_215632_411.wav" ]; then
    ./bin/main -m ../../models/ggml-base.en.bin -f ../../audio-files/audio_20250716_215632_411.wav
else
    echo "⚠️  No test audio file found at ../../audio-files/"
fi
```

### Benchmark Script
```bash
#!/bin/bash
# benchmark-whisper.sh

PROJECT_ROOT="/home/alex/github-repos/wt-cv-stylus-copilot"
BUILD_DIR="$PROJECT_ROOT/build/debug"

cd "$BUILD_DIR" && pwd

echo "=== Whisper Performance Benchmark ==="

for model in tiny.en base.en small.en; do
    if [ -f "../../models/ggml-${model}.bin" ]; then
        echo "Testing $model..."
        ./bin/bench -m "../../models/ggml-${model}.bin" -t 4
    else
        echo "⚠️  Model not found: ../../models/ggml-${model}.bin"
    fi
done
```

## Next Steps

1. **Download Models**: Ensure you have whisper models in `./models/`
2. **Test Integration**: Run the benchmark script to verify everything works
3. **Explore Examples**: Try different whisper examples for your use cases
4. **Main App Development**: Focus on your WT application with integrated whisper features

## Related Files in Your Project

- **Main App**: `src/main.cpp`
- **Whisper Integration**: `src/000-Server/Whisper/WhisperAi.cpp`
- **Voice Component**: `src/003-Components/VoiceRecorder.cpp`
- **Build Config**: `CMakeLists.txt`
- **Development Guide**: `docs/development_guide.md`
