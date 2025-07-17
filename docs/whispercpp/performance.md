# Performance & Benchmarking

This document covers performance optimization techniques, benchmarking methods, and hardware-specific optimizations for whisper.cpp.

## Benchmarking Tools

### Whisper-bench - Core Performance Testing

The primary tool for measuring whisper.cpp performance across different configurations.

#### Basic Benchmarking

```bash
# Test default configuration
./build/bin/whisper-bench -m ./models/ggml-base.en.bin -t 4

# Test different thread counts
for threads in 1 2 4 8 16; do
    echo "=== Testing $threads threads ==="
    ./build/bin/whisper-bench -m ./models/ggml-base.en.bin -t $threads
done

# Test different models
for model in tiny.en base.en small.en medium.en; do
    echo "=== Testing $model ==="
    ./build/bin/whisper-bench -m ./models/ggml-${model}.bin -t 8
done
```

#### Comprehensive Benchmark Script

```bash
#!/bin/bash
# comprehensive-benchmark.sh

MODELS_DIR="./models"
RESULTS_FILE="benchmark_results_$(date +%Y%m%d_%H%M%S).csv"

# System info
echo "System Information:"
uname -a
lscpu | grep -E "Model name|CPU\(s\)|Thread|MHz"
free -h
echo ""

# CSV header
echo "Model,Threads,LoadTime(ms),MelTime(ms),EncodeTime(ms),TotalTime(ms),MemoryUsage(MB),CPU%" > "$RESULTS_FILE"

# Test configurations
MODELS=("tiny.en" "base.en" "small.en" "medium.en")
THREAD_COUNTS=(1 2 4 8 16)

for model in "${MODELS[@]}"; do
    model_file="$MODELS_DIR/ggml-${model}.bin"
    
    if [ ! -f "$model_file" ]; then
        echo "Warning: $model_file not found, skipping..."
        continue
    fi
    
    echo "Testing model: $model"
    
    for threads in "${THREAD_COUNTS[@]}"; do
        echo "  Threads: $threads"
        
        # Run benchmark with monitoring
        {
            # Start memory/CPU monitoring in background
            (
                while true; do
                    ps -p $$ -o pid,pcpu,pmem,rss --no-headers 2>/dev/null || break
                    sleep 0.1
                done
            ) > "/tmp/monitor_${model}_${threads}.log" &
            monitor_pid=$!
            
            # Run actual benchmark
            output=$(./build/bin/whisper-bench -m "$model_file" -t $threads 2>&1)
            
            # Stop monitoring
            kill $monitor_pid 2>/dev/null
            
            # Parse results
            load_time=$(echo "$output" | grep "load time" | sed 's/.*= *\([0-9.]*\) ms.*/\1/')
            mel_time=$(echo "$output" | grep "mel time" | sed 's/.*= *\([0-9.]*\) ms.*/\1/')
            encode_time=$(echo "$output" | grep "encode time" | sed 's/.*= *\([0-9.]*\) ms.*/\1/')
            total_time=$(echo "$output" | grep "total time" | sed 's/.*= *\([0-9.]*\) ms.*/\1/')
            
            # Get peak memory usage
            if [ -f "/tmp/monitor_${model}_${threads}.log" ]; then
                max_memory=$(awk '{print $4}' "/tmp/monitor_${model}_${threads}.log" | sort -n | tail -1)
                avg_cpu=$(awk '{sum+=$2; count++} END {print sum/count}' "/tmp/monitor_${model}_${threads}.log")
                rm "/tmp/monitor_${model}_${threads}.log"
            else
                max_memory="N/A"
                avg_cpu="N/A"
            fi
            
            # Save to CSV
            echo "$model,$threads,$load_time,$mel_time,$encode_time,$total_time,$max_memory,$avg_cpu" >> "$RESULTS_FILE"
        }
    done
done

echo "Benchmark complete. Results saved to: $RESULTS_FILE"

# Generate summary report
python3 - << EOF
import pandas as pd
import sys

try:
    df = pd.read_csv('$RESULTS_FILE')
    
    print("\n=== BENCHMARK SUMMARY ===")
    print("\nFastest configurations by model:")
    fastest = df.loc[df.groupby('Model')['TotalTime(ms)'].idxmin()]
    print(fastest[['Model', 'Threads', 'TotalTime(ms)']].to_string(index=False))
    
    print("\nOptimal thread count by model (best time/thread ratio):")
    df['TimePerThread'] = df['TotalTime(ms)'] / df['Threads']
    optimal = df.loc[df.groupby('Model')['TimePerThread'].idxmin()]
    print(optimal[['Model', 'Threads', 'TotalTime(ms)', 'TimePerThread']].to_string(index=False))
    
    print(f"\nDetailed results saved to: $RESULTS_FILE")
    
except ImportError:
    print("Install pandas for detailed analysis: pip install pandas")
except Exception as e:
    print(f"Analysis error: {e}")
EOF
```

