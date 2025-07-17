# Core Examples

This document covers the main command-line tools that form the foundation of whisper.cpp usage. These examples provide batch processing capabilities and are the most commonly used tools.

## CLI - Command Line Interface

The primary tool for transcribing audio files with extensive configuration options.

### Basic Usage
```bash
./build/bin/whisper-cli -m ./models/ggml-base.en.bin -f audio.wav
```

### Key Features
- Batch processing of audio files
- Multiple output formats (txt, srt, vtt, json, csv)
- Language detection and translation
- Timestamp generation
- Word-level timestamps
- Speaker diarization support

### Complete Parameter Reference

#### Model and Input
```bash
-m MODEL, --model MODEL           # Path to model file (required)
-f FILE,  --file FILE            # Audio file to process
```

#### Performance Options
```bash
-t N,     --threads N            # Number of threads (default: 4)
-p N,     --processors N         # Number of processors (default: 1)
```

#### Audio Processing
```bash
-ot N,    --offset-t N           # Time offset in milliseconds
-on N,    --offset-n N           # Segment index offset
-d N,     --duration N           # Duration to process in milliseconds
-ac N,    --audio-ctx N          # Audio context size (0 = all)
```

#### Quality and Decoding
```bash
-mc N,    --max-context N        # Maximum text context tokens (-1 = model default)
-ml N,    --max-len N            # Maximum segment length in characters
-sow,     --split-on-word        # Split on word rather than token
-bo N,    --best-of N            # Number of best candidates to keep (default: 5)
-bs N,    --beam-size N          # Beam size for beam search (default: 5)
```

#### Thresholds and Detection
```bash
-wt N,    --word-thold N         # Word timestamp probability threshold (0.01)
-et N,    --entropy-thold N      # Entropy threshold for decoder fail (2.40)
-lpt N,   --logprob-thold N      # Log probability threshold (-1.00)
-nth N,   --no-speech-thold N    # No speech threshold (0.60)
```

#### Language and Translation
```bash
-l LANG,  --language LANG        # Spoken language ('auto' for auto-detect)
-tr,      --translate            # Translate to English
```

#### Output Options
```bash
-otxt,    --output-txt           # Output plain text file
-ovtt,    --output-vtt           # Output WebVTT subtitle file
-osrt,    --output-srt           # Output SubRip subtitle file
-olrc,    --output-lrc           # Output LRC lyric file
-owts,    --output-words         # Output word-level timestamps
-ocsv,    --output-csv           # Output CSV file
-oj,      --output-json          # Output JSON file
-ojf,     --output-json-full     # Full JSON with additional info
-of FILE, --output-file FILE     # Custom output file path
```

#### Display Options
```bash
-np,      --no-prints            # Only output results, no progress
-ps,      --print-special        # Print special tokens
-pc,      --print-colors         # Print with colors
-pp,      --print-progress       # Print progress information
-nt,      --no-timestamps        # Don't print timestamps
```

#### Advanced Features
```bash
-di,      --diarize              # Stereo audio diarization
-tdrz,    --tinydiarize          # Enable tinydiarize (requires tdrz model)
-nf,      --no-fallback          # No temperature fallback
-debug,   --debug-mode           # Enable debug output
```

### Common Use Cases

#### 1. Basic Transcription
```bash
# Simple transcription
./build/bin/whisper-cli -m ./models/ggml-base.en.bin -f meeting.wav

# With text output file
./build/bin/whisper-cli -m ./models/ggml-base.en.bin -f meeting.wav -otxt
```

#### 2. Subtitle Generation
```bash
# Generate SRT subtitles
./build/bin/whisper-cli -m ./models/ggml-base.en.bin -f movie.wav -osrt

# Generate VTT for web
./build/bin/whisper-cli -m ./models/ggml-base.en.bin -f video.wav -ovtt

# Both formats
./build/bin/whisper-cli -m ./models/ggml-base.en.bin -f content.wav -osrt -ovtt
```

#### 3. High-Quality Transcription
```bash
# Use larger model with optimized settings
./build/bin/whisper-cli \
  -m ./models/ggml-small.en.bin \
  -f interview.wav \
  -bo 10 \
  -bs 10 \
  -wt 0.02 \
  -otxt -oj
```

