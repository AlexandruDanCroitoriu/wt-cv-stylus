#include "003-Components/VoiceRecorder.h"
#include "003-Components/Button.h"
#include "999-ExternalServices/WhisperCliService.h"
#include <Wt/WApplication.h>
#include <Wt/WJavaScript.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>
#include <Wt/WTimer.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

VoiceRecorder::VoiceRecorder() 
    : is_recording_(false), 
    recording_timer_(std::make_unique<Wt::WTimer>()),
    js_signal_voice_recording_supported_(this, "voiceRecordingSupported"),
    js_signal_microphone_avalable_(this, "microphoneAvailable"),
    js_signal_audio_widget_has_media_(this, "audioWidgetHasMedia"),
    is_audio_supported_(false),
    is_microphone_available_(false),
    is_enabled_(true),
    transcription_in_progress_(false)
{
    // No external dependencies needed - using built-in WAV encoding
    
    js_signal_voice_recording_supported_.connect(this, [=](bool is_supported){ 
        std::cout << "Voice recording support status: " << (is_supported ? "Supported" : "Not Supported") << std::endl;
        is_audio_supported_ = is_supported; 
            if (is_supported) {
                // No UI messages - keep display clean for transcription only
                enable();
            }else {
                // Show error message in transcription area if audio not supported
                transcription_display_->setText("Audio recording is not supported in your browser. Please use a modern browser (Chrome, Firefox, Edge) and ensure your microphone is connected.");
                disable();
        }
    });
    js_signal_microphone_avalable_.connect(this, [=](bool is_available){
        std::cout << "Microphone availability status: " << (is_available ? "Available" : "Not Available") << std::endl;
        is_microphone_available_ = is_available;
        if(is_available) {
            // No UI messages - keep display clean for transcription only
            enable();
        } else {
            // Show error message in transcription area if microphone not available
            transcription_display_->setText("No microphone detected. Please connect a microphone and refresh the page.");
            disable();
        }
    });
    js_signal_audio_widget_has_media_.connect(this, [=](bool has_media){
        std::cout << "Audio widget media status: " << (has_media ? "Has Media" : "No Media") << std::endl;
        if (has_media) {
            uploadFile();
            // Note: transcription will be started from onFileUploaded() after file is saved
        }
    });

    setupUI();
    setupJavaScriptRecorder();
    // Initialize the recorder when the DOM is ready - but don't block UI loading
    doJavaScript("setTimeout(function() { if (" + jsRef() + ") { " + jsRef() + ".initAsync(); } }, 2000);");
}

VoiceRecorder::~VoiceRecorder()
{
    // Stop timer if running
    if (recording_timer_ && recording_timer_->isActive()) {
        recording_timer_->stop();
    }
    
    // No thread cleanup needed since we use detached threads
}