#### Real-world Performance Testing

```bash
#!/bin/bash
# real-world-benchmark.sh

TEST_AUDIO="test-samples"
MODELS_DIR="./models"
RESULTS_DIR="./benchmark-results-$(date +%Y%m%d)"

mkdir -p "$RESULTS_DIR"

# Create test audio files of different lengths
create_test_files() {
    mkdir -p "$TEST_AUDIO"
    
    # Generate test audio with ffmpeg (sine waves with speech-like characteristics)
    for duration in 10 30 60 180 300; do
        ffmpeg -f lavfi -i "sine=frequency=1000:duration=${duration}" \
               -f lavfi -i "sine=frequency=300:duration=${duration}" \
               -filter_complex "[0:a][1:a]amix=inputs=2" \
               -ar 16000 -ac 1 \
               "$TEST_AUDIO/test_${duration}s.wav" -y
    done
}

# Test transcription performance
test_transcription_performance() {
    local model=$1
    local test_file=$2
    local threads=$3
    
    echo "Testing: $model, file: $(basename $test_file), threads: $threads"
    
    start_time=$(date +%s.%N)
    
    result=$(./build/bin/whisper-cli \
        -m "$MODELS_DIR/ggml-${model}.bin" \
        -f "$test_file" \
        -t $threads \
        -np 2>&1)
    
    end_time=$(date +%s.%N)
    
    duration=$(echo "$end_time - $start_time" | bc)
    
    # Get audio duration
    audio_duration=$(ffprobe -v quiet -show_entries format=duration \
                     -of csv="p=0" "$test_file")
    
    # Calculate real-time factor
    rtf=$(echo "scale=3; $duration / $audio_duration" | bc)
    
    echo "$model,$(basename $test_file),$threads,$audio_duration,$duration,$rtf" >> "$RESULTS_DIR/transcription_performance.csv"
}

# Main execution
echo "Creating test files..."
create_test_files

echo "Model,TestFile,Threads,AudioDuration(s),ProcessTime(s),RTF" > "$RESULTS_DIR/transcription_performance.csv"

echo "Running transcription benchmarks..."
for model in tiny.en base.en small.en; do
    for test_file in "$TEST_AUDIO"/*.wav; do
        for threads in 1 4 8; do
            test_transcription_performance "$model" "$test_file" $threads
        done
    done
done

echo "Generating performance report..."
python3 - << EOF
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('$RESULTS_DIR/transcription_performance.csv')

# Real-time factor analysis
print("=== REAL-TIME FACTOR ANALYSIS ===")
print("(RTF < 1.0 = faster than real-time)")
print()

rtf_summary = df.groupby(['Model', 'Threads'])['RTF'].agg(['mean', 'min', 'max']).round(3)
print(rtf_summary)

# Find best configurations
print("\n=== BEST CONFIGURATIONS FOR REAL-TIME ===")
realtime_configs = df[df['RTF'] < 1.0].groupby('Model')['RTF'].min().sort_values()
print(realtime_configs)

# Memory usage vs performance
print("\n=== PERFORMANCE RECOMMENDATIONS ===")
for model in df['Model'].unique():
    model_data = df[df['Model'] == model]
    best_rtf = model_data.loc[model_data['RTF'].idxmin()]
    print(f"{model}: Best RTF={best_rtf['RTF']:.3f} with {best_rtf['Threads']} threads")
EOF

echo "Benchmark complete. Results in: $RESULTS_DIR"
```

