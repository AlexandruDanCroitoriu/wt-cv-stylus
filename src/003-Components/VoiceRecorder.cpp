#include "003-Components/VoiceRecorder.h"
#include "003-Components/Button.h"
#include "003-Components/WhisperWrapper.h"
#include <Wt/WApplication.h>
#include <Wt/WJavaScript.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

VoiceRecorder::VoiceRecorder() 
    : is_recording_(false), 
    js_signal_voice_recording_supported_(this, "voiceRecordingSupported"),
    js_signal_microphone_avalable_(this, "microphoneAvailable"),
    js_signal_audio_widget_has_media_(this, "audioWidgetHasMedia"),
    is_audio_supported_(false),
    is_microphone_available_(false),
    is_enabled_(true),
    whisper_(std::make_unique<WhisperWrapper>())
{
    // setStyleClass("space-y-2 flex items-center border relative rounded-radius m-5");
    
    js_signal_voice_recording_supported_.connect(this, [=](bool is_supported){ 
        std::cout << "Voice recording support status: " << (is_supported ? "Supported" : "Not Supported") << std::endl;
        is_audio_supported_ = is_supported; 
            if (is_supported) {
                recording_info_->addNew<Wt::WText>("Audio recording is supported in your browser.")->setStyleClass("text-green-500");
                enable();
            }else {
                Wt::WString temp = R"(
                    <div class='bg-surface text-red-500 p-2 rounded-radius border border-outline'>Audio recording is not supported in your browser.</div>
                    <div class='text-sm'>Using a modern browser (Chrome, Firefox, Edge) is recommended.</div>
                    <div class='text-sm'>Check your browser's console for more details.</div>
                    <div class='text-sm'>Make sure your microphone is connected.</div>
                    )";
                recording_info_->addNew<Wt::WTemplate>(temp);
                disable();
        }
    });
    js_signal_microphone_avalable_.connect(this, [=](bool is_available){
        std::cout << "Microphone availability status: " << (is_available ? "Available" : "Not Available") << std::endl;
        is_microphone_available_ = is_available;
        if(is_available) {
            recording_info_->addNew<Wt::WText>("Microphone is available")->setStyleClass("text-green-500");
            enable();
        } else {
            recording_info_->addNew<Wt::WText>("No microphone detected")->setStyleClass("text-red-500");

            disable();
        }
    });
    js_signal_audio_widget_has_media_.connect(this, [=](bool has_media){
        std::cout << "Audio widget media status: " << (has_media ? "Has Media" : "No Media") << std::endl;
        if (has_media) {
            uploadFile();
            transcribe_btn_->enable();
        } else {
            transcribe_btn_->disable();
        }
    });

    setupUI();
    setupJavaScriptRecorder();
    initializeWhisper();
    // Initialize the recorder when the DOM is ready
    doJavaScript("setTimeout(function() { if (" + jsRef() + ") { " + jsRef() + ".init(); } }, 200);");
}

VoiceRecorder::~VoiceRecorder() = default;

void VoiceRecorder::setupUI()
{
    clear();
    
    auto widget_wrapper = addNew<Wt::WContainerWidget>();
    widget_wrapper->setStyleClass("space-y-2 pt-2 flex items-center border border-outline relative rounded-radius m-5 relative w-[500px]");

    auto debug_wrapper = addNew<Wt::WContainerWidget>();
    debug_wrapper->setStyleClass("space-y-2 flex items-center");
    
    // Status indicator
    status_text_ = widget_wrapper->addNew<Wt::WText>("Ready to record audio");
    status_text_->addStyleClass("text-lg text-on-surface-variant absolute -top-4 left-3 bg-surface");

    // Start recording button
    play_pause_btn_ = widget_wrapper->addNew<Button>(std::string(Wt::WString::tr("app:microphone-svg").toUTF8()), "m-1.5 text-xs ", PenguinUiWidgetTheme::BtnSuccessAction);
    play_pause_btn_->clicked().connect(this, [=]
    {
        if(!is_recording_) {
            startRecording();
        }else {
            stopRecording();
        }
    });

    audio_player_ = widget_wrapper->addNew<Wt::WAudio>();
    audio_player_->setOptions(Wt::PlayerOption::Controls);
    audio_player_->setStyleClass("grow w-full p-2");
    audio_player_->setAlternativeContent(std::make_unique<Wt::WText>("You have no HTML5 Audio!"));

    // Upload button
    upload_btn_ = widget_wrapper->addNew<Button>(std::string(Wt::WString::tr("app:download-svg").toUTF8()), "m-1.5 text-xs ", PenguinUiWidgetTheme::BtnSuccessAction);
    upload_btn_->clicked().connect(this, &VoiceRecorder::uploadFile);
    upload_btn_->disable();
    
    // Transcribe button
    transcribe_btn_ = widget_wrapper->addNew<Button>("🎤→📝", "m-1.5 text-xs ", PenguinUiWidgetTheme::BtnPrimaryAction);
    transcribe_btn_->clicked().connect(this, &VoiceRecorder::transcribeCurrentAudio);
    transcribe_btn_->disable();
    
    recording_info_ = debug_wrapper->addNew<Wt::WContainerWidget>();
    recording_info_->setStyleClass("flex flex-col space-y-2");
    
    // Transcription display area
    transcription_display_ = debug_wrapper->addNew<Wt::WTextArea>();
    transcription_display_->setStyleClass("w-full h-32 p-2 border border-outline rounded-radius bg-surface text-on-surface");
    transcription_display_->setPlaceholderText("Transcribed text will appear here...");
    transcription_display_->setReadOnly(true);
     
    file_upload_ = debug_wrapper->addNew<Wt::WFileUpload>();
    file_upload_->setStyleClass("bg-white mb-2");
    // file_upload_->hide();

    // Connect file upload signal
    file_upload_->uploaded().connect(this, &VoiceRecorder::onFileUploaded);
    file_upload_->fileTooLarge().connect(this, &VoiceRecorder::onFileTooLarge);
    

}

