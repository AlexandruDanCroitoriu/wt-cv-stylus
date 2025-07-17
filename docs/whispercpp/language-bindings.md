# Language Bindings

This document covers Python bindings, mobile examples (Android, iOS), and other language-specific implementations of whisper.cpp.

## Python Bindings

Simple Python wrapper for whisper.cpp functionality.

### Basic Python Wrapper

The included Python example provides a simple interface for audio transcription.

#### File: `python/whisper_processor.py`

```python
import subprocess
import sys
import os

def process_audio(wav_file, model_name="base.en"):
    """
    Processes an audio file using a specified model and returns the processed string.
    
    :param wav_file: Path to the WAV file
    :param model_name: Name of the model to use
    :return: Processed string output from the audio processing
    :raises: Exception if an error occurs during processing
    """
    
    model = f"./models/ggml-{model_name}.bin"
    
    # Check if the file exists
    if not os.path.exists(model):
        raise FileNotFoundError(f"Model file not found: {model}")
    
    if not os.path.exists(wav_file):
        raise FileNotFoundError(f"WAV file not found: {wav_file}")
    
    full_command = f"./main -m {model} -f {wav_file} -nt"
    
    # Execute the command
    process = subprocess.Popen(
        full_command, 
        shell=True, 
        stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE
    )
    
    # Get the output and error (if any)
    output, error = process.communicate()
    
    if error:
        raise Exception(f"Error processing audio: {error.decode('utf-8')}")
    
    # Process and return the output string
    decoded_str = output.decode('utf-8').strip()
    processed_str = decoded_str.replace('[BLANK_AUDIO]', '').strip()
    
    return processed_str
```

### Usage Examples

#### Basic Usage
```python
from whisper_processor import process_audio

# Transcribe a file
try:
    result = process_audio("meeting.wav", "base.en")
    print(f"Transcription: {result}")
except Exception as e:
    print(f"Error: {e}")
```

#### Batch Processing
```python
import os
import glob
from whisper_processor import process_audio

def batch_transcribe(input_dir, model_name="base.en"):
    """Transcribe all WAV files in a directory"""
    
    wav_files = glob.glob(os.path.join(input_dir, "*.wav"))
    results = {}
    
    for wav_file in wav_files:
        filename = os.path.basename(wav_file)
        try:
            print(f"Processing {filename}...")
            result = process_audio(wav_file, model_name)
            results[filename] = result
            print(f"✓ {filename}: {result[:100]}...")
        except Exception as e:
            results[filename] = f"Error: {e}"
            print(f"✗ {filename}: {e}")
    
    return results

# Usage
results = batch_transcribe("./audio-files/", "small.en")
```

#### Advanced Python Wrapper

```python
import subprocess
import json
import tempfile
import os
from typing import Optional, Dict, List
from dataclasses import dataclass

@dataclass
class TranscriptionResult:
    text: str
    language: str
    segments: List[Dict]
    processing_time: float

class WhisperProcessor:
    def __init__(self, model_path: str, executable_path: str = "./build/bin/whisper-cli"):
        self.model_path = model_path
        self.executable_path = executable_path
        
        if not os.path.exists(model_path):
            raise FileNotFoundError(f"Model not found: {model_path}")
        
        if not os.path.exists(executable_path):
            raise FileNotFoundError(f"Whisper executable not found: {executable_path}")
    
    def transcribe(
        self, 
        audio_file: str,
        language: Optional[str] = None,
        translate: bool = False,
        word_timestamps: bool = False,
        max_len: int = 0,
        threads: int = 4
    ) -> TranscriptionResult:
        """
        Transcribe audio file with advanced options
        """
        
        # Build command
        cmd = [
            self.executable_path,
            "-m", self.model_path,
            "-f", audio_file,
            "-t", str(threads),
            "-oj",  # JSON output
            "-np"   # No prints
        ]
        
        if language:
            cmd.extend(["-l", language])
        
        if translate:
            cmd.append("-tr")
        
        if word_timestamps:
            cmd.append("-owts")
        
        if max_len > 0:
            cmd.extend(["-ml", str(max_len)])
        
        # Create temporary file for JSON output
        with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as tmp:
            tmp_path = tmp.name
        
        cmd.extend(["-of", tmp_path])
        
        try:
            # Execute command
            result = subprocess.run(
                cmd, 
                capture_output=True, 
                text=True, 
                check=True
            )
            
            # Read JSON output
            with open(f"{tmp_path}.json", 'r') as f:
                json_data = json.load(f)
            
            # Parse results
            transcription = ""
            segments = []
            
            if "transcription" in json_data:
                for segment in json_data["transcription"]:
                    transcription += segment["text"]
                    segments.append({
                        "start": segment["offsets"]["from"] / 1000.0,
                        "end": segment["offsets"]["to"] / 1000.0,
                        "text": segment["text"].strip()
                    })
            
            return TranscriptionResult(
                text=transcription.strip(),
                language=json_data.get("result", {}).get("language", "unknown"),
                segments=segments,
                processing_time=0.0  # Could extract from timings
            )
            
        except subprocess.CalledProcessError as e:
            raise Exception(f"Whisper failed: {e.stderr}")
        
        finally:
            # Cleanup temporary files
            for ext in ['', '.json', '.txt']:
                temp_file = f"{tmp_path}{ext}"
                if os.path.exists(temp_file):
                    os.remove(temp_file)
    
    def transcribe_with_timestamps(self, audio_file: str) -> List[Dict]:
        """Get word-level timestamps"""
        result = self.transcribe(audio_file, word_timestamps=True)
        return result.segments
    
    def detect_language(self, audio_file: str) -> str:
        """Auto-detect language"""
        result = self.transcribe(audio_file, language="auto")
        return result.language

# Usage example
processor = WhisperProcessor("./models/ggml-base.en.bin")

# Basic transcription
result = processor.transcribe("meeting.wav")
print(f"Text: {result.text}")
print(f"Language: {result.language}")

# With timestamps
segments = processor.transcribe_with_timestamps("interview.wav")
for segment in segments:
    print(f"[{segment['start']:.2f}s - {segment['end']:.2f}s]: {segment['text']}")

# Language detection
language = processor.detect_language("unknown-language.wav")
print(f"Detected language: {language}")
```

