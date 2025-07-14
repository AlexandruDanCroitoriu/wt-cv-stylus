#include "010-VoiceRecorder/VoiceRecorder.h"
#include "003-Components/Button.h"
#include <Wt/WApplication.h>
#include <Wt/WJavaScript.h>
#include <Wt/WText.h>
#include <iostream>

VoiceRecorder::VoiceRecorder() : is_recording_(false), js_signal_voice_recording_supported_(this, "voiceRecordingSupported")
{
    addStyleClass("space-y-2 flex items-center border relative rounded-radius m-5");
    js_recorder_referance_ = "window.voiceRecorder" + id() + "Widget";
    setupUI();
    js_signal_voice_recording_supported_.connect(this, [=](bool is_supported){ is_supported_ = is_supported; });
}

void VoiceRecorder::setupUI()
{
    // Status indicator
    status_text_ = addNew<Wt::WText>("Ready to record audio");
    status_text_->addStyleClass("text-lg text-on-surface-variant absolute -top-4 bg-surface");

    // Start recording button
    play_pause_btn_ = addNew<Button>(std::string(Wt::WString::tr("app:microphone-svg").toUTF8()), "m-1.5 text-xs ", PenguinUiWidgetTheme::BtnSuccessAction);
    play_pause_btn_->clicked().connect(this, [=]
    {
        if(!is_recording_) {
            startRecording();
        }else {
            stopRecording();
        }
    });

    audio_player_ = addNew<Wt::WAudio>();
    audio_player_->setOptions(Wt::PlayerOption::Controls);
    audio_player_->setStyleClass("w-full mt-2");
    audio_player_->setAlternativeContent(std::make_unique<Wt::WText>("You have no HTML5 Audio!"));

    // Upload button
    upload_btn_ = addNew<Button>(std::string(Wt::WString::tr("app:download-svg").toUTF8()), "m-1.5 text-xs ", PenguinUiWidgetTheme::BtnSuccessAction);
    upload_btn_->clicked().connect(this, &VoiceRecorder::uploadFile);

     
    file_upload_ = addNew<Wt::WFileUpload>();
    file_upload_->setStyleClass("bg-white mb-2 w-full");
    // file_upload_->hide();

    // Connect file upload signal
    file_upload_->uploaded().connect(this, &VoiceRecorder::onFileUploaded);
    file_upload_->fileTooLarge().connect(this, &VoiceRecorder::onFileTooLarge);
    {
    // Add Audio Recording JavaScript
    std::string jsCode = R"(
        if()" + js_recorder_referance_ + R"() {
            console.warn('VoiceRecorder widget already initialized');
            return;
        }
        )" + js_recorder_referance_ + R"( = {
            mediaRecorder: null,
            audioChunks: [],
            recordedBlob: null,
            audioUrl: null,
            isSupported: false,
            audioElement: null,
            
            init: function() {
                // Check if MediaRecorder is supported
                console.log('Initializing audio recording...');
                console.log('MediaRecorder available:', 'MediaRecorder' in window);
                console.log('getUserMedia available:', navigator.mediaDevices && 'getUserMedia' in navigator.mediaDevices);
                
                if ('MediaRecorder' in window && navigator.mediaDevices && 'getUserMedia' in navigator.mediaDevices) {
                    this.isSupported = true;
                    console.log('Audio recording supported');
                } else {
                    console.log('Audio recording not supported');
                    this.isSupported = false;
                    return false;
                }
            },
            
            start: function() {
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
                            self.audioElement = document.getElementById(')" + audio_player_->id() + R"(');
                            console.log('Audio element:', self.audioElement);
               
                            self.audioElement.src = self.audioUrl;
                            self.audioElement.load(); // Force reload the audio element
                            console.log('Audio source set on WT audio widget:', self.audioUrl);
                            console.log('Audio element after setting source:', self.audioElement);
                            
                            // Set the recorded audio file to the file upload widget
                            var fileInput = document.getElementById(')" + file_upload_->id() + R"(').querySelector('input[type="file"]');
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
                            } else {
                                console.error('File input element not found');
                            }
                            
                            // Stop the stream
                            stream.getTracks().forEach(track => track.stop());
                        };
                        
                        self.mediaRecorder.start(1000); // Record in 1-second chunks
                        console.log('Audio recording started with timeslice');
                        console.log('Status: Recording audio... Speak now');
                    })
                    .catch(function(error) {
                        console.error('Error accessing microphone:', error);
                        alert('Error Message: ' + error.message);
                        return false;
                    });
                    
                return true;
            },
            
            stop: function() {
                // Stop audio recording
                if (this.mediaRecorder && this.mediaRecorder.state === 'recording') {
                    this.mediaRecorder.stop();
                    console.log('Audio recording stopped');
                    console.log('Status: Processing audio...');
                    return true;
                }
                return false;
            },
        };
        
        // Initialize when page loads
        if (document.readyState === 'loading') {
            document.addEventListener('DOMContentLoaded', function() {
                setTimeout(function() {
                    )" + js_recorder_referance_ + R"(.init();
                }, 100); // Small delay to ensure WT widgets are fully rendered
            });
        } else {
            setTimeout(function() {
                )" + js_recorder_referance_ + R"(.init();
            }, 100); // Small delay to ensure WT widgets are fully rendered
        }
    )";
    doJavaScript(jsCode);
    }
    
    // wApp->doJavaScript(jsCode);
}

void VoiceRecorder::startRecording()
{
    if (!is_recording_) {
        wApp->doJavaScript(R"(
            console.log('Starting audio recording...');
            console.log('Audio recording supported:', )" + js_recorder_referance_ + R"(.isSupported);
            
            if (!)" + js_recorder_referance_ + R"(.isSupported) {
                var message = 'Audio recording is not available. ';
                message += 'Please try:\n';
                message += '1. Using a modern browser (Chrome, Firefox, Edge)\n';
                message += '2. Checking microphone permissions\n';
                message += '3. Making sure your microphone is connected';
                
                alert(message);
                return;
            }
            
            if ()" + js_recorder_referance_ + R"(.start()) {
                console.log('Audio recording started successfully');
            } else {
                alert('Failed to start recording. Please check microphone permissions.');
            }
        )");
        
        is_recording_ = true;
        // play_pause_btn_->setEnabled(false);
        // stop_btn_->setEnabled(true);
        play_pause_btn_->toggleStyleClass("animate-pulse", true);
        status_text_->setText("Recording audio... Speak now");
    }
}

void VoiceRecorder::stopRecording()
{
    if (is_recording_) {
        wApp->doJavaScript(js_recorder_referance_ + ".stop();");
        
        is_recording_ = false;
        play_pause_btn_->toggleStyleClass("animate-pulse", false);

        // play_pause_btn_->setEnabled(true);
        // stop_btn_->setEnabled(false);
        status_text_->setText("Audio recording stopped");
    }
}


void VoiceRecorder::onFileUploaded()
{
    std::cout << "File uploaded successfully." << std::endl;
    std::string fileName = file_upload_->spoolFileName();
    std::string clientFileName = file_upload_->clientFileName().toUTF8();
    
    if (!fileName.empty()) {
        status_text_->setText("Audio file uploaded: " + clientFileName);
        
        std::cout << "Audio file uploaded: " << clientFileName 
                  << " (temp file: " << fileName << ")" << std::endl;
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