## Hardware-Specific Optimizations

### CPU Optimizations

#### Thread Configuration

```bash
# Detect optimal thread count
detect_optimal_threads() {
    local model=$1
    local test_file=$2
    
    echo "Detecting optimal thread count for $model..."
    
    best_time=999999
    best_threads=1
    
    for threads in $(seq 1 $(nproc)); do
        time_result=$(./build/bin/whisper-cli \
            -m "$model" \
            -f "$test_file" \
            -t $threads \
            -np 2>&1 | grep "total time" | sed 's/.*= *\([0-9.]*\) ms.*/\1/')
        
        if (( $(echo "$time_result < $best_time" | bc -l) )); then
            best_time=$time_result
            best_threads=$threads
        fi
        
        echo "  $threads threads: ${time_result}ms"
    done
    
    echo "Optimal: $best_threads threads (${best_time}ms)"
}

# Usage
detect_optimal_threads "./models/ggml-base.en.bin" "test.wav"
```

#### CPU Feature Detection

```bash
#!/bin/bash
# cpu-optimization.sh

echo "=== CPU Feature Detection ==="

# Check CPU features
if grep -q avx2 /proc/cpuinfo; then
    echo "✓ AVX2 supported"
    AVX2_FLAG="-DGGML_AVX2=ON"
else
    echo "✗ AVX2 not supported"
    AVX2_FLAG="-DGGML_AVX2=OFF"
fi

if grep -q fma /proc/cpuinfo; then
    echo "✓ FMA supported"
    FMA_FLAG="-DGGML_FMA=ON"
else
    echo "✗ FMA not supported"
    FMA_FLAG="-DGGML_FMA=OFF"
fi

if grep -q avx512 /proc/cpuinfo; then
    echo "✓ AVX512 supported"
    AVX512_FLAG="-DGGML_AVX512=ON"
else
    echo "✗ AVX512 not supported"
    AVX512_FLAG="-DGGML_AVX512=OFF"
fi

# Rebuild with optimal flags
echo "Building with optimal CPU flags..."
cmake -B build-optimized -S . \
    $AVX2_FLAG \
    $FMA_FLAG \
    $AVX512_FLAG \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS="-O3 -march=native" \
    -DCMAKE_CXX_FLAGS="-O3 -march=native"

cmake --build build-optimized --config Release -j$(nproc)

echo "Optimized build complete in: build-optimized/"
```

### GPU Acceleration

#### CUDA Setup

```bash
# Build with CUDA support
cmake -B build-cuda -S . \
    -DWHISPER_CUDA=ON \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build-cuda --config Release

# Test CUDA performance
echo "Testing CUDA vs CPU performance..."

echo "CPU Performance:"
time ./build/bin/whisper-cli -m ./models/ggml-base.en.bin -f test.wav -t 8

echo "CUDA Performance:"
time ./build-cuda/bin/whisper-cli -m ./models/ggml-base.en.bin -f test.wav -ngl 32
```

#### GPU Monitoring

```bash
#!/bin/bash
# gpu-benchmark.sh

# Monitor GPU usage during inference
monitor_gpu() {
    while true; do
        nvidia-smi --query-gpu=timestamp,memory.used,memory.total,utilization.gpu,temperature.gpu \
                   --format=csv,noheader,nounits >> gpu_usage.log
        sleep 0.1
    done
}

# Start monitoring
monitor_gpu &
monitor_pid=$!

# Run benchmark
./build-cuda/bin/whisper-cli \
    -m ./models/ggml-small.en.bin \
    -f long-audio.wav \
    -ngl 32 \
    -t 8

# Stop monitoring
kill $monitor_pid

# Analyze GPU usage
python3 - << EOF
import pandas as pd

df = pd.read_csv('gpu_usage.log', names=['timestamp', 'memory_used', 'memory_total', 'gpu_util', 'temp'])

print("=== GPU USAGE ANALYSIS ===")
print(f"Peak memory usage: {df['memory_used'].max()} MB / {df['memory_total'].iloc[0]} MB")
print(f"Average GPU utilization: {df['gpu_util'].mean():.1f}%")
print(f"Peak GPU utilization: {df['gpu_util'].max()}%")
print(f"Average temperature: {df['temp'].mean():.1f}°C")
print(f"Peak temperature: {df['temp'].max()}°C")
EOF
```