#### 4. Batch Processing
```bash
# Process multiple files
./build/bin/whisper-cli -m ./models/ggml-base.en.bin \
  file1.wav file2.wav file3.wav -otxt

# Using shell scripting
for file in *.wav; do
  ./build/bin/whisper-cli -m ./models/ggml-base.en.bin -f "$file" -otxt
done
```

#### 5. Non-English Content
```bash
# Auto-detect language
./build/bin/whisper-cli -m ./models/ggml-base.bin -f spanish.wav -l auto

# Specific language
./build/bin/whisper-cli -m ./models/ggml-base.bin -f french.wav -l fr

# Translate to English
./build/bin/whisper-cli -m ./models/ggml-base.bin -f german.wav -tr
```

#### 6. Long Audio Processing
```bash
# Process first 5 minutes only
./build/bin/whisper-cli -m ./models/ggml-base.en.bin -f long-audio.wav -d 300000

# Start from 1 minute mark
./build/bin/whisper-cli -m ./models/ggml-base.en.bin -f long-audio.wav -ot 60000

# Process specific segment (1-6 minutes)
./build/bin/whisper-cli -m ./models/ggml-base.en.bin -f long-audio.wav -ot 60000 -d 300000
```

### Output Format Examples

#### Plain Text (.txt)
```
And so my fellow Americans, ask not what your country can do for you, ask what you can do for your country.
```

#### JSON Output (-oj)
```json
{
  "systeminfo": "...",
  "model": {
    "type": "base.en",
    "multilingual": false,
    "vocab": 51864,
    "audio": {
      "ctx": 1500,
      "state": 512,
      "head": 8,
      "layer": 6
    }
  },
  "params": {
    "model": "./models/ggml-base.en.bin",
    "language": "en",
    "translate": false
  },
  "result": {
    "language": "en"
  },
  "transcription": [
    {
      "timestamps": {
        "from": "00:00:00,000",
        "to": "00:00:08,000"
      },
      "offsets": {
        "from": 0,
        "to": 8000
      },
      "text": " And so my fellow Americans, ask not what your country can do for you, ask what you can do for your country."
    }
  ]
}
```

#### SubRip (.srt)
```
1
00:00:00,000 --> 00:00:08,000
And so my fellow Americans, ask not what your country can do for you, ask what you can do for your country.
```

#### WebVTT (.vtt)
```
WEBVTT

00:00.000 --> 00:08.000
And so my fellow Americans, ask not what your country can do for you, ask what you can do for your country.
```

## Bench - Performance Benchmarking

Tool for measuring whisper.cpp performance on your system.

### Basic Usage
```bash
./build/bin/whisper-bench -m ./models/ggml-base.en.bin -t 4
```

### Parameters
```bash
-m MODEL, --model MODEL          # Model file to benchmark
-t N,     --threads N            # Number of threads to use
-ngl N,   --n-gpu-layers N       # Number of GPU layers (if CUDA enabled)
```

### Example Output
```
whisper_model_load: loading model from './models/ggml-base.en.bin'
whisper_model_load: n_vocab       = 51864
whisper_model_load: n_audio_ctx   = 1500
whisper_model_load: n_audio_state = 512
whisper_model_load: n_audio_head  = 8
whisper_model_load: n_audio_layer = 6
whisper_model_load: n_text_ctx    = 448
whisper_model_load: n_text_state  = 512
whisper_model_load: n_text_head   = 8
whisper_model_load: n_text_layer  = 6
whisper_model_load: n_mels        = 80
whisper_model_load: f16           = 1
whisper_model_load: type          = 2
whisper_model_load: mem_required  =  506.00 MB
whisper_model_load: adding 1607 extra tokens
whisper_model_load: model size    =  140.54 MB

system_info: n_threads = 4 / 8 | AVX = 1 | AVX2 = 1 | AVX512 = 0 | FMA = 1 | NEON = 0 | ARM_FMA = 0 | F16C = 1 | FP16_VA = 0 | WASM_SIMD = 0 | BLAS = 0 | SSE3 = 1 | VSX = 0 | 

whisper_print_timings:     load time =   152.32 ms
whisper_print_timings:     mel time =    45.62 ms
whisper_print_timings:  sample time =     0.00 ms /     1 runs (    0.00 ms per run)
whisper_print_timings:  encode time =   463.85 ms /     1 runs (  463.85 ms per run)
whisper_print_timings:  decode time =     0.00 ms /     1 runs (    0.00 ms per run)
whisper_print_timings:   total time =   661.79 ms
```

