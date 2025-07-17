# Web Examples

This document covers WebAssembly (WASM) examples that enable whisper.cpp to run in web browsers, providing client-side speech recognition without server dependencies.

## Whisper.wasm - Browser-based Transcription

A complete WebAssembly port enabling whisper.cpp to run directly in web browsers.

### Features
- Client-side processing (no data leaves your computer)
- Support for file upload and microphone recording
- Models up to 'small' size (larger models have performance/memory issues)
- WebRTC-based microphone access
- Multiple output formats

### Live Demo
- **URL**: https://ggml.ai/whisper.cpp/
- **Supported Models**: tiny, base, small
- **Max Audio Length**: 120 seconds
- **Browsers**: Chrome, Firefox, Safari (with WebAssembly SIMD support)

### Building

#### Prerequisites
```bash
# Install Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

#### Build Process
```bash
# Build WebAssembly version
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake

# Build specific WASM example
cd examples/whisper.wasm
make

# Output files:
# - index.html (web interface)
# - whisper.wasm (WebAssembly binary)
# - whisper.js (JavaScript bindings)
```

### Deployment

#### Simple HTTP Server
```bash
# Serve files (HTTPS required for microphone access)
cd examples/whisper.wasm
python3 -m http.server 8080 --bind 127.0.0.1

# Or with SSL for microphone support
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes
python3 -m http.server 8443 --bind 127.0.0.1 --certfile cert.pem --keyfile key.pem
```

#### Docker Deployment
```dockerfile
FROM nginx:alpine

# Copy WASM files
COPY examples/whisper.wasm/ /usr/share/nginx/html/

# Enable CORS and proper MIME types
COPY <<EOF /etc/nginx/conf.d/default.conf
server {
    listen 80;
    
    location / {
        root /usr/share/nginx/html;
        index index.html;
        
        # Enable CORS
        add_header 'Access-Control-Allow-Origin' '*';
        add_header 'Access-Control-Allow-Methods' 'GET, POST, OPTIONS';
        add_header 'Access-Control-Allow-Headers' 'DNT,User-Agent,X-Requested-With,If-Modified-Since,Cache-Control,Content-Type,Range';
        
        # WASM MIME type
        location ~* \\.wasm$ {
            add_header 'Content-Type' 'application/wasm';
        }
        
        # Enable SharedArrayBuffer (required for threading)
        add_header 'Cross-Origin-Embedder-Policy' 'require-corp';
        add_header 'Cross-Origin-Opener-Policy' 'same-origin';
    }
}
EOF