void VoiceRecorder::setupUI()
{
    clear();
    
    // Container for voice recorder functionality
    auto main_wrapper = addNew<Wt::WContainerWidget>();
    main_wrapper->setStyleClass("w-full max-w-2xl mx-auto p-4");

    // Control elements - compact but functional
    auto controls_container = main_wrapper->addNew<Wt::WContainerWidget>();
    controls_container->setStyleClass("flex flex-col space-y-4 mb-6");
    
    // Status indicator - compact display
    status_text_ = controls_container->addNew<Wt::WText>("Ready to record audio");
    status_text_->setStyleClass("text-sm text-gray-600 mb-2");

    // Recording controls in a horizontal layout
    auto recording_controls = controls_container->addNew<Wt::WContainerWidget>();
    recording_controls->setStyleClass("flex items-center space-x-4");

    // Start/Stop recording button - prominent but compact
    microphone_svg_ = std::string(Wt::WString::tr("app:microphone-svg").toUTF8());
    play_pause_btn_ = recording_controls->addNew<Button>(microphone_svg_, "text-lg", PenguinUiWidgetTheme::BtnSuccessAction);
    play_pause_btn_->clicked().connect(this, [=]
    {
        if(!is_recording_) {
            startRecording();
        }else {
            stopRecording();
        }
    });

    // Audio player - compact controls
    audio_player_ = recording_controls->addNew<Wt::WAudio>();
    audio_player_->setOptions(Wt::PlayerOption::Controls);
    audio_player_->setAlternativeContent(std::make_unique<Wt::WText>("Audio player not supported"));
    audio_player_->setStyleClass("max-w-xs");

    // Recording timer info
    recording_info_ = controls_container->addNew<Wt::WContainerWidget>();
    recording_info_->setStyleClass("text-xs text-gray-500");
    
    
    file_upload_ = addNew<Wt::WFileUpload>();
    file_upload_->setStyleClass("text-sm hidden");

    // Connect file upload signal
    file_upload_->uploaded().connect(this, &VoiceRecorder::onFileUploaded);
    file_upload_->fileTooLarge().connect(this, &VoiceRecorder::onFileTooLarge);
    file_upload_->hide();
    
    // MAIN DISPLAY: Prominent transcription output
    auto transcription_container = main_wrapper->addNew<Wt::WContainerWidget>();
    transcription_container->setStyleClass("border-t pt-6");
    
    auto transcription_label = transcription_container->addNew<Wt::WText>("Transcription:");
    transcription_label->setStyleClass("text-lg font-semibold text-gray-800 block mb-3");
    
    transcription_display_ = transcription_container->addNew<Wt::WTextArea>();
    transcription_display_->setStyleClass("w-full min-h-[200px] p-4 border border-outline rounded-lg bg-surface text-on-surface text-base leading-relaxed");
    transcription_display_->setPlaceholderText("Audio transcription will appear here...");
    transcription_display_->setReadOnly(true);
    
    // Setup recording timer
    recording_timer_->setInterval(std::chrono::milliseconds(1000)); // Update every second
    recording_timer_->timeout().connect(this, &VoiceRecorder::updateRecordingTimer);
    

}

void VoiceRecorder::startRecording()
{
    if (!is_recording_) {        
        // Call the start member function
        callJavaScriptMember("start", "");
        
        is_recording_ = true;
        recording_start_time_ = std::chrono::steady_clock::now();
        
        // Start the timer and update button
        recording_timer_->start();
        play_pause_btn_->setText("0");
        play_pause_btn_->toggleStyleClass("animate-pulse", true);
        play_pause_btn_->toggleStyleClass("outline", true);
        play_pause_btn_->toggleStyleClass("outline-2", true);
        
        status_text_->setText("Recording audio... Speak now");
        // Removed upload_btn_->disable() since button was removed
    }
}

void VoiceRecorder::stopRecording()
{
    if (is_recording_) {
        // Call the stop member function
        callJavaScriptMember("stop", "");
        is_recording_ = false;
        
        // Stop the timer and restore microphone icon
        recording_timer_->stop();
        play_pause_btn_->setText(microphone_svg_);
        play_pause_btn_->toggleStyleClass("animate-pulse", false);
        play_pause_btn_->toggleStyleClass("outline-2", false);
        play_pause_btn_->toggleStyleClass("outline", false);

        status_text_->setText("Audio recording stopped");
        // Removed upload_btn_->enable() since button was removed
    }
}