void VoiceRecorder::startRecording()
{
    if (!is_recording_) {        
        // Call the start member function
        callJavaScriptMember("start", "");
        
        is_recording_ = true;
        play_pause_btn_->toggleStyleClass("animate-pulse", true);
        status_text_->setText("Recording audio... Speak now");
        upload_btn_->disable();
    }
}

void VoiceRecorder::stopRecording()
{
    if (is_recording_) {
        // Call the stop member function
        callJavaScriptMember("stop", "");
        is_recording_ = false;
        play_pause_btn_->toggleStyleClass("animate-pulse", false);
        status_text_->setText("Audio recording stopped");
        upload_btn_->enable();
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
            // Generate unique filename to avoid conflicts
            std::string uniqueFileName = generateUniqueFileName(clientFileName);
            std::string permanentPath = audioDir + "/" + uniqueFileName;
            
            // Save the uploaded file to the permanent location
            if (saveAudioFile(tempFileName, permanentPath)) {
                current_audio_file_ = permanentPath;
                transcribe_btn_->enable();
                status_text_->setText("Audio file saved: " + uniqueFileName);
                std::cout << "Audio file saved: " << permanentPath << std::endl;
                // Trigger transcription after upload
                transcription_display_->setText("Transcribing audio...");
                current_transcription_ = whisper_->transcribeFile(current_audio_file_);
                transcription_display_->setText(current_transcription_);
                transcription_complete_.emit(current_transcription_);
            } else {
                status_text_->setText("Error: Failed to save audio file");
                std::cout << "Failed to save audio file to: " << permanentPath << std::endl;
            }
        } else {
            status_text_->setText("Error: Could not create audio-files directory");
            // Fallback to using temp file
            current_audio_file_ = tempFileName;
            transcribe_btn_->enable();
        }
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
    // upload_btn_->enable();
}

void VoiceRecorder::disable()
{
    is_enabled_ = false;
    audio_player_->disable();
    play_pause_btn_->disable();
    transcribe_btn_->disable();
    // upload_btn_->disable();
}