EXPOSE 80
```

### Integration Examples

#### Basic HTML Integration
```html
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Whisper.cpp Web Demo</title>
</head>
<body>
    <div id="main">
        <h1>Speech Recognition</h1>
        
        <!-- Model Selection -->
        <select id="model-select">
            <option value="tiny.en">Tiny (39MB)</option>
            <option value="base.en" selected>Base (142MB)</option>
            <option value="small.en">Small (466MB)</option>
        </select>
        
        <!-- File Upload -->
        <input type="file" id="file-input" accept="audio/*">
        
        <!-- Microphone -->
        <button id="record-button">Start Recording</button>
        
        <!-- Results -->
        <div id="output"></div>
    </div>
    
    <script src="whisper.js"></script>
    <script>
        let whisperModule = null;
        let isRecording = false;
        let mediaRecorder = null;
        
        // Initialize Whisper
        async function initWhisper() {
            whisperModule = await createWhisperModule();
            console.log('Whisper initialized');
        }
        
        // Load model
        async function loadModel(modelName) {
            const modelUrl = `models/ggml-${modelName}.bin`;
            const response = await fetch(modelUrl);
            const modelData = await response.arrayBuffer();
            
            const result = whisperModule.loadModel(new Uint8Array(modelData));
            if (result === 0) {
                console.log(`Model ${modelName} loaded successfully`);
                return true;
            } else {
                console.error(`Failed to load model: ${result}`);
                return false;
            }
        }
        
        // Process audio file
        async function processAudioFile(file) {
            const audioContext = new AudioContext();
            const arrayBuffer = await file.arrayBuffer();
            const audioBuffer = await audioContext.decodeAudioData(arrayBuffer);
            
            // Convert to 16kHz mono
            const sampleRate = 16000;
            const audioData = audioBuffer.getChannelData(0);
            const resampledData = resampleAudio(audioData, audioBuffer.sampleRate, sampleRate);
            
            // Process with Whisper
            const result = whisperModule.transcribe(resampledData);
            document.getElementById('output').textContent = result;
        }
        
        // Microphone recording
        async function startRecording() {
            try {
                const stream = await navigator.mediaDevices.getUserMedia({ 
                    audio: { sampleRate: 16000, channelCount: 1 } 
                });
                
                mediaRecorder = new MediaRecorder(stream);
                const audioChunks = [];
                
                mediaRecorder.ondataavailable = event => {
                    audioChunks.push(event.data);
                };
                
                mediaRecorder.onstop = async () => {
                    const audioBlob = new Blob(audioChunks, { type: 'audio/wav' });
                    await processAudioFile(audioBlob);
                };
                
                mediaRecorder.start();
                isRecording = true;
                document.getElementById('record-button').textContent = 'Stop Recording';
                
            } catch (error) {
                console.error('Microphone access denied:', error);
            }
        }
        
        function stopRecording() {
            if (mediaRecorder && isRecording) {
                mediaRecorder.stop();
                isRecording = false;
                document.getElementById('record-button').textContent = 'Start Recording';
            }
        }
        
        // Audio resampling utility
        function resampleAudio(audioData, fromSampleRate, toSampleRate) {
            if (fromSampleRate === toSampleRate) return audioData;
            
            const ratio = fromSampleRate / toSampleRate;
            const newLength = Math.round(audioData.length / ratio);
            const result = new Float32Array(newLength);
            
            for (let i = 0; i < newLength; i++) {
                const index = i * ratio;
                const indexFloor = Math.floor(index);
                const indexCeil = Math.min(indexFloor + 1, audioData.length - 1);
                const t = index - indexFloor;
                
                result[i] = audioData[indexFloor] * (1 - t) + audioData[indexCeil] * t;
            }
            
            return result;
        }
        
        // Event listeners
        document.getElementById('model-select').addEventListener('change', async (e) => {
            await loadModel(e.target.value);
        });
        
        document.getElementById('file-input').addEventListener('change', (e) => {
            if (e.target.files[0]) {
                processAudioFile(e.target.files[0]);
            }
        });
        
        document.getElementById('record-button').addEventListener('click', () => {
            if (isRecording) {
                stopRecording();
            } else {
                startRecording();
            }
        });
        
        // Initialize on page load
        initWhisper().then(() => {
            loadModel('base.en');
        });
    </script>
</body>
</html>
```

#### React Integration
```jsx
import React, { useState, useEffect, useRef } from 'react';