void VoiceRecorder::onFileUploaded()
{
    std::cout << "File uploaded successfully." << std::endl;
    std::string tempFileName = file_upload_->spoolFileName();
    std::string clientFileName = file_upload_->clientFileName().toUTF8();
    
    if (!tempFileName.empty()) {
        // Create audio-files directory if it doesn't exist
        std::string audioDir = createAudioFilesDirectory();
        
        if (!audioDir.empty()) {
            // Generate unique filename to avoid conflicts - force new filename each time
            std::string uniqueFileName = generateUniqueFileName(clientFileName);
            std::string permanentPath = audioDir + "/" + uniqueFileName;
            
            // Save the uploaded file to the permanent location
            if (saveAudioFile(tempFileName, permanentPath)) {
                // Update current audio file path for this recording
                current_audio_file_ = permanentPath;
                status_text_->setText("Audio file saved: " + uniqueFileName);
                std::cout << "Audio file saved: " << permanentPath << std::endl;
                
                // Clear any previous transcription to avoid showing old results
                transcription_display_->setText("⏳ Transcribing audio, please wait...");
                
                // Start automatic transcription in background
                transcribeCurrentAudio();
            } else {
                status_text_->setText("Error: Failed to save audio file");
                std::cout << "Failed to save audio file to: " << permanentPath << std::endl;
            }
        } else {
            status_text_->setText("Error: Could not create audio-files directory");
            // Fallback to using temp file with unique name
            current_audio_file_ = tempFileName;
            std::cout << "Using temp file for transcription: " << tempFileName << std::endl;
            
            // Clear any previous transcription to avoid showing old results
            transcription_display_->setText("⏳ Transcribing audio, please wait...");
            
            // Start automatic transcription even with temp file
            transcribeCurrentAudio();
        }
    } else {
        status_text_->setText("Error: No file received for upload");
        std::cout << "Error: tempFileName is empty" << std::endl;
    }
}

void VoiceRecorder::onFileTooLarge()
{
    status_text_->setText("Error: Audio file too large. Please record a shorter audio clip.");
}

void VoiceRecorder::uploadFile()
{
    if (file_upload_->canUpload()) {
        status_text_->setText("Uploading file...");
        file_upload_->upload();
    } else {
        status_text_->setText("No file selected for upload or upload already in progress.");
    }
}


void VoiceRecorder::enable()
{
    is_enabled_ = true;
    audio_player_->enable();
    play_pause_btn_->enable();
}

void VoiceRecorder::disable()
{
    is_enabled_ = false;
    audio_player_->disable();
    play_pause_btn_->disable();
}


