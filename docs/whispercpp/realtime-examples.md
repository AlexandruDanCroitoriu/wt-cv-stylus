# Real-time Examples

This document covers examples that process audio in real-time, including streaming transcription, voice commands, and conversational AI.

## Stream - Real-time Microphone Transcription

Continuous transcription from microphone input with configurable processing windows.

### Prerequisites
```bash
# Install SDL2 for microphone support
sudo apt-get install libsdl2-dev  # Ubuntu/Debian
sudo dnf install SDL2-devel       # Fedora/RHEL  
brew install sdl2                 # macOS

# Build with SDL2 support
cmake .. -DWHISPER_SDL2=ON
cmake --build . --config Release
```

### Basic Usage
```bash
# Start real-time transcription
./build/bin/whisper-stream -m ./models/ggml-base.en.bin -t 8

# With custom processing window
./build/bin/whisper-stream -m ./models/ggml-base.en.bin -t 8 --step 500 --length 5000
```

### Parameters

#### Core Options
```bash
-m MODEL, --model MODEL          # Model file path
-t N,     --threads N            # Number of threads
-l LANG,  --language LANG        # Language ('en', 'auto', etc.)
-tr,      --translate            # Translate to English
```

#### Timing Control
```bash
--step N                         # Processing step in milliseconds (500)
--length N                       # Audio buffer length in milliseconds (5000)
-kc N,    --keep-context N       # Keep context from previous chunks (0)
```

#### Audio Processing
```bash
-ac N,    --audio-ctx N          # Audio context size (0 = all)
-vth N,   --vad-thold N          # VAD threshold (0.6)
-fth N,   --freq-thold N         # Frequency threshold (100.0)
```

#### Quality Settings
```bash
-mc N,    --max-context N        # Maximum context tokens
-wt N,    --word-thold N         # Word timestamp threshold (0.01)
-et N,    --entropy-thold N      # Entropy threshold (2.40)
-lpt N,   --logprob-thold N      # Log probability threshold (-1.00)
```

#### Output Options
```bash
-np,      --no-prints            # Reduce output verbosity
-pc,      --print-colors         # Colored output
-ps,      --print-special        # Print special tokens
```

### Operating Modes

#### 1. Continuous Mode (Default)
```bash
# Process audio every 500ms with 5-second buffer
./build/bin/whisper-stream -m ./models/ggml-base.en.bin --step 500 --length 5000
```

**How it works:**
- Captures audio continuously
- Processes in overlapping windows
- Outputs partial transcriptions
- Good for live captions

#### 2. Sliding Window Mode (VAD)
```bash
# Enable VAD-based processing
./build/bin/whisper-stream -m ./models/ggml-base.en.bin --step 0 --length 30000 -vth 0.6
```

**How it works:**
- Uses Voice Activity Detection
- Transcribes only when speech detected
- Waits for silence before processing
- Better for command detection

### Configuration Examples

#### High-Quality Real-time
```bash
./build/bin/whisper-stream \
  -m ./models/ggml-small.en.bin \
  -t 8 \
  --step 1000 \
  --length 8000 \
  -wt 0.005 \
  -vth 0.7
```

#### Fast/Responsive
```bash
./build/bin/whisper-stream \
  -m ./models/ggml-tiny.en.bin \
  -t 4 \
  --step 300 \
  --length 3000 \
  -ac 512
```

#### Multi-language
```bash
./build/bin/whisper-stream \
  -m ./models/ggml-base.bin \
  -l auto \
  -tr \
  --step 500 \
  --length 5000
```

## Command - Voice Command Recognition

Real-time voice command recognition with guided mode support.

### Basic Usage
```bash
# Start voice command recognition
./build/bin/whisper-command -m ./models/ggml-small.en.bin -t 8

# With command file (guided mode)
./build/bin/whisper-command -m ./models/ggml-base.en.bin -cmd ./commands.txt
```

### Parameters
```bash
-m MODEL, --model MODEL          # Model file path
-t N,     --threads N            # Number of threads
-c N,     --capture N            # Capture device ID (0)
-cmd FILE,--commands FILE        # Command list file (guided mode)
-ac N,    --audio-ctx N          # Audio context size
-vth N,   --vad-thold N          # VAD threshold
-fth N,   --freq-thold N         # Frequency threshold
```

### Guided Mode

Create a commands file with allowed phrases:

**commands.txt:**
```
turn on the lights
turn off the lights
increase volume
decrease volume
play music
stop music
what time is it
what's the weather
open browser
close application
```