#### OpenCL Support

```bash
# Build with OpenCL (for AMD GPUs)
cmake -B build-opencl -S . \
    -DWHISPER_OPENCL=ON \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build-opencl --config Release

# Test OpenCL
./build-opencl/bin/whisper-cli -m ./models/ggml-base.en.bin -f test.wav
```

### Memory Optimization

#### Memory Usage Analysis

```bash
#!/bin/bash
# memory-analysis.sh

analyze_memory_usage() {
    local model=$1
    local audio_file=$2
    
    echo "Analyzing memory usage for $model..."
    
    # Use valgrind for detailed memory analysis
    valgrind --tool=massif \
             --massif-out-file=massif.out \
             ./build/bin/whisper-cli \
             -m "$model" \
             -f "$audio_file" \
             -t 4
    
    # Parse massif output
    ms_print massif.out > memory_report.txt
    
    # Get peak memory usage
    peak_mb=$(grep -o "MB ([0-9.]*MB)" memory_report.txt | head -1 | sed 's/MB (\(.*\)MB)/\1/')
    
    echo "Peak memory usage: ${peak_mb}MB"
    
    # Cleanup
    rm massif.out memory_report.txt
}

# Test different models
for model in tiny.en base.en small.en; do
    analyze_memory_usage "./models/ggml-${model}.bin" "test.wav"
done
```

#### Memory-Constrained Optimization

```bash
# Optimize for low memory usage
./build/bin/whisper-cli \
    -m ./models/ggml-tiny.en.bin \
    -f audio.wav \
    -ac 256 \          # Reduce audio context
    -mc 512 \          # Reduce max context tokens
    -t 2               # Fewer threads to reduce memory

# For very long audio files
./build/bin/whisper-cli \
    -m ./models/ggml-base.en.bin \
    -f very-long-audio.wav \
    -d 300000 \        # Process in 5-minute chunks
    -ot 0              # Start from beginning each time
```

## Performance Tuning

### Model Selection Guidelines

```bash
#!/bin/bash
# model-selection-guide.sh

# Performance vs Quality matrix
cat << EOF
=== MODEL SELECTION GUIDE ===

Use Case                    | Recommended Model | Threads | Memory  | RTF
---------------------------|-------------------|---------|---------|-----
IoT/Embedded devices       | tiny.en          | 1-2     | <100MB  | 0.1-0.3
Mobile applications        | tiny.en/base.en  | 2-4     | <200MB  | 0.2-0.5
Real-time desktop apps     | base.en/small.en | 4-8     | <500MB  | 0.3-0.8
Batch processing           | small.en/medium  | 8-16    | <2GB    | 0.5-2.0
High-quality transcription | medium/large     | 16+     | 2-8GB   | 1.0-5.0

RTF = Real-Time Factor (processing_time / audio_duration)
RTF < 1.0 = faster than real-time
EOF

# Automated model recommendation
recommend_model() {
    local use_case=$1
    local available_memory_gb=$2
    local cpu_cores=$3
    
    echo "Recommendations for: $use_case"
    echo "Available: ${available_memory_gb}GB RAM, $cpu_cores cores"
    
    case "$use_case" in
        "realtime")
            if (( $(echo "$available_memory_gb < 1" | bc -l) )); then
                echo "  Recommended: tiny.en (limited memory)"
            elif (( cpu_cores < 4 )); then
                echo "  Recommended: tiny.en or base.en (limited CPU)"
            else
                echo "  Recommended: base.en or small.en"
            fi
            ;;
        "batch")
            if (( $(echo "$available_memory_gb > 4" | bc -l) )); then
                echo "  Recommended: medium or large"
            else
                echo "  Recommended: small.en"
            fi
            ;;
        "mobile")
            echo "  Recommended: tiny.en"
            ;;
        *)
            echo "  Unknown use case"
            ;;
    esac
}

# Get system info
memory_gb=$(free -g | awk '/^Mem:/{print $2}')
cpu_cores=$(nproc)

echo "System: ${memory_gb}GB RAM, ${cpu_cores} CPU cores"
echo

recommend_model "realtime" $memory_gb $cpu_cores
recommend_model "batch" $memory_gb $cpu_cores
recommend_model "mobile" $memory_gb $cpu_cores
```

