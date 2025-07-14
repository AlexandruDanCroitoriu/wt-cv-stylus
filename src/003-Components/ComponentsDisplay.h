#pragma once
#include <Wt/WContainerWidget.h>

enum ButtonSize
{
    XS,
    SM,
    MD,
    LG,
    XL,
};

class ComponentsDisplay : public Wt::WContainerWidget
{
public:
    ComponentsDisplay();
    
    void createButtons();
    void createMonacoEditor();
    void createVoiceRecorder();



private:
    void setCopyToClipboardAction(Wt::WInteractWidget *widget, Wt::WComboBox* combo_box, const std::string &text_start, const std::string &text_end);
    ButtonSize selected_size_ = ButtonSize::XS;

};