void VoiceRecorder::setupJavaScriptRecorder()
{
    // Initialize the JavaScript object for media recording
    setJavaScriptMember("audioContext", "null");
    setJavaScriptMember("mediaRecorder", "null");
    setJavaScriptMember("recordedSamples", "[]");
    setJavaScriptMember("recordedBlob", "null");
    setJavaScriptMember("audioUrl", "null");
    setJavaScriptMember("isSupported", "false");
    setJavaScriptMember("audioElement", "null");
    setJavaScriptMember("processorNode", "null");
    setJavaScriptMember("sourceNode", "null");
    setJavaScriptMember("mediaStream", "null");
    
    // WAV encoder helper functions
    setJavaScriptMember("encodeWAV", R"(
        function(samples, sampleRate) {
            var buffer = new ArrayBuffer(44 + samples.length * 2);
            var view = new DataView(buffer);
            
            // WAV header
            var writeString = function(offset, string) {
                for (var i = 0; i < string.length; i++) {
                    view.setUint8(offset + i, string.charCodeAt(i));
                }
            };
            
            writeString(0, 'RIFF');
            view.setUint32(4, 36 + samples.length * 2, true);
            writeString(8, 'WAVE');
            writeString(12, 'fmt ');
            view.setUint32(16, 16, true);
            view.setUint16(20, 1, true); // PCM
            view.setUint16(22, 1, true); // mono
            view.setUint32(24, sampleRate, true);
            view.setUint32(28, sampleRate * 2, true);
            view.setUint16(32, 2, true);
            view.setUint16(34, 16, true);
            writeString(36, 'data');
            view.setUint32(40, samples.length * 2, true);
            
            // Convert float samples to 16-bit PCM
            var offset = 44;
            for (var i = 0; i < samples.length; i++) {
                var sample = Math.max(-1, Math.min(1, samples[i]));
                view.setInt16(offset, sample < 0 ? sample * 0x8000 : sample * 0x7FFF, true);
                offset += 2;
            }
            
            return buffer;
        }
    )");
    
    // Resampling function to convert to 16kHz
    setJavaScriptMember("resampleTo16kHz", R"(
        function(audioBuffer) {
            var originalSampleRate = audioBuffer.sampleRate;
            var targetSampleRate = 16000;
            var ratio = originalSampleRate / targetSampleRate;
            var newLength = Math.round(audioBuffer.length / ratio);
            var result = new Float32Array(newLength);
            
            // Get channel data (convert to mono if stereo)
            var channelData;
            if (audioBuffer.numberOfChannels === 1) {
                channelData = audioBuffer.getChannelData(0);
            } else {
                // Convert stereo to mono by averaging channels
                var left = audioBuffer.getChannelData(0);
                var right = audioBuffer.getChannelData(1);
                channelData = new Float32Array(audioBuffer.length);
                for (var i = 0; i < audioBuffer.length; i++) {
                    channelData[i] = (left[i] + right[i]) / 2;
                }
            }
            
            // Simple linear interpolation resampling
            for (var i = 0; i < newLength; i++) {
                var index = i * ratio;
                var indexInt = Math.floor(index);
                var indexFrac = index - indexInt;
                
                if (indexInt >= channelData.length - 1) {
                    result[i] = channelData[channelData.length - 1];
                } else {
                    result[i] = channelData[indexInt] * (1 - indexFrac) + 
                               channelData[indexInt + 1] * indexFrac;
                }
            }
            
            return result;
        }
    )");
    
    // Initialize function - check if Web Audio API is supported
    setJavaScriptMember("init", R"(
        function() {
            console.log('Initializing audio recording...');
            console.log('AudioContext available:', 'AudioContext' in window || 'webkitAudioContext' in window);
            console.log('getUserMedia available:', navigator.mediaDevices && 'getUserMedia' in navigator.mediaDevices);
            
            if (('AudioContext' in window || 'webkitAudioContext' in window) && 
                navigator.mediaDevices && 'getUserMedia' in navigator.mediaDevices) {
                this.isSupported = true;
                console.log('Audio recording supported');
            } else {
                console.log('Audio recording not supported');
                this.isSupported = false;
            }

            )" + js_signal_voice_recording_supported_.createCall({"this.isSupported"}) + R"(
        }
    )");

    // Async initialization function - doesn't block UI loading
    setJavaScriptMember("initAsync", R"(
        function() {
            console.log('Async initializing audio recording...');
            console.log('AudioContext available:', 'AudioContext' in window || 'webkitAudioContext' in window);
            console.log('getUserMedia available:', navigator.mediaDevices && 'getUserMedia' in navigator.mediaDevices);
            
            if (('AudioContext' in window || 'webkitAudioContext' in window) && 
                navigator.mediaDevices && 'getUserMedia' in navigator.mediaDevices) {
                this.isSupported = true;
                console.log('Audio recording supported');
            } else {
                console.log('Audio recording not supported');
                this.isSupported = false;
            }

            )" + js_signal_voice_recording_supported_.createCall({"this.isSupported"}) + R"(

            // Check microphone availability asynchronously
            console.log('Checking microphone availability...');
            navigator.mediaDevices.getUserMedia({ audio: true })
                .then(function(stream) {
                    console.log('Microphone is available');
                    )" + js_signal_microphone_avalable_.createCall({"true"}) + R"(
                    // Stop the stream immediately since we're just checking availability
                    stream.getTracks().forEach(track => track.stop());
                })
                .catch(function(error) {
                    console.error('Microphone access denied or not available:', error);
                    )" + js_signal_microphone_avalable_.createCall({"false"}) + R"(
                });
        }
    )");

    // Start recording function
    setJavaScriptMember("start", R"(
        function() {
            var self = this;
            
            console.log('Start function called, supported:', this.isSupported);
            
            if (!this.isSupported) {
                console.log('Audio recording not supported');
                return false;
            }
            
            // Initialize audio context
            try {
                var AudioContext = window.AudioContext || window.webkitAudioContext;
                self.audioContext = new AudioContext();
                console.log('AudioContext created, sample rate:', self.audioContext.sampleRate);
            } catch (e) {
                console.error('Failed to create AudioContext:', e);
                return false;
            }
            
            // Start audio recording
            navigator.mediaDevices.getUserMedia({ 
                audio: {
                    sampleRate: 44100,
                    channelCount: 1,
                    echoCancellation: true,
                    noiseSuppression: true,
                    autoGainControl: true
                }
            })
            .then(function(stream) {
                console.log('Microphone access granted');
                self.mediaStream = stream;
                self.recordedSamples = [];
                
                // Create audio nodes
                self.sourceNode = self.audioContext.createMediaStreamSource(stream);
                
                // Create ScriptProcessorNode for capturing audio data
                var bufferSize = 4096;
                self.processorNode = self.audioContext.createScriptProcessor(bufferSize, 1, 1);
                
                self.processorNode.onaudioprocess = function(event) {
                    var inputBuffer = event.inputBuffer;
                    var inputData = inputBuffer.getChannelData(0);
                    
                    // Copy the audio data
                    var samples = new Float32Array(inputData.length);
                    for (var i = 0; i < inputData.length; i++) {
                        samples[i] = inputData[i];
                    }
                    self.recordedSamples.push(samples);
                };
                
                // Connect the audio nodes
                self.sourceNode.connect(self.processorNode);
                self.processorNode.connect(self.audioContext.destination);
                
                console.log('Audio recording started with Web Audio API');
                console.log('Status: Recording audio... Speak now');
                return true;
            })
            .catch(function(error) {
                console.error('Error accessing microphone:', error);
                alert('Error Message: ' + error.message);
                return false;
            });
                
            return true;
        }
    )");

    // Stop recording function
    setJavaScriptMember("stop", R"(
        function() {
            var self = this;
            
            if (!self.audioContext || !self.mediaStream) {
                console.log('No active recording to stop');
                return false;
            }
            
            console.log('Stopping audio recording...');
            
            // Disconnect audio nodes
            if (self.sourceNode) {
                self.sourceNode.disconnect();
                self.sourceNode = null;
            }
            if (self.processorNode) {
                self.processorNode.disconnect();
                self.processorNode = null;
            }
            
            // Stop media stream
            self.mediaStream.getTracks().forEach(track => track.stop());
            self.mediaStream = null;
            
            // Process recorded audio data
            if (self.recordedSamples.length === 0) {
                console.log('No audio data recorded');
                return false;
            }
            
            console.log('Processing', self.recordedSamples.length, 'audio chunks...');
            
            // Concatenate all recorded samples
            var totalLength = 0;
            for (var i = 0; i < self.recordedSamples.length; i++) {
                totalLength += self.recordedSamples[i].length;
            }
            
            var concatenated = new Float32Array(totalLength);
            var offset = 0;
            for (var i = 0; i < self.recordedSamples.length; i++) {
                concatenated.set(self.recordedSamples[i], offset);
                offset += self.recordedSamples[i].length;
            }
            
            console.log('Total samples recorded:', concatenated.length);
            console.log('Original sample rate:', self.audioContext.sampleRate);
            
            // Create an audio buffer and resample to 16kHz
            var audioBuffer = self.audioContext.createBuffer(1, concatenated.length, self.audioContext.sampleRate);
            audioBuffer.getChannelData(0).set(concatenated);
            
            var resampledData = self.resampleTo16kHz(audioBuffer);
            console.log('Resampled to 16kHz, samples:', resampledData.length);
            console.log('Duration:', resampledData.length / 16000, 'seconds');
            
            // Encode as WAV
            var wavBuffer = self.encodeWAV(resampledData, 16000);
            self.recordedBlob = new Blob([wavBuffer], { type: 'audio/wav' });
            console.log('Created WAV blob:', self.recordedBlob.size, 'bytes');
            
            self.audioUrl = URL.createObjectURL(self.recordedBlob);
            console.log('Created audio URL:', self.audioUrl);
            
            // Update UI elements synchronously to ensure proper sequencing
            self.audioElement = document.getElementById(')" + audio_player_->id() + R"(');
            console.log('Audio element:', self.audioElement);

            if (self.audioElement) {
                self.audioElement.src = self.audioUrl;
                self.audioElement.load();
                console.log('Audio source set on WT audio widget:', self.audioUrl);
            } else {
                console.error('Audio element not found with ID: )" + audio_player_->id() + R"(');
            }
            
            // Set the recorded audio file to the file upload widget synchronously
            var fileUploadElement = document.getElementById(')" + file_upload_->id() + R"(');
            if (fileUploadElement) {
                var fileInput = fileUploadElement.querySelector('input[type="file"]');
                if (fileInput) {
                    // Create a File object from the WAV blob
                    var audioFile = new File([self.recordedBlob], 'recorded_audio_16khz_mono.wav', { 
                        type: 'audio/wav',
                        lastModified: Date.now()
                    });
                    
                    // Create a DataTransfer object to set the file
                    var dataTransfer = new DataTransfer();
                    dataTransfer.items.add(audioFile);
                    fileInput.files = dataTransfer.files;
                    
                    // Trigger the change event to notify WT
                    var changeEvent = new Event('change', { bubbles: true });
                    fileInput.dispatchEvent(changeEvent);
                    
                    console.log('16kHz WAV file set to upload widget:', audioFile.name, audioFile.size, 'bytes');
                    )" + js_signal_audio_widget_has_media_.createCall({"true"}) + R"(
                
                } else {
                    console.error('File input element not found in:', fileUploadElement);
                    )" + js_signal_audio_widget_has_media_.createCall({"false"}) + R"(
                }
            } else {
                console.error('File upload element not found with ID: )" + file_upload_->id() + R"(');
            }
            
            // Close audio context
            if (self.audioContext) {
                self.audioContext.close();
                self.audioContext = null;
            }
            
            console.log('Audio recording stopped and processed');
            return true;
        }
    )");
}