### Quality vs Speed Trade-offs

```bash
#!/bin/bash
# quality-speed-tradeoffs.sh

test_quality_speed() {
    local model=$1
    local audio_file=$2
    local reference_text="$3"
    
    echo "Testing $model..."
    
    # Speed test
    start_time=$(date +%s.%N)
    result=$(./build/bin/whisper-cli -m "$model" -f "$audio_file" -np)
    end_time=$(date +%s.%N)
    
    processing_time=$(echo "$end_time - $start_time" | bc)
    
    # Audio duration
    audio_duration=$(ffprobe -v quiet -show_entries format=duration -of csv="p=0" "$audio_file")
    
    # Real-time factor
    rtf=$(echo "scale=3; $processing_time / $audio_duration" | bc)
    
    # Quality estimate (word error rate approximation)
    if [ -n "$reference_text" ]; then
        # Simple word count comparison (replace with proper WER calculation)
        reference_words=$(echo "$reference_text" | wc -w)
        result_words=$(echo "$result" | wc -w)
        word_diff=$(( reference_words - result_words ))
        if [ $word_diff -lt 0 ]; then word_diff=$(( -word_diff )); fi
        accuracy=$(echo "scale=2; (1 - $word_diff / $reference_words) * 100" | bc)
    else
        accuracy="N/A"
    fi
    
    printf "%-15s | %8.3f | %8.2f%% | %s\n" \
           "$(basename $model)" "$rtf" "$accuracy" "$(echo $result | cut -c1-50)..."
}

echo "Model Quality vs Speed Analysis"
echo "==============================================="
printf "%-15s | %8s | %8s | %s\n" "Model" "RTF" "Accuracy" "Sample Output"
echo "---------------|----------|----------|------------------"

# Test with sample audio and known reference
AUDIO_FILE="test-sample.wav"
REFERENCE="The quick brown fox jumps over the lazy dog"

for model in ./models/ggml-tiny.en.bin ./models/ggml-base.en.bin ./models/ggml-small.en.bin; do
    if [ -f "$model" ]; then
        test_quality_speed "$model" "$AUDIO_FILE" "$REFERENCE"
    fi
done
```

### Configuration Optimization

```bash
#!/bin/bash
# config-optimization.sh

optimize_for_use_case() {
    local use_case=$1
    
    case "$use_case" in
        "speed")
            echo "Speed-optimized configuration:"
            echo "./build/bin/whisper-cli \\"
            echo "  -m ./models/ggml-tiny.en.bin \\"
            echo "  -t $(nproc) \\"
            echo "  -bo 1 \\"
            echo "  -bs 1 \\"
            echo "  -ac 512 \\"
            echo "  -f audio.wav"
            ;;
        "quality")
            echo "Quality-optimized configuration:"
            echo "./build/bin/whisper-cli \\"
            echo "  -m ./models/ggml-small.en.bin \\"
            echo "  -t 8 \\"
            echo "  -bo 10 \\"
            echo "  -bs 10 \\"
            echo "  -wt 0.005 \\"
            echo "  -et 2.0 \\"
            echo "  -f audio.wav"
            ;;
        "balanced")
            echo "Balanced configuration:"
            echo "./build/bin/whisper-cli \\"
            echo "  -m ./models/ggml-base.en.bin \\"
            echo "  -t $(($(nproc) / 2)) \\"
            echo "  -bo 5 \\"
            echo "  -bs 5 \\"
            echo "  -f audio.wav"
            ;;
        "realtime")
            echo "Real-time configuration:"
            echo "./build/bin/whisper-stream \\"
            echo "  -m ./models/ggml-base.en.bin \\"
            echo "  -t 4 \\"
            echo "  --step 500 \\"
            echo "  --length 3000 \\"
            echo "  -ac 512"
            ;;
    esac
}

echo "=== CONFIGURATION OPTIMIZATION ==="
echo

for config in speed quality balanced realtime; do
    optimize_for_use_case "$config"
    echo
done
```