### FastAPI Web Service

```python
from fastapi import FastAPI, File, UploadFile, HTTPException
from fastapi.responses import JSONResponse
import tempfile
import os
from whisper_processor import WhisperProcessor

app = FastAPI(title="Whisper Transcription API")

# Initialize processor
processor = WhisperProcessor("./models/ggml-base.en.bin")

@app.post("/transcribe")
async def transcribe_audio(
    file: UploadFile = File(...),
    language: str = "auto",
    translate: bool = False
):
    """Transcribe uploaded audio file"""
    
    # Validate file type
    allowed_types = ["audio/wav", "audio/mpeg", "audio/flac", "audio/ogg"]
    if file.content_type not in allowed_types:
        raise HTTPException(
            status_code=400, 
            detail=f"Unsupported file type: {file.content_type}"
        )
    
    # Save uploaded file temporarily
    with tempfile.NamedTemporaryFile(delete=False, suffix=".wav") as tmp:
        content = await file.read()
        tmp.write(content)
        tmp_path = tmp.name
    
    try:
        # Transcribe
        result = processor.transcribe(
            tmp_path,
            language=language if language != "auto" else None,
            translate=translate
        )
        
        return JSONResponse({
            "text": result.text,
            "language": result.language,
            "segments": result.segments
        })
        
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
    
    finally:
        # Cleanup
        if os.path.exists(tmp_path):
            os.remove(tmp_path)

@app.get("/health")
async def health_check():
    return {"status": "healthy"}

# Run with: uvicorn whisper_api:app --host 0.0.0.0 --port 8000
```

## Android Examples

### Building for Android

#### Prerequisites
```bash
# Install Android NDK
export ANDROID_NDK=/path/to/android-ndk

# Install CMake for Android
sdkmanager "cmake;3.22.1"
```

#### Build Process
```bash
cd examples/whisper.android

# Build for ARM64
mkdir build-arm64 && cd build-arm64
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-23
make -j4

# Build for ARMv7
mkdir build-armv7 && cd build-armv7
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=armeabi-v7a \
    -DANDROID_PLATFORM=android-23
make -j4
```

### Java JNI Interface

#### WhisperJNI.java
```java
package com.example.whisper;

public class WhisperJNI {
    static {
        System.loadLibrary("whisper_android");
    }
    
    public static native boolean loadModel(String modelPath);
    public static native String transcribeAudio(float[] audioData, int sampleRate);
    public static native String transcribeFile(String filePath);
    public static native void releaseModel();
    
    // Configuration methods
    public static native void setThreads(int threads);
    public static native void setLanguage(String language);
    public static native void setTranslate(boolean translate);
}
```

### Android App Integration

