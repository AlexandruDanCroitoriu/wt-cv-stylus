# Advanced Examples

This document covers specialized whisper.cpp examples for advanced use cases including model quantization, voice activity detection, and integration scenarios.

## Quantize - Model Compression

Tool for reducing model size through integer quantization while maintaining acceptable quality.

### Overview

Model quantization converts 16-bit floating point weights to 8-bit integers, significantly reducing:
- Model file size (typically 50% reduction)
- Memory usage during inference
- Loading time

### Basic Usage

```bash
# Quantize a model
./build/bin/whisper-quantize ./models/ggml-base.en.bin ./models/ggml-base.en-q8_0.bin q8_0

# Available quantization types:
# q4_0 - 4-bit quantization (highest compression, lowest quality)
# q4_1 - 4-bit quantization (improved quality)
# q5_0 - 5-bit quantization (balanced)
# q5_1 - 5-bit quantization (improved quality)
# q8_0 - 8-bit quantization (lower compression, higher quality)
```

### Quantization Types Comparison

| Type | Bits | Compression | Quality | Use Case |
|------|------|-------------|---------|----------|
| q4_0 | 4 | ~75% | Lower | Mobile/IoT devices |
| q4_1 | 4 | ~75% | Better | Resource-constrained systems |
| q5_0 | 5 | ~65% | Good | Balanced mobile apps |
| q5_1 | 5 | ~65% | Better | Desktop applications |
| q8_0 | 8 | ~50% | High | Production systems |

### Batch Quantization Script

```bash
#!/bin/bash
# quantize-models.sh

MODELS_DIR="./models"
QUANTIZE_BIN="./build/bin/whisper-quantize"

# Models to quantize
MODELS=("tiny.en" "base.en" "small.en" "medium.en")

# Quantization types
QUANT_TYPES=("q4_0" "q5_0" "q8_0")

for model in "${MODELS[@]}"; do
    for qtype in "${QUANT_TYPES[@]}"; do
        input_file="${MODELS_DIR}/ggml-${model}.bin"
        output_file="${MODELS_DIR}/ggml-${model}-${qtype}.bin"
        
        if [ -f "$input_file" ]; then
            echo "Quantizing $model with $qtype..."
            
            start_time=$(date +%s)
            $QUANTIZE_BIN "$input_file" "$output_file" "$qtype"
            end_time=$(date +%s)
            
            if [ -f "$output_file" ]; then
                original_size=$(stat -f%z "$input_file" 2>/dev/null || stat -c%s "$input_file")
                quantized_size=$(stat -f%z "$output_file" 2>/dev/null || stat -c%s "$output_file")
                compression_ratio=$(echo "scale=2; $quantized_size * 100 / $original_size" | bc)
                duration=$((end_time - start_time))
                
                echo "✓ $model ($qtype): ${compression_ratio}% of original size, took ${duration}s"
            else
                echo "✗ Failed to quantize $model with $qtype"
            fi
        else
            echo "⚠ Model $input_file not found, skipping..."
        fi
    done
done
```

### Quality Assessment

```bash
#!/bin/bash
# test-quantization-quality.sh

TEST_AUDIO="test-sample.wav"
MODELS_DIR="./models"

# Test different quantization levels
echo "Testing quantization quality..."
echo "Model,Type,Size(MB),Time(s),WER(%)"

for model in tiny.en base.en small.en; do
    for qtype in "" q4_0 q5_0 q8_0; do
        if [ -z "$qtype" ]; then
            model_file="${MODELS_DIR}/ggml-${model}.bin"
            model_name="${model}"
        else
            model_file="${MODELS_DIR}/ggml-${model}-${qtype}.bin"
            model_name="${model}-${qtype}"
        fi
        
        if [ -f "$model_file" ]; then
            size=$(stat -f%z "$model_file" 2>/dev/null || stat -c%s "$model_file")
            size_mb=$(echo "scale=1; $size / 1024 / 1024" | bc)
            
            start_time=$(date +%s.%N)
            result=$(./build/bin/whisper-cli -m "$model_file" -f "$TEST_AUDIO" -np)
            end_time=$(date +%s.%N)
            
            duration=$(echo "$end_time - $start_time" | bc)
            
            # Calculate WER (Word Error Rate) if reference available
            # wer=$(calculate_wer "$result" "$reference_text")
            
            echo "$model_name,,${size_mb},${duration},N/A"
        fi
    done
done
```

