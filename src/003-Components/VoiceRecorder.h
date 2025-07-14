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

class Button; // Forward declaration


class VoiceRecorder : public Wt::WContainerWidget
{
public:
    VoiceRecorder();

    void disable();
    void enable();

private:
    void setupUI();
    void setupJavaScriptRecorder();
    void startRecording();
    void stopRecording();
    void onFileUploaded();
    void onFileTooLarge();
    void uploadFile();

    Wt::WText* status_text_;
    Wt::WAudio* audio_player_;
    Wt::WFileUpload* file_upload_;
    Button* play_pause_btn_;
    Button* upload_btn_;
    
    Wt::WContainerWidget* recording_info_;

    bool is_recording_;
    Wt::JSignal<bool> js_signal_voice_recording_supported_;
    Wt::JSignal<bool> js_signal_microphone_avalable_;
    bool is_audio_supported_;
    bool is_microphone_available_;
    bool is_enabled_;

};
