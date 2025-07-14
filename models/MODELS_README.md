# Whisper Models Directory

This directory contains Whisper AI models for speech-to-text functionality.

## Quick Start

Run the download script to get the base English model:
```bash
./download_models.sh
```

## Available Models

| Model | Size | Speed | Accuracy | Use Case |
|-------|------|-------|----------|----------|
| ggml-tiny.bin | 39MB | Fastest | Basic | Quick transcription, real-time |
| ggml-base.en.bin | 74MB | Fast | Good | English-only, recommended |
| ggml-small.bin | 244MB | Medium | Better | Multi-language |
| ggml-medium.bin | 769MB | Slow | Great | High accuracy needed |
| ggml-large-v3.bin | 1550MB | Slowest | Best | Maximum accuracy |

## Download Manually

If the script doesn't work, download manually:

```bash
# Base English model (recommended)
wget https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.en.bin

# Other models
wget https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-tiny.bin
wget https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-small.bin
```

## Requirements

- ffmpeg (for audio conversion)
- Internet connection for downloads

## Usage

The VoiceRecorder component will automatically detect and use the first available model in this order:
1. ggml-base.en.bin
2. ggml-base.bin
3. ggml-small.en.bin
4. ggml-small.bin
5. ggml-tiny.en.bin
6. ggml-tiny.bin

## Supported Languages

When using multilingual models (non-.en versions), Whisper supports 99 languages including:
- English (en)
- Spanish (es)
- French (fr)
- German (de)
- Italian (it)
- Portuguese (pt)
- Russian (ru)
- Japanese (ja)
- Korean (ko)
- Chinese (zh)
- And many more...

Use "auto" for automatic language detection.