## VAD Speech Segments - Voice Activity Detection

Advanced voice activity detection for segmenting audio into speech/non-speech regions.

### Features

- Automatic speech segment detection
- Configurable sensitivity thresholds  
- Multiple VAD model support
- Integration with whisper transcription

### Building

```bash
# Build VAD example
cmake -S . -B build
cmake --build build -j8 --target vad-speech-segments
```

### Basic Usage

```bash
# Detect speech segments
./build/bin/vad-speech-segments \
    -vad-model models/for-tests-silero-v5.1.2-ggml.bin \
    --file samples/jfk.wav \
    --no-prints

# Output:
# Detected 5 speech segments:
# Speech segment 0: start = 0.29, end = 2.21
# Speech segment 1: start = 3.30, end = 3.77
# Speech segment 2: start = 4.00, end = 4.35
```

### Parameters

```bash
-vad-model MODEL     # VAD model file path
--file AUDIO         # Input audio file
--no-prints          # Reduce output verbosity
--output-dir DIR     # Output directory for segments
--min-length MS      # Minimum segment length (milliseconds)
--max-length MS      # Maximum segment length (milliseconds)
--threshold FLOAT    # VAD sensitivity threshold (0.0-1.0)
```

### Advanced Usage

#### Export Speech Segments

```bash
# Export detected segments as separate files
./build/bin/vad-speech-segments \
    -vad-model models/silero-vad.bin \
    --file long-interview.wav \
    --output-dir ./segments/ \
    --min-length 1000 \
    --max-length 30000 \
    --threshold 0.7

# Creates:
# segments/segment_000.wav (0.29s - 2.21s)
# segments/segment_001.wav (3.30s - 3.77s)
# segments/segment_002.wav (4.00s - 4.35s)
```

#### Batch Processing with VAD

```bash
#!/bin/bash
# vad-batch-process.sh

VAD_MODEL="models/silero-vad.bin"
WHISPER_MODEL="models/ggml-base.en.bin"
INPUT_DIR="./audio-files"
OUTPUT_DIR="./transcriptions"

mkdir -p "$OUTPUT_DIR"

for audio_file in "$INPUT_DIR"/*.wav; do
    filename=$(basename "$audio_file" .wav)
    segments_dir="$OUTPUT_DIR/${filename}_segments"
    
    echo "Processing: $filename"
    
    # 1. Detect speech segments
    ./build/bin/vad-speech-segments \
        -vad-model "$VAD_MODEL" \
        --file "$audio_file" \
        --output-dir "$segments_dir" \
        --min-length 2000 \
        --max-length 15000 \
        --no-prints
    
    # 2. Transcribe each segment
    transcription_file="$OUTPUT_DIR/${filename}_transcription.txt"
    > "$transcription_file"  # Clear file
    
    for segment in "$segments_dir"/*.wav; do
        if [ -f "$segment" ]; then
            segment_name=$(basename "$segment" .wav)
            
            # Get timing from filename (if encoded)
            # segment_002_1.23_4.56.wav -> start=1.23, end=4.56
            
            echo "Transcribing segment: $segment_name"
            
            result=$(./build/bin/whisper-cli \
                -m "$WHISPER_MODEL" \
                -f "$segment" \
                -np)
            
            echo "[$segment_name] $result" >> "$transcription_file"
        fi
    done
    
    echo "Completed: $filename"
done
```

### Integration with Real-time Processing