#### MainActivity.java
```java
package com.example.whisper;

import android.Manifest;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

public class MainActivity extends AppCompatActivity {
    private static final int SAMPLE_RATE = 16000;
    private static final int RECORD_PERMISSION_CODE = 1;
    
    private AudioRecord audioRecord;
    private boolean isRecording = false;
    private Thread recordingThread;
    
    private Button recordButton;
    private TextView resultText;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        recordButton = findViewById(R.id.record_button);
        resultText = findViewById(R.id.result_text);
        
        // Load Whisper model
        String modelPath = getFilesDir() + "/ggml-base.en.bin";
        if (WhisperJNI.loadModel(modelPath)) {
            resultText.setText("Model loaded successfully");
        } else {
            resultText.setText("Failed to load model");
            return;
        }
        
        // Configure Whisper
        WhisperJNI.setThreads(4);
        WhisperJNI.setLanguage("en");
        
        recordButton.setOnClickListener(v -> {
            if (isRecording) {
                stopRecording();
            } else {
                startRecording();
            }
        });
        
        // Request microphone permission
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) 
            != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(
                this, 
                new String[]{Manifest.permission.RECORD_AUDIO}, 
                RECORD_PERMISSION_CODE
            );
        }
    }
    
    private void startRecording() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) 
            != PackageManager.PERMISSION_GRANTED) {
            return;
        }
        
        int bufferSize = AudioRecord.getMinBufferSize(
            SAMPLE_RATE,
            AudioFormat.CHANNEL_IN_MONO,
            AudioFormat.ENCODING_PCM_16BIT
        );
        
        audioRecord = new AudioRecord(
            MediaRecorder.AudioSource.MIC,
            SAMPLE_RATE,
            AudioFormat.CHANNEL_IN_MONO,
            AudioFormat.ENCODING_PCM_16BIT,
            bufferSize
        );
        
        audioRecord.startRecording();
        isRecording = true;
        recordButton.setText("Stop Recording");
        
        recordingThread = new Thread(this::recordAudio);
        recordingThread.start();
    }
    
    private void stopRecording() {
        isRecording = false;
        recordButton.setText("Start Recording");
        
        if (audioRecord != null) {
            audioRecord.stop();
            audioRecord.release();
            audioRecord = null;
        }
    }
    
    private void recordAudio() {
        int bufferSize = AudioRecord.getMinBufferSize(
            SAMPLE_RATE,
            AudioFormat.CHANNEL_IN_MONO,
            AudioFormat.ENCODING_PCM_16BIT
        );
        
        short[] audioBuffer = new short[bufferSize];
        float[] floatBuffer = new float[bufferSize];
        
        while (isRecording) {
            int bytesRead = audioRecord.read(audioBuffer, 0, audioBuffer.length);
            
            if (bytesRead > 0) {
                // Convert to float
                for (int i = 0; i < bytesRead; i++) {
                    floatBuffer[i] = audioBuffer[i] / 32768.0f;
                }
                
                // Transcribe audio chunk
                String result = WhisperJNI.transcribeAudio(floatBuffer, SAMPLE_RATE);
                
                if (result != null && !result.trim().isEmpty()) {
                    runOnUiThread(() -> {
                        resultText.setText(result);
                    });
                }
            }
        }
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        WhisperJNI.releaseModel();
        if (isRecording) {
            stopRecording();
        }
    }
}
```

## iOS Examples

### Swift Integration

#### WhisperWrapper.swift
```swift
import Foundation
import AVFoundation

class WhisperWrapper: NSObject {
    private var modelLoaded = false
    
    override init() {
        super.init()
        loadModel()
    }
    
    deinit {
        whisper_free_context()
    }
    
    private func loadModel() {
        guard let modelPath = Bundle.main.path(forResource: "ggml-base.en", ofType: "bin") else {
            print("Model file not found")
            return
        }
        
        let success = whisper_load_model(modelPath)
        modelLoaded = (success == 0)
        
        if modelLoaded {
            whisper_set_threads(4)
            print("Whisper model loaded successfully")
        } else {
            print("Failed to load Whisper model")
        }
    }
    
    func transcribeAudio(audioData: [Float], sampleRate: Int32) -> String? {
        guard modelLoaded else { return nil }
        
        let result = whisper_transcribe_audio(audioData, Int32(audioData.count), sampleRate)
        return result.map { String(cString: $0) }
    }
    
    func transcribeFile(at url: URL) -> String? {
        guard modelLoaded else { return nil }
        
        let filePath = url.path
        let result = whisper_transcribe_file(filePath)
        return result.map { String(cString: $0) }
    }
}

// C interface declarations
@_silgen_name("whisper_load_model")
func whisper_load_model(_ path: UnsafePointer<CChar>) -> Int32

@_silgen_name("whisper_transcribe_audio") 
func whisper_transcribe_audio(_ data: UnsafePointer<Float>, _ count: Int32, _ sampleRate: Int32) -> UnsafePointer<CChar>?

@_silgen_name("whisper_transcribe_file")
func whisper_transcribe_file(_ path: UnsafePointer<CChar>) -> UnsafePointer<CChar>?

@_silgen_name("whisper_set_threads")
func whisper_set_threads(_ threads: Int32)

@_silgen_name("whisper_free_context")
func whisper_free_context()
```

