#pragma once

#include "999-Stylus/000-Utils/StylusPanelWrapper.h"
#include "999-Stylus/000-Utils/StylusState.h"
#include "003-Components/MonacoEditor.h"
#include "999-Stylus/000-Utils/FileExplorerTree.h"
#include "003-Components/DragBar.h"

namespace Stylus {

class TailwindCss : public StylusPanelWrapper
{
public:
    TailwindCss(std::shared_ptr<StylusState> state);

    Wt::Signal<std::string>& folders_changed() { return folders_changed_; }

private:
    FileExplorerTree* file_explorer_tree_;
    MonacoEditor* monaco_editor_;
    DragBar* drag_bar_;
    std::vector<std::string> getConfigFiles();
    void generateCssFile();

    std::string css_selected_file_path_;

    Wt::WString current_css_file_path_;
    Wt::WString prev_css_file_path_;
    Wt::WComboBox* config_files_combobox_;

    Wt::Signal<std::string> folders_changed_; // used mostly by xml files manager to recreate xml file brains

};
}