const WhisperComponent = () => {
    const [whisperModule, setWhisperModule] = useState(null);
    const [isLoading, setIsLoading] = useState(true);
    const [transcription, setTranscription] = useState('');
    const [isRecording, setIsRecording] = useState(false);
    const mediaRecorderRef = useRef(null);
    
    useEffect(() => {
        // Initialize Whisper module
        const initWhisper = async () => {
            try {
                const module = await window.createWhisperModule();
                setWhisperModule(module);
                
                // Load default model
                const response = await fetch('/models/ggml-base.en.bin');
                const modelData = await response.arrayBuffer();
                await module.loadModel(new Uint8Array(modelData));
                
                setIsLoading(false);
            } catch (error) {
                console.error('Failed to initialize Whisper:', error);
            }
        };
        
        initWhisper();
    }, []);
    
    const handleFileUpload = async (event) => {
        const file = event.target.files[0];
        if (!file || !whisperModule) return;
        
        try {
            const audioContext = new AudioContext();
            const arrayBuffer = await file.arrayBuffer();
            const audioBuffer = await audioContext.decodeAudioData(arrayBuffer);
            
            // Process audio and transcribe
            const result = await processAudio(audioBuffer);
            setTranscription(result);
        } catch (error) {
            console.error('Transcription failed:', error);
        }
    };
    
    const processAudio = async (audioBuffer) => {
        // Convert to 16kHz mono and transcribe
        const audioData = audioBuffer.getChannelData(0);
        const sampleRate = 16000;
        const resampledData = resampleAudio(
            audioData, 
            audioBuffer.sampleRate, 
            sampleRate
        );
        
        return whisperModule.transcribe(resampledData);
    };
    
    const startRecording = async () => {
        try {
            const stream = await navigator.mediaDevices.getUserMedia({ 
                audio: true 
            });
            
            mediaRecorderRef.current = new MediaRecorder(stream);
            const audioChunks = [];
            
            mediaRecorderRef.current.ondataavailable = (event) => {
                audioChunks.push(event.data);
            };
            
            mediaRecorderRef.current.onstop = async () => {
                const audioBlob = new Blob(audioChunks, { type: 'audio/wav' });
                const audioContext = new AudioContext();
                const arrayBuffer = await audioBlob.arrayBuffer();
                const audioBuffer = await audioContext.decodeAudioData(arrayBuffer);
                
                const result = await processAudio(audioBuffer);
                setTranscription(result);
            };
            
            mediaRecorderRef.current.start();
            setIsRecording(true);
        } catch (error) {
            console.error('Recording failed:', error);
        }
    };
    
    const stopRecording = () => {
        if (mediaRecorderRef.current) {
            mediaRecorderRef.current.stop();
            setIsRecording(false);
        }
    };
    
    if (isLoading) {
        return <div>Loading Whisper...</div>;
    }
    
    return (
        <div>
            <h2>Whisper Speech Recognition</h2>
            
            <div>
                <input 
                    type="file" 
                    accept="audio/*" 
                    onChange={handleFileUpload}
                />
            </div>
            
            <div>
                <button 
                    onClick={isRecording ? stopRecording : startRecording}
                >
                    {isRecording ? 'Stop Recording' : 'Start Recording'}
                </button>
            </div>
            
            <div>
                <h3>Transcription:</h3>
                <p>{transcription}</p>
            </div>
        </div>
    );
};

export default WhisperComponent;
```

## Command.wasm - Web Voice Commands

WebAssembly version of the command recognition example.

### Features
- Real-time voice command recognition in browser
- Guided mode with predefined command lists
- Low latency for interactive applications
- No server dependencies

### Building
```bash
cd examples/command.wasm
emmake make

# Output:
# - command.html
# - command.wasm  
# - command.js
```

### Usage Example
```html
<!DOCTYPE html>
<html>
<head>
    <title>Voice Commands</title>
</head>
<body>
    <div id="commands">
        <h2>Available Commands:</h2>
        <ul id="command-list"></ul>
    </div>
    
    <button id="listen-button">Start Listening</button>
    <div id="status"></div>
    <div id="result"></div>
    
    <script src="command.js"></script>
    <script>
        const commands = [
            "turn on lights",
            "turn off lights", 
            "play music",
            "stop music",
            "what time is it",
            "open browser"
        ];
        
        let commandModule = null;
        let isListening = false;
        
        async function initCommands() {
            commandModule = await createCommandModule();
            
            // Load model and commands
            const modelResponse = await fetch('models/ggml-tiny.en.bin');
            const modelData = await response.arrayBuffer();
            await commandModule.loadModel(new Uint8Array(modelData));
            
            commandModule.setCommands(commands);
            
            // Display available commands
            const list = document.getElementById('command-list');
            commands.forEach(cmd => {
                const li = document.createElement('li');
                li.textContent = cmd;
                list.appendChild(li);
            });
        }
        
        async function startListening() {
            if (isListening) return;
            
            try {
                const stream = await navigator.mediaDevices.getUserMedia({ 
                    audio: true 
                });
                
                isListening = true;
                document.getElementById('listen-button').textContent = 'Listening...';
                document.getElementById('status').textContent = 'Listening for commands';
                
                // Process audio stream
                const audioContext = new AudioContext();
                const source = audioContext.createMediaStreamSource(stream);
                const processor = audioContext.createScriptProcessor(4096, 1, 1);
                
                processor.onaudioprocess = (event) => {
                    const audioData = event.inputBuffer.getChannelData(0);
                    
                    // Send to command recognition
                    const result = commandModule.processAudio(audioData);
                    if (result) {
                        document.getElementById('result').textContent = 
                            `Recognized: ${result}`;
                        executeCommand(result);
                    }
                };
                
                source.connect(processor);
                processor.connect(audioContext.destination);
                
            } catch (error) {
                console.error('Failed to start listening:', error);
                document.getElementById('status').textContent = 'Error: ' + error.message;
            }
        }
        
        function executeCommand(command) {
            // Handle recognized commands
            const lowerCmd = command.toLowerCase();
            
            if (lowerCmd.includes('lights on')) {
                // Send API call or trigger action
                console.log('Turning lights on');
            } else if (lowerCmd.includes('lights off')) {
                console.log('Turning lights off');
            } else if (lowerCmd.includes('play music')) {
                console.log('Playing music');
            } else if (lowerCmd.includes('stop music')) {
                console.log('Stopping music');
            }
            
            // Visual feedback
            document.getElementById('result').style.backgroundColor = '#90EE90';
            setTimeout(() => {
                document.getElementById('result').style.backgroundColor = '';
            }, 1000);
        }
        
        document.getElementById('listen-button').addEventListener('click', startListening);
        
        // Initialize on page load
        initCommands();
    </script>
