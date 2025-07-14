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


class VoiceRecorder : public Wt::WContainerWidget
{
public:
    VoiceRecorder();

private:
    void setupUI();
    void startRecording();
    void stopRecording();
    void onFileUploaded();
    void onFileTooLarge();
    void uploadFile();
    
    Wt::WText* status_text_;
    Wt::WAudio* audio_player_;
    Wt::WFileUpload* file_upload_;
    Wt::WPushButton* play_pause_btn_;
    Wt::WPushButton* upload_btn_;
    Wt::WPushButton* clear_btn_;
    
    Wt::WContainerWidget* recording_info_;
    bool is_recording_;
    std::string js_recorder_referance_;
    Wt::JSignal<bool> js_signal_voice_recording_supported_;
    bool is_supported_ = false;

};