## Profiling and Debugging

### Performance Profiling

```bash
#!/bin/bash
# profile-performance.sh

# CPU profiling with perf
profile_cpu() {
    echo "CPU profiling..."
    
    perf record -g ./build/bin/whisper-cli \
        -m ./models/ggml-base.en.bin \
        -f test-audio.wav \
        -t 8
    
    perf report > cpu_profile.txt
    echo "CPU profile saved to: cpu_profile.txt"
}

# Memory profiling with valgrind
profile_memory() {
    echo "Memory profiling..."
    
    valgrind --tool=memcheck \
             --leak-check=full \
             --show-leak-kinds=all \
             --track-origins=yes \
             ./build/bin/whisper-cli \
             -m ./models/ggml-base.en.bin \
             -f test-audio.wav \
             -t 4 \
             2> memory_profile.txt
    
    echo "Memory profile saved to: memory_profile.txt"
}

# Cache profiling
profile_cache() {
    echo "Cache profiling..."
    
    perf stat -e cache-misses,cache-references,LLC-loads,LLC-load-misses \
        ./build/bin/whisper-cli \
        -m ./models/ggml-base.en.bin \
        -f test-audio.wav \
        -t 8
}

# Run all profiling
if command -v perf >/dev/null 2>&1; then
    profile_cpu
    profile_cache
else
    echo "perf not available, skipping CPU profiling"
fi

if command -v valgrind >/dev/null 2>&1; then
    profile_memory
else
    echo "valgrind not available, skipping memory profiling"
fi
```

### Debug Builds

```bash
# Build debug version for profiling
cmake -B build-debug -S . \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_FLAGS="-g -O0 -fno-omit-frame-pointer" \
    -DCMAKE_CXX_FLAGS="-g -O0 -fno-omit-frame-pointer"

cmake --build build-debug --config Debug

# Run with debug output
./build-debug/bin/whisper-cli \
    -m ./models/ggml-tiny.en.bin \
    -f test.wav \
    -debug \
    -pp \
    -pc
```

## Performance Results Database

### Automated Performance Tracking