</body>
</html>
```

## Stream.wasm - Real-time Web Transcription

WebAssembly port of the streaming transcription example.

### Features
- Real-time streaming transcription
- Configurable processing windows
- VAD (Voice Activity Detection) support
- Low-latency processing

### Integration with Web Audio API
```javascript
class WhisperStream {
    constructor() {
        this.whisperModule = null;
        this.audioContext = null;
        this.processor = null;
        this.isActive = false;
        this.audioBuffer = [];
        this.bufferSize = 16000; // 1 second at 16kHz
    }
    
    async init() {
        this.whisperModule = await createWhisperStreamModule();
        
        // Load model
        const response = await fetch('models/ggml-base.en.bin');
        const modelData = await response.arrayBuffer();
        await this.whisperModule.loadModel(new Uint8Array(modelData));
        
        this.audioContext = new AudioContext({ sampleRate: 16000 });
    }
    
    async start() {
        if (this.isActive) return;
        
        const stream = await navigator.mediaDevices.getUserMedia({ 
            audio: { 
                sampleRate: 16000,
                channelCount: 1,
                echoCancellation: true,
                noiseSuppression: true
            } 
        });
        
        const source = this.audioContext.createMediaStreamSource(stream);
        this.processor = this.audioContext.createScriptProcessor(4096, 1, 1);
        
        this.processor.onaudioprocess = (event) => {
            const audioData = event.inputBuffer.getChannelData(0);
            this.processAudioChunk(audioData);
        };
        
        source.connect(this.processor);
        this.processor.connect(this.audioContext.destination);
        
        this.isActive = true;
    }
    
    processAudioChunk(audioData) {
        // Add to buffer
        this.audioBuffer.push(...audioData);
        
        // Process when buffer is full
        if (this.audioBuffer.length >= this.bufferSize) {
            const processData = new Float32Array(this.audioBuffer.splice(0, this.bufferSize));
            
            // Send to Whisper for processing
            const result = this.whisperModule.transcribeChunk(processData);
            if (result && result.trim()) {
                this.onTranscription(result);
            }
        }
    }
    
    onTranscription(text) {
        // Override this method to handle transcriptions
        console.log('Transcription:', text);
    }
    
    stop() {
        if (this.processor) {
            this.processor.disconnect();
        }
        this.isActive = false;
    }
}

// Usage
const stream = new WhisperStream();
stream.onTranscription = (text) => {
    document.getElementById('transcription').textContent = text;
};

