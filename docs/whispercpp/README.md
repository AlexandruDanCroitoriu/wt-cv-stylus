# Whisper.cpp Examples Documentation

This directory contains comprehensive documentation for all whisper.cpp examples available in the project. Whisper.cpp is a port of OpenAI's Whisper automatic speech recognition (ASR) model written in C/C++.

## Table of Contents

- [Getting Started](getting-started.md) - Basic setup and usage
- [Core Examples](core-examples.md) - Main command-line tools
- [Real-time Examples](realtime-examples.md) - Streaming and live transcription
- [Web Examples](web-examples.md) - WebAssembly and browser-based tools
- [Language Bindings](language-bindings.md) - Python, Android, iOS examples
- [Advanced Examples](advanced-examples.md) - Specialized use cases
- [Performance & Benchmarking](performance.md) - Optimization and testing tools
- [Integration Guide](integration-guide.md) - How to use whisper.cpp in your projects

## Quick Overview

### Core Examples
- **CLI** - Main command-line transcription tool with extensive options
- **Server** - HTTP server for transcription via REST API
- **Bench** - Performance benchmarking tool

### Real-time Examples
- **Stream** - Real-time microphone transcription
- **Command** - Voice command recognition
- **Talk-LLaMA** - Conversational AI with speech input/output

### Web Examples
- **Whisper.wasm** - Browser-based transcription
- **Command.wasm** - Web voice commands
- **Stream.wasm** - Real-time web transcription

### Specialized Examples
- **Quantize** - Model compression tool
- **VAD Speech Segments** - Voice activity detection
- **Python bindings** - Python interface

## Prerequisites

Before using any examples, ensure you have:

1. **Built whisper.cpp**: Follow the main build instructions
2. **Downloaded models**: Use `./download_models.sh` or manual download
3. **Required dependencies**: SDL2 for microphone examples, etc.

## Model Files

All examples require whisper model files in GGML format:
- `ggml-tiny.en.bin` - Fastest, lowest quality (39 MB)
- `ggml-base.en.bin` - Good balance (142 MB)
- `ggml-small.en.bin` - Better quality (466 MB)
- `ggml-medium.en.bin` - High quality (1.5 GB)
- `ggml-large-v3.bin` - Best quality (3.1 GB)

Models are located in the `models/` directory after download.

## Common Use Cases

### Basic File Transcription
```bash
./build/bin/whisper-cli -m models/ggml-base.en.bin -f audio.wav
```

### Real-time Microphone
```bash
./build/bin/whisper-stream -m models/ggml-base.en.bin -t 8
```

### HTTP Server
```bash
./build/bin/whisper-server -m models/ggml-base.en.bin -p 8080
```

### Voice Commands
```bash
./build/bin/whisper-command -m models/ggml-small.en.bin -t 8
```

## Example Categories

### By Complexity
- **Beginner**: CLI, Bench, Python
- **Intermediate**: Stream, Server, Command
- **Advanced**: Talk-LLaMA, WebAssembly, Mobile

### By Use Case
- **Batch Processing**: CLI, Python, Quantize
- **Real-time**: Stream, Command, Talk-LLaMA
- **Web Integration**: Server, WebAssembly examples
- **Mobile**: Android, iOS, Swift examples
- **Development**: Bench, VAD, Integration examples

## Performance Considerations

- **Model size vs. speed**: Smaller models are faster but less accurate
- **Threading**: Use `-t` parameter to set thread count
- **Memory**: Larger models require more RAM
- **Real-time**: Base/small models recommended for live transcription

## Next Steps

1. Start with [Getting Started](getting-started.md) for basic setup
2. Explore [Core Examples](core-examples.md) for main tools
3. Check [Integration Guide](integration-guide.md) for using in your projects
4. Review specific example documentation for detailed usage

## Support

For issues and questions:
- Check individual example documentation
- Review the main whisper.cpp repository
- Consult the performance benchmarking results