```python
#!/usr/bin/env python3
# performance-tracker.py

import sqlite3
import subprocess
import json
import time
import platform
import psutil
from datetime import datetime

class PerformanceTracker:
    def __init__(self, db_path="performance.db"):
        self.db_path = db_path
        self.init_database()
    
    def init_database(self):
        conn = sqlite3.connect(self.db_path)
        conn.execute("""
            CREATE TABLE IF NOT EXISTS benchmarks (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT,
                system_info TEXT,
                model TEXT,
                threads INTEGER,
                audio_duration REAL,
                processing_time REAL,
                rtf REAL,
                memory_peak INTEGER,
                cpu_avg REAL,
                config TEXT
            )
        """)
        conn.commit()
        conn.close()
    
    def get_system_info(self):
        return {
            'platform': platform.platform(),
            'processor': platform.processor(),
            'cpu_count': psutil.cpu_count(),
            'memory_total': psutil.virtual_memory().total,
            'python_version': platform.python_version()
        }
    
    def run_benchmark(self, model_path, audio_file, threads=4, config=None):
        """Run benchmark and return results"""
        
        # Get audio duration
        duration_cmd = [
            'ffprobe', '-v', 'quiet', 
            '-show_entries', 'format=duration',
            '-of', 'csv=p=0', audio_file
        ]
        audio_duration = float(subprocess.check_output(duration_cmd).decode().strip())
        
        # Run whisper benchmark
        start_time = time.time()
        
        cmd = [
            './build/bin/whisper-cli',
            '-m', model_path,
            '-f', audio_file,
            '-t', str(threads),
            '-np'
        ]
        
        if config:
            cmd.extend(config)
        
        # Monitor memory during execution
        process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        
        memory_samples = []
        cpu_samples = []
        
        while process.poll() is None:
            try:
                proc_info = psutil.Process(process.pid)
                memory_samples.append(proc_info.memory_info().rss)
                cpu_samples.append(proc_info.cpu_percent())
            except psutil.NoSuchProcess:
                break
            time.sleep(0.1)
        
        stdout, stderr = process.communicate()
        end_time = time.time()
        
        processing_time = end_time - start_time
        rtf = processing_time / audio_duration
        memory_peak = max(memory_samples) if memory_samples else 0
        cpu_avg = sum(cpu_samples) / len(cpu_samples) if cpu_samples else 0
        
        return {
            'model': model_path,
            'threads': threads,
            'audio_duration': audio_duration,
            'processing_time': processing_time,
            'rtf': rtf,
            'memory_peak': memory_peak,
            'cpu_avg': cpu_avg,
            'config': json.dumps(config) if config else None,
            'success': process.returncode == 0
        }
    
    def save_benchmark(self, results):
        """Save benchmark results to database"""
        conn = sqlite3.connect(self.db_path)
        
        conn.execute("""
            INSERT INTO benchmarks 
            (timestamp, system_info, model, threads, audio_duration, 
             processing_time, rtf, memory_peak, cpu_avg, config)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, (
            datetime.now().isoformat(),
            json.dumps(self.get_system_info()),
            results['model'],
            results['threads'],
            results['audio_duration'],
            results['processing_time'],
            results['rtf'],
            results['memory_peak'],
            results['cpu_avg'],
            results['config']
        ))
        
        conn.commit()
        conn.close()
    
    def generate_report(self):
        """Generate performance report"""
        conn = sqlite3.connect(self.db_path)
        
        # Best performing configurations
        query = """
            SELECT model, threads, AVG(rtf) as avg_rtf, MIN(rtf) as best_rtf
            FROM benchmarks 
            GROUP BY model, threads 
            ORDER BY model, avg_rtf
        """
        
        results = conn.execute(query).fetchall()
        
        print("=== PERFORMANCE REPORT ===")
        print("Model           | Threads | Avg RTF | Best RTF")
        print("----------------|---------|---------|----------")
        
        for row in results:
            print(f"{row[0]:<15} | {row[1]:<7} | {row[2]:<7.3f} | {row[3]:<8.3f}")
        
        conn.close()

# Usage example
if __name__ == "__main__":
    tracker = PerformanceTracker()
    
    # Run benchmarks
    models = [
        "./models/ggml-tiny.en.bin",
        "./models/ggml-base.en.bin",
        "./models/ggml-small.en.bin"
    ]
    
    for model in models:
        for threads in [1, 2, 4, 8]:
            try:
                results = tracker.run_benchmark(
                    model, 
                    "test-audio.wav", 
                    threads
                )
                
                if results['success']:
                    tracker.save_benchmark(results)
                    print(f"✓ {model} ({threads}t): RTF={results['rtf']:.3f}")
                else:
                    print(f"✗ {model} ({threads}t): Failed")
                    
            except Exception as e:
                print(f"✗ {model} ({threads}t): Error - {e}")
    
    # Generate report
    tracker.generate_report()
```

## Next Steps

- **Integration Guide**: Learn about [integration patterns](integration-guide.md)
- **Advanced Examples**: Explore [advanced features](advanced-examples.md)
- **Language Bindings**: Check [Python and mobile examples](language-bindings.md)
- **Web Examples**: Try [WebAssembly examples](web-examples.md)