### Benchmarking Different Models
```bash
# Compare model performance
./build/bin/whisper-bench -m ./models/ggml-tiny.en.bin -t 4
./build/bin/whisper-bench -m ./models/ggml-base.en.bin -t 4
./build/bin/whisper-bench -m ./models/ggml-small.en.bin -t 4

# Test thread scaling
for threads in 1 2 4 8; do
  echo "Testing with $threads threads:"
  ./build/bin/whisper-bench -m ./models/ggml-base.en.bin -t $threads
done
```

## Performance Tips

### Model Selection
- **tiny/base**: Real-time applications, resource-constrained systems
- **small**: Good balance of speed and quality
- **medium/large**: Maximum quality, offline processing

### Threading
```bash
# Find optimal thread count
for t in 1 2 4 8 16; do
  echo "Threads: $t"
  time ./build/bin/whisper-cli -m ./models/ggml-base.en.bin -f test.wav -t $t -np
done
```

### Memory Optimization
```bash
# Reduce memory usage for large files
./build/bin/whisper-cli -m ./models/ggml-base.en.bin -f large-file.wav -ac 512 -mc 1000
```

### Quality vs Speed Trade-offs
```bash
# Fast processing (lower quality)
./build/bin/whisper-cli -m ./models/ggml-tiny.en.bin -f audio.wav -bo 1 -bs 1

# High quality (slower)
./build/bin/whisper-cli -m ./models/ggml-small.en.bin -f audio.wav -bo 10 -bs 10 -wt 0.005
```

## Integration Examples

### Shell Script Integration
```bash
#!/bin/bash
# transcribe-folder.sh
MODEL="./models/ggml-base.en.bin"
INPUT_DIR="./audio-files"
OUTPUT_DIR="./transcriptions"

mkdir -p "$OUTPUT_DIR"

for file in "$INPUT_DIR"/*.wav; do
  filename=$(basename "$file" .wav)
  echo "Processing: $filename"
  
  ./build/bin/whisper-cli \
    -m "$MODEL" \
    -f "$file" \
    -of "$OUTPUT_DIR/$filename" \
    -otxt -osrt -oj \
    -t 8 \
    -np
    
  echo "Completed: $filename"
done
```

### Makefile Integration
```makefile
# Makefile
WHISPER_CLI = ./build/bin/whisper-cli
MODEL = ./models/ggml-base.en.bin
THREADS = 8

%.txt: %.wav
	$(WHISPER_CLI) -m $(MODEL) -f $< -of $(basename $@) -otxt -t $(THREADS)

%.srt: %.wav
	$(WHISPER_CLI) -m $(MODEL) -f $< -of $(basename $@) -osrt -t $(THREADS)

transcribe-all: $(patsubst %.wav,%.txt,$(wildcard *.wav))
```

## Error Handling

### Common Issues and Solutions

**Model loading errors:**
```bash
# Check model file exists and has correct permissions
ls -la ./models/ggml-base.en.bin
# Re-download if corrupted
bash ./models/download-ggml-model.sh base.en
```

**Audio format errors:**
```bash
# Convert unsupported formats
ffmpeg -i input.mp4 -ar 16000 -ac 1 output.wav
ffmpeg -i input.m4a -ar 16000 -ac 1 output.wav
```

**Out of memory errors:**
```bash
# Use smaller model or reduce context
./build/bin/whisper-cli -m ./models/ggml-tiny.en.bin -f large-file.wav -ac 512
```

## Next Steps

- **Real-time Processing**: Learn about [streaming examples](realtime-examples.md)
- **Web Integration**: Explore [web examples](web-examples.md)  
- **Advanced Features**: Check [advanced examples](advanced-examples.md)
- **Performance Tuning**: Read [performance optimization](performance.md)