**Usage:**
```bash
./build/bin/whisper-command \
  -m ./models/ggml-base.en.bin \
  -cmd ./commands.txt \
  -t 8 \
  -ac 128
```

### Implementation Example

**Voice-controlled system script:**
```bash
#!/bin/bash
# voice-control.sh

FIFO="/tmp/voice_commands"
mkfifo "$FIFO"

# Start command recognition in background
./build/bin/whisper-command \
  -m ./models/ggml-base.en.bin \
  -cmd ./commands.txt \
  -t 4 \
  > "$FIFO" &

WHISPER_PID=$!

# Process commands
while read -r command < "$FIFO"; do
  case "$command" in
    *"lights on"*)
      echo "Turning lights on"
      # Add your light control command
      ;;
    *"lights off"*)
      echo "Turning lights off" 
      # Add your light control command
      ;;
    *"volume up"*)
      amixer set Master 5%+
      ;;
    *"volume down"*)
      amixer set Master 5%-
      ;;
    *"quit"* | *"exit"*)
      break
      ;;
  esac
done

# Cleanup
kill $WHISPER_PID
rm "$FIFO"
```

### Platform-Specific Optimizations

#### Raspberry Pi
```bash
# Optimized for Raspberry Pi
./build/bin/whisper-command \
  -m ./models/ggml-tiny.en.bin \
  -ac 768 \
  -t 3 \
  -c 0 \
  -cmd ./commands.txt
```

#### Low-power devices
```bash
# Minimal resource usage
./build/bin/whisper-command \
  -m ./models/ggml-tiny.en.bin \
  -ac 128 \
  -t 2 \
  -vth 0.8
```

## Talk-LLaMA - Conversational AI

Voice-activated conversational AI combining Whisper for speech recognition with LLaMA for response generation.

### Prerequisites
```bash
# Install SDL2
sudo apt-get install libsdl2-dev

# Download LLaMA model (example)
# Follow instructions at https://github.com/ggerganov/llama.cpp

# Build with SDL2 support
cmake .. -DWHISPER_SDL2=ON
cmake --build . --config Release
```

### Basic Usage
```bash
./build/bin/whisper-talk-llama \
  -mw ./models/ggml-small.en.bin \
  -ml ../llama.cpp/models/llama-7b/ggml-model-q4_0.gguf \
  -p "Assistant" \
  -t 8
```

### Parameters

#### Model Configuration
```bash
-mw MODEL, --model-whisper MODEL # Whisper model path
-ml MODEL, --model-llama MODEL   # LLaMA model path
-p NAME,   --person NAME         # Assistant persona name
```

#### Performance
```bash
-t N,      --threads N           # Number of threads
-ngl N,    --n-gpu-layers N      # GPU layers (if CUDA enabled)
-c N,      --ctx-size N          # LLaMA context size
```

#### Audio Settings
```bash
-ac N,     --audio-ctx N         # Audio context size
-vth N,    --vad-thold N         # VAD threshold
-fth N,    --freq-thold N        # Frequency threshold
```

#### Session Management
```bash
--session FILE                   # Session file for persistence
```

### Session Persistence

Enable conversation memory across sessions:

```bash
# Start with session file
./build/bin/whisper-talk-llama \
  -mw ./models/ggml-small.en.bin \
  -ml ../llama.cpp/models/llama-7b/ggml-model-q4_0.gguf \
  -p "Assistant" \
  --session ./my-conversation.bin \
  -t 8

# Resume previous conversation
./build/bin/whisper-talk-llama \
  -mw ./models/ggml-small.en.bin \
  -ml ../llama.cpp/models/llama-7b/ggml-model-q4_0.gguf \
  -p "Assistant" \
  --session ./my-conversation.bin \
  -t 8
```

### Usage Examples

#### Personal Assistant
```bash
./build/bin/whisper-talk-llama \
  -mw ./models/ggml-base.en.bin \
  -ml ./models/llama-7b-chat-q4_0.gguf \
  -p "AI Assistant" \
  --session ./assistant-session.bin \
  -t 8 \
  -c 2048
```

#### Educational Tutor
```bash
./build/bin/whisper-talk-llama \
  -mw ./models/ggml-small.en.bin \
  -ml ./models/llama-13b-instruct-q4_0.gguf \
  -p "Tutor" \
  --session ./tutoring-session.bin \
  -t 8 \
  -c 4096
```

#### Creative Companion
```bash
./build/bin/whisper-talk-llama \
  -mw ./models/ggml-base.en.bin \
  -ml ./models/llama-7b-creative-q4_0.gguf \
  -p "Creative Writer" \
  -t 6 \
  -c 2048
```