void VoiceRecorder::transcribeCurrentAudio()
{
    if (current_audio_file_.empty()) {
        transcription_display_->setText("No audio file to transcribe");
        return;
    }
    
    // Check if transcription is already in progress
    if (transcription_in_progress_) {
        std::cout << "Transcription already in progress, ignoring second request" << std::endl;
        return;
    }
    
    // Log the file being transcribed for debugging
    std::cout << "Starting transcription for file: " << current_audio_file_ << std::endl;
    
    // Show loading message in transcription area
    transcription_display_->setText("⏳ Transcribing audio, please wait...");
    
    // Mark transcription as in progress
    transcription_in_progress_ = true;
    
    // Get the application instance to enable server push
    Wt::WApplication* app = Wt::WApplication::instance();
    app->enableUpdates(true);
    
    // Capture the current audio file path to avoid race conditions
    std::string audio_file_to_transcribe = current_audio_file_;
    
    // Create and detach a new thread for each transcription
    // No need to manage thread lifetime - it cleans up automatically
    std::thread([this, app, audio_file_to_transcribe]() {
        this->performTranscriptionInBackground(app, audio_file_to_transcribe);
    }).detach();
}

void VoiceRecorder::performTranscriptionInBackground(Wt::WApplication* app, const std::string& audio_file_path)
{
    std::string transcription_result;
    std::string error_message;
    
    std::cout << "Background transcription started for: " << audio_file_path << std::endl;
    
    try {
        // Create WhisperCliService only when needed
        auto whisper_client = std::make_unique<WhisperCliService>();
        
        // Build paths relative to the current working directory (build/release or build/debug)
        std::string whisper_executable_path = "./whisper_service";  // In current build directory
        std::string model_path = "/apps/cv/models/ggml-base.en.bin";   // Absolute path for Docker container
        
        if (!whisper_client->initialize(whisper_executable_path, model_path)) {
            error_message = "Failed to initialize Whisper service: " + whisper_client->getLastError();
        } else {
            std::cout << "WhisperCliService initialized successfully" << std::endl;
            
            // Use the synchronous API to transcribe the specific file
            transcription_result = whisper_client->transcribeFile(audio_file_path);
            
            // Check if the result indicates an error
            if (transcription_result.find("ERROR:") == 0) {
                error_message = transcription_result.substr(6); // Remove "ERROR:" prefix
                transcription_result.clear();
            }
        }
    } catch (const std::exception& e) {
        error_message = "Exception during transcription: " + std::string(e.what());
    }
    
    // Update the UI thread-safely using the application update lock
    Wt::WApplication::UpdateLock uiLock(app);
    if (uiLock) {
        if (!transcription_result.empty()) {
            // Success - only show the transcription text, no status messages
            current_transcription_ = transcription_result;
            transcription_display_->setText(transcription_result);
            status_text_->setText("Transcription complete ✓");
            
            // Emit signal for external handlers
            transcription_complete_.emit(transcription_result);
            
            std::cout << "Transcription completed: " << transcription_result << std::endl;
        } else {
            // Error
            std::string error_text = "Transcription failed";
            if (!error_message.empty()) {
                error_text += ": " + error_message;
            }
            
            transcription_display_->setText(error_text);
            
            std::cout << "Transcription failed: " << error_message << std::endl;
        }
        
        // Mark transcription as completed
        transcription_in_progress_ = false;
        
        // Trigger UI update
        app->triggerUpdate();
        
        // Disable server push when done
        app->enableUpdates(false);
    }
    
    // Thread will be automatically cleaned up when this function exits
}