```python
# vad_realtime.py
import subprocess
import numpy as np
from collections import deque
import threading
import queue

class VADProcessor:
    def __init__(self, vad_model_path, whisper_model_path):
        self.vad_model = vad_model_path
        self.whisper_model = whisper_model_path
        self.audio_buffer = deque(maxlen=16000*10)  # 10 seconds at 16kHz
        self.speech_queue = queue.Queue()
        self.is_processing = False
        
    def process_audio_chunk(self, audio_data):
        """Process incoming audio chunk"""
        self.audio_buffer.extend(audio_data)
        
        if len(self.audio_buffer) >= 16000:  # 1 second minimum
            # Check for speech activity
            speech_detected = self.detect_speech(list(self.audio_buffer))
            
            if speech_detected and not self.is_processing:
                # Start processing speech segment
                self.is_processing = True
                threading.Thread(
                    target=self.process_speech_segment,
                    args=(list(self.audio_buffer),),
                    daemon=True
                ).start()
    
    def detect_speech(self, audio_data):
        """Use VAD to detect speech"""
        # Write audio to temporary file
        import tempfile
        import wave
        
        with tempfile.NamedTemporaryFile(suffix='.wav', delete=False) as tmp:
            with wave.open(tmp.name, 'wb') as wav:
                wav.setnchannels(1)
                wav.setsampwidth(2)
                wav.setframerate(16000)
                
                # Convert float to int16
                audio_int16 = (np.array(audio_data) * 32767).astype(np.int16)
                wav.writeframes(audio_int16.tobytes())
            
            # Run VAD
            try:
                result = subprocess.run([
                    './build/bin/vad-speech-segments',
                    '-vad-model', self.vad_model,
                    '--file', tmp.name,
                    '--no-prints'
                ], capture_output=True, text=True, timeout=5)
                
                # Check if speech detected
                return 'Detected' in result.stdout and 'speech segments' in result.stdout
                
            except subprocess.TimeoutExpired:
                return False
            finally:
                import os
                os.unlink(tmp.name)
    
    def process_speech_segment(self, audio_data):
        """Transcribe detected speech segment"""
        try:
            # Write to temp file and transcribe
            import tempfile
            import wave
            
            with tempfile.NamedTemporaryFile(suffix='.wav', delete=False) as tmp:
                with wave.open(tmp.name, 'wb') as wav:
                    wav.setnchannels(1)
                    wav.setsampwidth(2) 
                    wav.setframerate(16000)
                    
                    audio_int16 = (np.array(audio_data) * 32767).astype(np.int16)
                    wav.writeframes(audio_int16.tobytes())
                
                # Transcribe
                result = subprocess.run([
                    './build/bin/whisper-cli',
                    '-m', self.whisper_model,
                    '-f', tmp.name,
                    '-np'
                ], capture_output=True, text=True, timeout=10)
                
                if result.returncode == 0 and result.stdout.strip():
                    self.speech_queue.put(result.stdout.strip())
                
            import os
            os.unlink(tmp.name)
            
        except Exception as e:
            print(f"Transcription error: {e}")
        finally:
            self.is_processing = False
    
    def get_transcription(self):
        """Get next available transcription"""
        try:
            return self.speech_queue.get_nowait()
        except queue.Empty:
            return None

# Usage example
vad = VADProcessor(
    'models/silero-vad.bin',
    'models/ggml-base.en.bin'
)

# Simulate audio processing
import time
while True:
    # audio_chunk = get_audio_from_microphone()  # Your audio source
    # vad.process_audio_chunk(audio_chunk)
    
    transcription = vad.get_transcription()
    if transcription:
        print(f"Transcribed: {transcription}")
    
    time.sleep(0.1)
```

## Integration Examples

### Multi-model Pipeline