#### ViewController.swift
```swift
import UIKit
import AVFoundation

class ViewController: UIViewController {
    @IBOutlet weak var recordButton: UIButton!
    @IBOutlet weak var transcriptionLabel: UILabel!
    
    private let whisper = WhisperWrapper()
    private var audioEngine = AVAudioEngine()
    private var isRecording = false
    
    override func viewDidLoad() {
        super.viewDidLoad()
        setupAudio()
    }
    
    private func setupAudio() {
        AVAudioSession.sharedInstance().requestRecordPermission { granted in
            if !granted {
                DispatchQueue.main.async {
                    self.transcriptionLabel.text = "Microphone permission required"
                }
            }
        }
    }
    
    @IBAction func recordButtonTapped(_ sender: UIButton) {
        if isRecording {
            stopRecording()
        } else {
            startRecording()
        }
    }
    
    private func startRecording() {
        do {
            try AVAudioSession.sharedInstance().setCategory(.record, mode: .measurement, options: .duckOthers)
            try AVAudioSession.sharedInstance().setActive(true, options: .notifyOthersOnDeactivation)
            
            let inputNode = audioEngine.inputNode
            let recordingFormat = inputNode.outputFormat(forBus: 0)
            
            inputNode.installTap(onBus: 0, bufferSize: 1024, format: recordingFormat) { buffer, _ in
                // Convert audio buffer to float array
                let audioData = self.bufferToFloatArray(buffer)
                
                // Transcribe audio
                if let transcription = self.whisper.transcribeAudio(
                    audioData: audioData, 
                    sampleRate: Int32(recordingFormat.sampleRate)
                ) {
                    DispatchQueue.main.async {
                        self.transcriptionLabel.text = transcription
                    }
                }
            }
            
            audioEngine.prepare()
            try audioEngine.start()
            
            isRecording = true
            recordButton.setTitle("Stop Recording", for: .normal)
            
        } catch {
            print("Failed to start recording: \(error)")
        }
    }
    
    private func stopRecording() {
        audioEngine.stop()
        audioEngine.inputNode.removeTap(onBus: 0)
        
        isRecording = false
        recordButton.setTitle("Start Recording", for: .normal)
    }
    
    private func bufferToFloatArray(_ buffer: AVAudioPCMBuffer) -> [Float] {
        guard let floatChannelData = buffer.floatChannelData else { return [] }
        
        let frameLength = Int(buffer.frameLength)
        let channelCount = Int(buffer.format.channelCount)
        
        var result: [Float] = []
        
        for frame in 0..<frameLength {
            var sum: Float = 0
            for channel in 0..<channelCount {
                sum += floatChannelData[channel][frame]
            }
            result.append(sum / Float(channelCount))
        }
        
        return result
    }
}
```

## Other Language Bindings

### Node.js Bindings