std::string VoiceRecorder::getTranscription() const
{
    return current_transcription_;
}

std::string VoiceRecorder::createAudioFilesDirectory()
{
    try {
        // Get the document root from Wt application
        std::string docRoot = Wt::WApplication::instance()->docRoot();
        std::string audioDir = docRoot + "/audio-files";
        
        // Create directory if it doesn't exist
        if (!std::filesystem::exists(audioDir)) {
            std::filesystem::create_directories(audioDir);
            std::cout << "Created audio-files directory: " << audioDir << std::endl;
        }
        
        return audioDir;
    } catch (const std::exception& e) {
        std::cerr << "Error creating audio-files directory: " << e.what() << std::endl;
        return "";
    }
}

std::string VoiceRecorder::generateUniqueFileName(const std::string& originalName)
{
    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
    ss << "_" << std::setfill('0') << std::setw(3) << ms.count();
    
    // Extract file extension
    std::string extension = "";
    size_t dotPos = originalName.find_last_of('.');
    if (dotPos != std::string::npos) {
        extension = originalName.substr(dotPos);
    } else {
        extension = ".webm"; // Default for recorded audio
    }
    
    return "audio_" + ss.str() + extension;
}

bool VoiceRecorder::saveAudioFile(const std::string& tempPath, const std::string& permanentPath)
{
    try {
        std::ifstream src(tempPath, std::ios::binary);
        if (!src.is_open()) {
            std::cerr << "Cannot open source file: " << tempPath << std::endl;
            return false;
        }
        
        std::ofstream dst(permanentPath, std::ios::binary);
        if (!dst.is_open()) {
            std::cerr << "Cannot create destination file: " << permanentPath << std::endl;
            src.close();
            return false;
        }
        
        // Copy file contents
        dst << src.rdbuf();
        
        src.close();
        dst.close();
        
        // Verify the file was created successfully
        if (std::filesystem::exists(permanentPath)) {
            std::cout << "Successfully saved audio file: " << permanentPath << std::endl;
            return true;
        } else {
            std::cerr << "File save verification failed: " << permanentPath << std::endl;
            return false;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Exception while saving audio file: " << e.what() << std::endl;
        return false;
    }
}

void VoiceRecorder::updateRecordingTimer()
{
    if (is_recording_) {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - recording_start_time_);
        int seconds = duration.count();
        
        std::string time_text = formatRecordingTime(seconds);
        play_pause_btn_->setText(time_text);
    }
}

std::string VoiceRecorder::formatRecordingTime(int seconds)
{
    return std::to_string(seconds);
}

