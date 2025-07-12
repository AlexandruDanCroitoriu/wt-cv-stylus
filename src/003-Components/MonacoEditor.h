#pragma once
#include <Wt/WContainerWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/WStringStream.h>
#include <Wt/WSignal.h>


    
class MonacoEditor : public Wt::WContainerWidget
{
    public:
        MonacoEditor(std::string language);
        void setReadOnly(bool read_only);
        
        bool unsavedChanges();
        std::string getUnsavedText() { return unsaved_text_; }
        void textSaved(); // used to fire the avalable_save to false and set the current text to unsaved text
        void setEditorText(std::string resource_path);
        
        Wt::Signal<std::string>& save_file_signal() { return save_file_signal_; }
        Wt::Signal<>& avalable_save() { return avalable_save_; }
        Wt::Signal<Wt::WString>& width_changed() { return width_changed_; }
        
        void toggleLineWrap();
        void toggleMinimap();
        static void setDarkTheme(bool dark);
        static std::string getFileText(std::string file_path);

        void SaveFile();

        void resetLayout();
    protected:
        // Custom implementation
        void layoutSizeChanged(int width, int height) override;
        
    private:
        void editorTextChanged(std::string text);
        std::string selected_file_path_;

        Wt::JSignal<std::string> js_signal_text_changed_;
        Wt::Signal<> avalable_save_;
        Wt::Signal<std::string> save_file_signal_;
        
        std::string current_text_;
        std::string unsaved_text_;
        std::string editor_js_var_name_;
        Wt::Signal<Wt::WString> width_changed_;
    
};