await stream.init();
await stream.start();
```

## Performance Considerations

### Model Size Recommendations
- **tiny.en (39MB)**: Best for real-time, mobile devices
- **base.en (142MB)**: Good balance for desktop browsers
- **small.en (466MB)**: Maximum size for reasonable performance

### Memory Management
```javascript
// Monitor memory usage
function logMemoryUsage() {
    if ('memory' in performance) {
        const mem = performance.memory;
        console.log({
            used: Math.round(mem.usedJSHeapSize / 1024 / 1024) + 'MB',
            total: Math.round(mem.totalJSHeapSize / 1024 / 1024) + 'MB',
            limit: Math.round(mem.jsHeapSizeLimit / 1024 / 1024) + 'MB'
        });
    }
}

// Cleanup function
function cleanup() {
    if (whisperModule) {
        whisperModule.cleanup();
    }
    if (audioContext) {
        audioContext.close();
    }
}

// Call cleanup on page unload
window.addEventListener('beforeunload', cleanup);
```

### Browser Compatibility

#### Required Features
- **WebAssembly**: All modern browsers
- **WebAssembly SIMD**: Chrome 91+, Firefox 89+, Safari 16.4+
- **SharedArrayBuffer**: Chrome 68+, Firefox 79+, Safari 15.2+
- **AudioContext**: All modern browsers
- **MediaDevices.getUserMedia**: All modern browsers (HTTPS required)

#### Feature Detection
```javascript
function checkBrowserSupport() {
    const support = {
        webassembly: typeof WebAssembly === 'object',
        simd: typeof WebAssembly.validate === 'function',
        sharedArrayBuffer: typeof SharedArrayBuffer !== 'undefined',
        audioContext: typeof AudioContext !== 'undefined',
        getUserMedia: navigator.mediaDevices && navigator.mediaDevices.getUserMedia
    };
    
    console.log('Browser Support:', support);
    
    if (!support.webassembly) {
        throw new Error('WebAssembly not supported');
    }
    
    if (!support.simd) {
        console.warn('WASM SIMD not available, performance may be reduced');
    }
    
    return support;
}
```

## Deployment Best Practices

### CDN Distribution
```bash
# Upload to CDN
aws s3 sync examples/whisper.wasm/ s3://your-bucket/whisper/ \
  --exclude "*.cpp" --exclude "*.h" \
  --cache-control "max-age=31536000"

# Set proper MIME types
aws s3 cp s3://your-bucket/whisper/ s3://your-bucket/whisper/ \
  --recursive --exclude "*" --include "*.wasm" \
  --content-type "application/wasm" \
  --metadata-directive REPLACE
```

### Service Worker for Caching
```javascript
// sw.js
const CACHE_NAME = 'whisper-cache-v1';
const urlsToCache = [
    '/whisper.js',
    '/whisper.wasm',
    '/models/ggml-tiny.en.bin',
    '/models/ggml-base.en.bin'
];

self.addEventListener('install', (event) => {
    event.waitUntil(
        caches.open(CACHE_NAME)
            .then((cache) => cache.addAll(urlsToCache))
    );
});

self.addEventListener('fetch', (event) => {
    event.respondWith(
        caches.match(event.request)
            .then((response) => response || fetch(event.request))
    );
});
```

### Progressive Loading
```javascript
// Load models progressively
async function loadModelProgressively(modelName) {
    const modelUrl = `models/ggml-${modelName}.bin`;
    
    // Show loading progress
    const response = await fetch(modelUrl);
    const contentLength = response.headers.get('Content-Length');
    const total = parseInt(contentLength, 10);
    let loaded = 0;
    
    const reader = response.body.getReader();
    const chunks = [];
    
    while (true) {
        const { done, value } = await reader.read();
        
        if (done) break;
        
        chunks.push(value);
        loaded += value.length;
        
        // Update progress
        const progress = (loaded / total) * 100;
        updateProgressBar(progress);
    }
    
    // Combine chunks and load model
    const modelData = new Uint8Array(loaded);
    let offset = 0;
    for (const chunk of chunks) {
        modelData.set(chunk, offset);
        offset += chunk.length;
    }
    
    return whisperModule.loadModel(modelData);
}
```

## Next Steps

- **Language Bindings**: Learn about [Python and mobile examples](language-bindings.md)
- **Advanced Features**: Explore [advanced examples](advanced-examples.md)
- **Performance**: Read [optimization guide](performance.md)
- **Integration**: Check [integration patterns](integration-guide.md)