```bash
#!/bin/bash
# multi-model-pipeline.sh

# Pipeline: VAD -> Language Detection -> Transcription/Translation

INPUT_AUDIO="$1"
OUTPUT_DIR="./pipeline_output"
VAD_MODEL="models/silero-vad.bin"

if [ -z "$INPUT_AUDIO" ]; then
    echo "Usage: $0 <audio_file>"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"
filename=$(basename "$INPUT_AUDIO" .wav)

echo "=== Multi-model Processing Pipeline ==="
echo "Input: $INPUT_AUDIO"

# Step 1: Voice Activity Detection
echo "Step 1: Detecting speech segments..."
segments_dir="$OUTPUT_DIR/${filename}_segments"

./build/bin/vad-speech-segments \
    -vad-model "$VAD_MODEL" \
    --file "$INPUT_AUDIO" \
    --output-dir "$segments_dir" \
    --min-length 2000

# Step 2: Language Detection
echo "Step 2: Detecting languages..."
languages_file="$OUTPUT_DIR/${filename}_languages.txt"
> "$languages_file"

for segment in "$segments_dir"/*.wav; do
    if [ -f "$segment" ]; then
        segment_name=$(basename "$segment")
        
        # Use multilingual model for language detection
        lang_result=$(./build/bin/whisper-cli \
            -m models/ggml-base.bin \
            -f "$segment" \
            -l auto \
            -oj \
            -np)
        
        # Extract language from JSON (simplified)
        language=$(echo "$lang_result" | grep '"language"' | sed 's/.*"language":\s*"\([^"]*\)".*/\1/')
        
        echo "$segment_name: $language" >> "$languages_file"
        echo "  $segment_name -> $language"
    fi
done

# Step 3: Targeted Transcription
echo "Step 3: Transcribing with appropriate models..."
transcriptions_dir="$OUTPUT_DIR/${filename}_transcriptions"
mkdir -p "$transcriptions_dir"

while IFS=': ' read -r segment_name language; do
    segment_file="$segments_dir/$segment_name"
    
    if [ -f "$segment_file" ]; then
        # Select appropriate model based on language
        case "$language" in
            "en")
                model="models/ggml-small.en.bin"
                translate=""
                ;;
            "es"|"fr"|"de"|"it"|"pt"|"ja"|"ko"|"zh"|"ru")
                model="models/ggml-small.bin"
                translate="-tr"  # Translate to English
                ;;
            *)
                model="models/ggml-base.bin"
                translate="-tr"
                ;;
        esac
        
        echo "  Transcribing $segment_name (${language}) with $(basename $model)"
        
        ./build/bin/whisper-cli \
            -m "$model" \
            -f "$segment_file" \
            -l "$language" \
            $translate \
            -otxt -osrt -oj \
            -of "$transcriptions_dir/${segment_name%.*}"
    fi
done < "$languages_file"

# Step 4: Combine Results
echo "Step 4: Combining results..."
final_transcript="$OUTPUT_DIR/${filename}_final_transcript.txt"
final_subtitles="$OUTPUT_DIR/${filename}_final_subtitles.srt"

> "$final_transcript"
counter=1

for txt_file in "$transcriptions_dir"/*.txt; do
    if [ -f "$txt_file" ]; then
        segment_name=$(basename "$txt_file" .txt)
        content=$(cat "$txt_file")
        
        echo "[$segment_name] $content" >> "$final_transcript"
        
        # Extract timing for subtitles (if available in filename)
        # Format: segment_001_0.29_2.21.txt
        if [[ $segment_name =~ _([0-9.]+)_([0-9.]+)$ ]]; then
            start_time=${BASH_REMATCH[1]}
            end_time=${BASH_REMATCH[2]}
            
            # Convert to SRT time format
            start_srt=$(printf "%02d:%02d:%06.3f" $((${start_time%.*}/3600)) $(((${start_time%.*}%3600)/60)) ${start_time})
            end_srt=$(printf "%02d:%02d:%06.3f" $((${end_time%.*}/3600)) $(((${end_time%.*}%3600)/60)) ${end_time})
            
            echo "$counter" >> "$final_subtitles"
            echo "${start_srt/./,} --> ${end_srt/./,}" >> "$final_subtitles"
            echo "$content" >> "$final_subtitles"
            echo "" >> "$final_subtitles"
            
            ((counter++))
        fi
    fi
done

echo "=== Pipeline Complete ==="
echo "Results saved to:"
echo "  Transcript: $final_transcript"
echo "  Subtitles: $final_subtitles"
echo "  Segments: $segments_dir/"
echo "  Individual transcriptions: $transcriptions_dir/"
```

### Docker Multi-stage Processing