## Performance Optimization

### Model Selection for Real-time

#### Ultra-fast (IoT/Embedded)
- **Whisper**: tiny.en (39MB)
- **Latency**: ~100-200ms
- **Quality**: Basic
- **Use case**: Simple commands, IoT devices

#### Balanced (Desktop/Mobile)
- **Whisper**: base.en (142MB)  
- **Latency**: ~200-500ms
- **Quality**: Good
- **Use case**: General applications

#### High-quality (Server/Workstation)
- **Whisper**: small.en (466MB)
- **Latency**: ~500-1000ms
- **Quality**: High
- **Use case**: Professional applications

### Threading Optimization

```bash
# Find optimal thread count for your system
for t in 1 2 4 6 8; do
  echo "Testing $t threads:"
  # Use whisper-bench to test
  ./build/bin/whisper-bench -m ./models/ggml-base.en.bin -t $t
done
```

### Memory Management

```bash
# Reduce memory usage for constrained systems
./build/bin/whisper-stream \
  -m ./models/ggml-tiny.en.bin \
  -ac 512 \
  -mc 1000 \
  --length 3000 \
  -t 2
```

### Latency Optimization

```bash
# Minimize latency
./build/bin/whisper-stream \
  -m ./models/ggml-tiny.en.bin \
  --step 200 \
  --length 2000 \
  -ac 256 \
  -t 4
```

## Integration Patterns

### Systemd Service
```ini
# /etc/systemd/system/voice-assistant.service
[Unit]
Description=Voice Assistant
After=sound.target

[Service]
Type=simple
User=whisper
WorkingDirectory=/opt/whisper
ExecStart=/opt/whisper/build/bin/whisper-command -m /opt/whisper/models/ggml-base.en.bin -cmd /opt/whisper/commands.txt
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

### Docker Container
```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libsdl2-2.0-0 \
    && rm -rf /var/lib/apt/lists/*

COPY build/bin/whisper-stream /usr/local/bin/
COPY models/ /models/

# Enable audio device access
VOLUME ["/dev/snd"]

CMD ["whisper-stream", "-m", "/models/ggml-base.en.bin", "-t", "4"]
```

### Python Integration
```python
import subprocess
import threading
import queue

class WhisperStream:
    def __init__(self, model_path):
        self.model_path = model_path
        self.process = None
        self.output_queue = queue.Queue()
        
    def start(self):
        cmd = [
            './build/bin/whisper-stream',
            '-m', self.model_path,
            '-t', '4',
            '--step', '500',
            '--length', '5000'
        ]
        
        self.process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        # Start reading output in separate thread
        threading.Thread(
            target=self._read_output,
            daemon=True
        ).start()
    
    def _read_output(self):
        for line in self.process.stdout:
            self.output_queue.put(line.strip())
    
    def get_transcription(self, timeout=1):
        try:
            return self.output_queue.get(timeout=timeout)
        except queue.Empty:
            return None
    
    def stop(self):
        if self.process:
            self.process.terminate()

# Usage
stream = WhisperStream('./models/ggml-base.en.bin')
stream.start()

while True:
    text = stream.get_transcription()
    if text:
        print(f"Transcribed: {text}")
```

## Troubleshooting

### Audio Issues
```bash
# Test microphone
arecord -l                        # List audio devices
arecord -D hw:0,0 -d 5 test.wav   # Test recording

# Check SDL2 audio
./build/bin/whisper-stream -m ./models/ggml-tiny.en.bin -c 0  # Device 0
./build/bin/whisper-stream -m ./models/ggml-tiny.en.bin -c 1  # Device 1
```

### Performance Issues
```bash
# Monitor CPU usage
top -p $(pgrep whisper-stream)

# Monitor memory usage  
valgrind --tool=massif ./build/bin/whisper-stream -m ./models/ggml-tiny.en.bin
```

### Quality Issues
```bash
# Improve VAD threshold
./build/bin/whisper-stream -m ./models/ggml-base.en.bin -vth 0.7

# Use larger model
./build/bin/whisper-stream -m ./models/ggml-small.en.bin

# Adjust processing window
./build/bin/whisper-stream -m ./models/ggml-base.en.bin --length 8000
```

## Next Steps

- **Web Integration**: Learn about [WebAssembly examples](web-examples.md)
- **Advanced Features**: Explore [advanced examples](advanced-examples.md)
- **Performance Tuning**: Read [performance optimization](performance.md)
- **Language Bindings**: Check [Python and mobile examples](language-bindings.md)
