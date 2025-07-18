#pragma once
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>
#include <Wt/WPushButton.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WTemplate.h>
#include "004-Dbo/Session.h"
#include <Wt/WAudio.h>
#include <Wt/WFileUpload.h>
#include <Wt/WSignal.h>
#include <Wt/WTimer.h>
#include <memory>
#include <thread>
#include <chrono>

class Button; // Forward declaration
class WhisperServiceClient; // Forward declaration


class VoiceRecorder : public Wt::WContainerWidget
{
public:
    VoiceRecorder();
    ~VoiceRecorder(); // Need explicit destructor to handle thread cleanup

    void disable();
    void enable();
    
    // Transcription methods
    void transcribeCurrentAudio();
    std::string getTranscription() const;
    
    // Signal for when transcription is complete
    Wt::Signal<std::string>& transcriptionComplete() { return transcription_complete_; }

private:
    void setupUI();
    void setupJavaScriptRecorder();
    void startRecording();
    void stopRecording();
    void onFileUploaded();
    void onFileTooLarge();
    void uploadFile();
    void performTranscriptionInBackground(Wt::WApplication* app, const std::string& audio_file_path);
    
    // Audio file management
    std::string createAudioFilesDirectory();
    std::string generateUniqueFileName(const std::string& originalName);
    bool saveAudioFile(const std::string& tempPath, const std::string& permanentPath);
    
    // Timer management
    void updateRecordingTimer();
    std::string formatRecordingTime(int seconds);

    Wt::WText* status_text_;
    Wt::WAudio* audio_player_;
    Wt::WFileUpload* file_upload_;
    Button* play_pause_btn_;
    
    Wt::WContainerWidget* recording_info_;
    Wt::WTextArea* transcription_display_;

    bool is_recording_;
    std::unique_ptr<Wt::WTimer> recording_timer_;
    std::chrono::steady_clock::time_point recording_start_time_;
    std::string microphone_svg_;
    Wt::JSignal<bool> js_signal_voice_recording_supported_;
    Wt::JSignal<bool> js_signal_microphone_avalable_;
    Wt::JSignal<bool> js_signal_audio_widget_has_media_;
    bool is_audio_supported_;
    bool is_microphone_available_;
    bool is_enabled_;
    
    // Transcription data
    std::string current_transcription_;
    std::string current_audio_file_;
    Wt::Signal<std::string> transcription_complete_;
    
    // Simple flag to prevent multiple simultaneous transcriptions
    bool transcription_in_progress_;

};