```dockerfile
# Dockerfile.whisper-pipeline
FROM ubuntu:22.04 as builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Build whisper.cpp
WORKDIR /whisper
COPY . .
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --config Release

# Runtime stage
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    ffmpeg \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# Copy built binaries
COPY --from=builder /whisper/build/bin/ /usr/local/bin/

# Install Python dependencies
RUN pip3 install numpy scipy

# Copy models and scripts
COPY models/ /models/
COPY scripts/ /scripts/

# Processing script
COPY <<EOF /scripts/process_audio.py
#!/usr/bin/env python3
import sys
import subprocess
import json
import os

def process_audio_file(input_file, output_dir):
    """Complete audio processing pipeline"""
    
    # Step 1: Convert to WAV if needed
    wav_file = convert_to_wav(input_file)
    
    # Step 2: VAD processing
    segments = detect_speech_segments(wav_file)
    
    # Step 3: Transcribe segments
    results = []
    for segment in segments:
        transcription = transcribe_segment(segment)
        results.append(transcription)
    
    # Step 4: Save results
    save_results(results, output_dir)

def convert_to_wav(input_file):
    output_file = "/tmp/converted.wav"
    subprocess.run([
        'ffmpeg', '-i', input_file,
        '-ar', '16000', '-ac', '1',
        output_file, '-y'
    ], check=True)
    return output_file

def detect_speech_segments(wav_file):
    result = subprocess.run([
        'vad-speech-segments',
        '-vad-model', '/models/silero-vad.bin',
        '--file', wav_file,
        '--output-dir', '/tmp/segments'
    ], capture_output=True, text=True)
    
    # Parse segment files
    segments = []
    for file in os.listdir('/tmp/segments'):
        if file.endswith('.wav'):
            segments.append(os.path.join('/tmp/segments', file))
    
    return segments

def transcribe_segment(segment_file):
    result = subprocess.run([
        'whisper-cli',
        '-m', '/models/ggml-base.en.bin',
        '-f', segment_file,
        '-oj', '-np'
    ], capture_output=True, text=True)
    
    return json.loads(result.stdout)

def save_results(results, output_dir):
    os.makedirs(output_dir, exist_ok=True)
    
    with open(f"{output_dir}/transcript.txt", 'w') as f:
        for result in results:
            for segment in result.get('transcription', []):
                f.write(segment['text'] + '\n')

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: process_audio.py <input_file> <output_dir>")
        sys.exit(1)
    
    process_audio_file(sys.argv[1], sys.argv[2])
EOF

RUN chmod +x /scripts/process_audio.py

WORKDIR /app
ENTRYPOINT ["/scripts/process_audio.py"]
```

### Kubernetes Job for Batch Processing

```yaml
# whisper-job.yaml
apiVersion: batch/v1
kind: Job
metadata:
  name: whisper-batch-transcription
spec:
  template:
    spec:
      containers:
      - name: whisper
        image: whisper-pipeline:latest
        resources:
          requests:
            memory: "2Gi"
            cpu: "2"
          limits:
            memory: "4Gi"
            cpu: "4"
        volumeMounts:
        - name: audio-files
          mountPath: /input
        - name: transcriptions
          mountPath: /output
        - name: models
          mountPath: /models
        command: ["/bin/bash"]
        args:
        - -c
        - |
          for file in /input/*.wav; do
            filename=$(basename "$file" .wav)
            echo "Processing: $filename"
            
            /scripts/process_audio.py "$file" "/output/$filename"
            
            if [ $? -eq 0 ]; then
              echo "✓ Completed: $filename"
            else
              echo "✗ Failed: $filename"
            fi
          done
      volumes:
      - name: audio-files
        persistentVolumeClaim:
          claimName: audio-files-pvc
      - name: transcriptions
        persistentVolumeClaim:
          claimName: transcriptions-pvc
      - name: models
        configMap:
          name: whisper-models
      restartPolicy: Never
  backoffLimit: 3
```

## Next Steps

- **Performance Optimization**: Read the [performance guide](performance.md)
- **Integration Patterns**: Check the [integration guide](integration-guide.md)
- **Web Examples**: Explore [WebAssembly examples](web-examples.md)
- **Language Bindings**: Learn about [Python and mobile examples](language-bindings.md)