```javascript
// whisper-node.js
const { spawn } = require('child_process');
const fs = require('fs');
const path = require('path');

class WhisperNode {
    constructor(modelPath, executablePath = './build/bin/whisper-cli') {
        this.modelPath = modelPath;
        this.executablePath = executablePath;
        
        if (!fs.existsSync(modelPath)) {
            throw new Error(`Model not found: ${modelPath}`);
        }
    }
    
    async transcribe(audioFile, options = {}) {
        return new Promise((resolve, reject) => {
            const args = [
                '-m', this.modelPath,
                '-f', audioFile,
                '-oj', // JSON output
                '-np'  // No prints
            ];
            
            if (options.language) {
                args.push('-l', options.language);
            }
            
            if (options.translate) {
                args.push('-tr');
            }
            
            if (options.threads) {
                args.push('-t', options.threads.toString());
            }
            
            // Create temporary output file
            const tempFile = path.join(__dirname, `temp_${Date.now()}.json`);
            args.push('-of', tempFile);
            
            const process = spawn(this.executablePath, args);
            
            let stderr = '';
            process.stderr.on('data', (data) => {
                stderr += data.toString();
            });
            
            process.on('close', (code) => {
                if (code !== 0) {
                    reject(new Error(`Whisper failed: ${stderr}`));
                    return;
                }
                
                try {
                    const resultFile = `${tempFile}.json`;
                    const result = JSON.parse(fs.readFileSync(resultFile, 'utf8'));
                    
                    // Cleanup
                    if (fs.existsSync(resultFile)) {
                        fs.unlinkSync(resultFile);
                    }
                    
                    resolve(result);
                } catch (error) {
                    reject(error);
                }
            });
        });
    }
}

module.exports = WhisperNode;

// Usage
const whisper = new WhisperNode('./models/ggml-base.en.bin');

whisper.transcribe('audio.wav', { 
    threads: 4,
    language: 'en'
}).then(result => {
    console.log('Transcription:', result.transcription[0].text);
}).catch(error => {
    console.error('Error:', error);
});
```

### Go Bindings

```go
package main

import (
    "encoding/json"
    "fmt"
    "os"
    "os/exec"
    "path/filepath"
)

type WhisperResult struct {
    Transcription []struct {
        Text      string `json:"text"`
        Timestamps struct {
            From string `json:"from"`
            To   string `json:"to"`
        } `json:"timestamps"`
    } `json:"transcription"`
    Result struct {
        Language string `json:"language"`
    } `json:"result"`
}

type WhisperProcessor struct {
    ModelPath      string
    ExecutablePath string
}

func NewWhisperProcessor(modelPath, executablePath string) (*WhisperProcessor, error) {
    if _, err := os.Stat(modelPath); os.IsNotExist(err) {
        return nil, fmt.Errorf("model not found: %s", modelPath)
    }
    
    if _, err := os.Stat(executablePath); os.IsNotExist(err) {
        return nil, fmt.Errorf("executable not found: %s", executablePath)
    }
    
    return &WhisperProcessor{
        ModelPath:      modelPath,
        ExecutablePath: executablePath,
    }, nil
}

func (w *WhisperProcessor) Transcribe(audioFile string, options map[string]interface{}) (*WhisperResult, error) {
    args := []string{
        "-m", w.ModelPath,
        "-f", audioFile,
        "-oj", // JSON output
        "-np", // No prints
    }
    
    if lang, ok := options["language"].(string); ok {
        args = append(args, "-l", lang)
    }
    
    if translate, ok := options["translate"].(bool); ok && translate {
        args = append(args, "-tr")
    }
    
    if threads, ok := options["threads"].(int); ok {
        args = append(args, "-t", fmt.Sprintf("%d", threads))
    }
    
    // Create temporary output file
    tempFile := filepath.Join(os.TempDir(), fmt.Sprintf("whisper_%d", os.Getpid()))
    args = append(args, "-of", tempFile)
    
    cmd := exec.Command(w.ExecutablePath, args...)
    
    if err := cmd.Run(); err != nil {
        return nil, fmt.Errorf("whisper failed: %v", err)
    }
    
    // Read JSON result
    resultFile := tempFile + ".json"
    defer os.Remove(resultFile)
    
    data, err := os.ReadFile(resultFile)
    if err != nil {
        return nil, fmt.Errorf("failed to read result: %v", err)
    }
    
    var result WhisperResult
    if err := json.Unmarshal(data, &result); err != nil {
        return nil, fmt.Errorf("failed to parse result: %v", err)
    }
    
    return &result, nil
}

func main() {
    processor, err := NewWhisperProcessor(
        "./models/ggml-base.en.bin",
        "./build/bin/whisper-cli",
    )
    if err != nil {
        fmt.Printf("Error: %v\n", err)
        return
    }
    
    result, err := processor.Transcribe("audio.wav", map[string]interface{}{
        "threads": 4,
        "language": "en",
    })
    if err != nil {
        fmt.Printf("Transcription failed: %v\n", err)
        return
    }
    
    if len(result.Transcription) > 0 {
        fmt.Printf("Transcription: %s\n", result.Transcription[0].Text)
        fmt.Printf("Language: %s\n", result.Result.Language)
    }
}
```

## Next Steps

- **Advanced Features**: Explore [advanced examples](advanced-examples.md)
- **Performance**: Read [optimization guide](performance.md)
- **Integration**: Check [integration patterns](integration-guide.md)
- **Web Examples**: Learn about [WebAssembly examples](web-examples.md)