void VoiceRecorder::setupJavaScriptRecorder()
{
    // Initialize the JavaScript object for media recording
    setJavaScriptMember("mediaRecorder", "null");
    setJavaScriptMember("audioChunks", "[]");
    setJavaScriptMember("recordedBlob", "null");
    setJavaScriptMember("audioUrl", "null");
    setJavaScriptMember("isSupported", "false");
    setJavaScriptMember("audioElement", "null");
    
    // Initialize function - check if MediaRecorder is supported
    setJavaScriptMember("init", R"(
        function() {
            console.log('Initializing audio recording...');
            console.log('MediaRecorder available:', 'MediaRecorder' in window);
            console.log('getUserMedia available:', navigator.mediaDevices && 'getUserMedia' in navigator.mediaDevices);
            
            if ('MediaRecorder' in window && navigator.mediaDevices && 'getUserMedia' in navigator.mediaDevices) {
                this.isSupported = true;
                console.log('Audio recording supported');
            } else {
                console.log('Audio recording not supported');
                this.isSupported = false;
            }

            )" + js_signal_voice_recording_supported_.createCall({"this.isSupported"}) + R"(

            // Check microphone availability
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
            
            // Start audio recording
            navigator.mediaDevices.getUserMedia({ audio: true })
                .then(function(stream) {
                    console.log('Microphone access granted');
                    self.mediaRecorder = new MediaRecorder(stream);
                    self.audioChunks = [];
                    
                    self.mediaRecorder.ondataavailable = function(event) {
                        console.log('Data available:', event.data.size, 'bytes');
                        if (event.data.size > 0) {
                            self.audioChunks.push(event.data);
                        }
                    };
                    
                    self.mediaRecorder.onstop = function() {
                        console.log('MediaRecorder stopped, processing audio...');
                        self.recordedBlob = new Blob(self.audioChunks, { type: 'audio/webm' });
                        console.log('Created blob:', self.recordedBlob.size, 'bytes');
                        self.audioUrl = URL.createObjectURL(self.recordedBlob);
                        console.log('Created audio URL:', self.audioUrl);
                        
                        // Use setTimeout to ensure DOM is ready
                        setTimeout(function() {
                            self.audioElement = document.getElementById(')" + audio_player_->id() + R"(');
                            console.log('Audio element:', self.audioElement);
               
                            if (self.audioElement) {
                                self.audioElement.src = self.audioUrl;
                                self.audioElement.load(); // Force reload the audio element
                                console.log('Audio source set on WT audio widget:', self.audioUrl);
                            } else {
                                console.error('Audio element not found with ID: )" + audio_player_->id() + R"(');
                            }
                            
                            // Set the recorded audio file to the file upload widget
                            var fileUploadElement = document.getElementById(')" + file_upload_->id() + R"(');
                            if (fileUploadElement) {
                                var fileInput = fileUploadElement.querySelector('input[type="file"]');
                                if (fileInput) {
                                    // Create a File object from the blob
                                    var audioFile = new File([self.recordedBlob], 'recorded_audio.webm', { 
                                        type: 'audio/webm',
                                        lastModified: Date.now()
                                    });
                                    
                                    // Create a DataTransfer object to set the file
                                    var dataTransfer = new DataTransfer();
                                    dataTransfer.items.add(audioFile);
                                    fileInput.files = dataTransfer.files;
                                    
                                    // Trigger the change event to notify WT
                                    var changeEvent = new Event('change', { bubbles: true });
                                    fileInput.dispatchEvent(changeEvent);
                                    
                                    console.log('Audio file set to file upload widget:', audioFile.name, audioFile.size, 'bytes');
                                    )" + js_signal_audio_widget_has_media_.createCall({"true"}) + R"(
                                
                                } else {
                                    console.error('File input element not found in:', fileUploadElement);
                                    )" + js_signal_audio_widget_has_media_.createCall({"false"}) + R"(
                                }
                            } else {
                                console.error('File upload element not found with ID: )" + file_upload_->id() + R"(');
                            }
                        }, 100);
                        
                        // Stop the stream
                        stream.getTracks().forEach(track => track.stop());
                    };
                    
                    self.mediaRecorder.start(100); // Record in 1-second chunks
                    console.log('Audio recording started with timeslice');
                    console.log('Status: Recording audio... Speak now');
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
            // Stop audio recording
            if (this.mediaRecorder && this.mediaRecorder.state === 'recording') {
                this.mediaRecorder.stop();
                console.log('Audio recording stopped');
                console.log('Status: Processing audio...');
                return true;
            }
            return false;
        }
    )");

   
}

void VoiceRecorder::initializeWhisper()
{
    if (whisper_->initialize()) {
        std::cout << "Whisper initialized with ggml-base.en.bin model" << std::endl;
        recording_info_->addNew<Wt::WText>("Speech-to-text ready ✓")->setStyleClass("text-green-500");
    } else {
        std::cout << "Failed to initialize Whisper: " << whisper_->getLastError() << std::endl;
        recording_info_->addNew<Wt::WText>("Whisper initialization failed. Transcription disabled.")->setStyleClass("text-red-500");
    }
}

void VoiceRecorder::transcribeCurrentAudio()
{
    if (!whisper_->isInitialized()) {
        status_text_->setText("Whisper not initialized");
        return;
    }
    
    if (current_audio_file_.empty()) {
        status_text_->setText("No audio file to transcribe");
        return;
    }
    
    status_text_->setText("Transcribing audio...");
    transcribe_btn_->disable();
    
    // Perform transcription in a separate thread to avoid blocking UI
    // For now, doing it synchronously for simplicity
    performTranscription();
}

void VoiceRecorder::performTranscription()
{
    std::string transcription = whisper_->transcribeFile(current_audio_file_);
    
    if (!transcription.empty()) {
        current_transcription_ = transcription;
        transcription_display_->setText(transcription);
        
        // Check if a converted WAV file was created
        std::string wav_file = current_audio_file_;
        size_t lastDot = wav_file.find_last_of('.');
        if (lastDot != std::string::npos) {
            wav_file = wav_file.substr(0, lastDot) + "_converted.wav";
        } else {
            wav_file += "_converted.wav";
        }
        
        if (std::filesystem::exists(wav_file)) {
            status_text_->setText("Transcription complete (WAV file saved)");
            std::cout << "Converted WAV file saved: " << wav_file << std::endl;
        } else {
            status_text_->setText("Transcription complete");
        }
        
        // Emit signal for external handlers
        transcription_complete_.emit(transcription);
        
        std::cout << "Transcription: " << transcription << std::endl;
    } else {
        status_text_->setText("Transcription failed: " + whisper_->getLastError());
        std::cout << "Transcription failed: " << whisper_->getLastError() << std::endl;
    }
    
    transcribe_btn_->enable();
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

