#include "999-Stylus/001-TailwindCss/TailwindCss.h"
#include "003-Components/DragBar.h"
#include "001-App/App.h"
#include "002-Theme/Theme.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <array>
#include <Wt/WRandom.h>
#include <Wt/WServer.h>
#include <Wt/WIOService.h>

namespace Stylus
{

    TailwindCss::TailwindCss(std::shared_ptr<StylusState> state)
        : StylusPanelWrapper(state), 
        css_selected_file_path_(state->css_node_->Attribute("selected-file-path"))
    {
        addStyleClass("");
        auto theme_ptr = wApp->theme();
        if (theme_ptr)
        {
            auto theme = dynamic_cast<Theme *>(theme_ptr.get());
            if (theme)
            {
                current_css_file_path_ = theme->current_tailwind_file_path_;
            }
            else
            {
                std::cout << "TailwindCss: Theme cast failed" << std::endl;
            }
        }
        else
        {
            std::cout << "TailwindCss: Theme is null" << std::endl;
            // generateCssFile();
            file_explorer_tree_ = addWidget(std::make_unique<FileExplorerTree>(state, state->css_editor_data_));

            folders_changed_.connect(this, [=](std::string selected_file_path)
                {
                    file_explorer_tree_->setTreeFolderWidgets();
                });
        }
        std::cout << "\n\nTailwindCss initialized with current CSS file path: " << current_css_file_path_.toUTF8() << "\n\n";
        generateCssFile();
        
        // Set up flexbox container
        addStyleClass("flex h-screen");
        
        // Create child widgets
        file_explorer_tree_ = addNew<FileExplorerTree>(state, state->css_editor_data_);
        // Create drag bar with file explorer tree as target
        drag_bar_ = addNew<DragBar>(file_explorer_tree_, 500, 200, 800);
        monaco_editor_ = addNew<MonacoEditor>(state->tailwind_config_editor_data_.extension_);
        
        
        // Apply styling to child widgets
        monaco_editor_->addStyleClass("h-screen flex-1");
        file_explorer_tree_->addStyleClass("h-screen");

        // Connect drag bar width change signal
        drag_bar_->widthChanged().connect([=](int new_width) {
            std::cout << "Drag bar width changed to: " << new_width << "px\n";
        });

        file_explorer_tree_->file_selected().connect(this, [=](const std::string &selected_file_path)
        {
            std::string file_path = state_->css_editor_data_.root_folder_path_ + selected_file_path;
            // std::string file_path = state_->css_editor_data_.root_resource_url_ + selected_file_path;
            // check if file is good in system
            if (std::filesystem::exists(file_path))
            {
                monaco_editor_->setEditorText(file_path);
                current_css_file_path_ = file_path;
            }
            else
            {
                std::cout << "File not found: " << file_path << "\n\n";
            }
        });

        monaco_editor_->avalable_save().connect(this, [=]()
                                        {
            if(css_selected_file_path_.compare("") == 0) {
                return;
            }

            if(!std::fstream(state_->css_editor_data_.root_folder_path_ +  css_selected_file_path_).good()) {
                std::cout << "\n\n avalable save but file not found: " << state_->css_editor_data_.root_folder_path_ + css_selected_file_path_ << "\n\n";
                return;
            }

            TreeNode* selected_node = file_explorer_tree_->selectedNode();

            if(selected_node && selected_node->label()->text().toUTF8().compare(css_selected_file_path_.substr(css_selected_file_path_.find_last_of("/")+1)) == 0) {
                if(monaco_editor_->unsavedChanges()) {
                    selected_node->toggleStyleClass("unsaved-changes", true, true);
                }
                else {
                    selected_node->toggleStyleClass("unsaved-changes", false, true);
                }
            }
            else {
                std::cout << "\n\n No matching node found for the selected file.\n\n";
            }
        });
        
        folders_changed_.connect(this, [=](std::string selected_file_path)
        {
            file_explorer_tree_->setTreeFolderWidgets();
            // if(css_selected_file_path.compare("") != 0) {
            //     css_selected_file_path = selected_file_path;
            // }
            // getFolders();
            // reuploadFile();
        });
    }

    std::vector<std::string> TailwindCss::getConfigFiles()
    {
        std::vector<std::string> config_files;
        for (const auto &entry : std::filesystem::directory_iterator(state_->css_editor_data_.root_folder_path_))
        {
            if (entry.is_regular_file())
            {
                config_files.push_back(entry.path().filename().string());
            }
        }
        return config_files;
    }

    void TailwindCss::generateCssFile()
    {

        // std::cout << "\n\nGenerating CSS file...\n\n";
        // std::ofstream file("../static/stylus-resources/tailwind4/input.css");
        std::ofstream file(state_->tailwind_config_file_path_);
        if (!file.is_open())
        {
            std::cerr << "\n\n Error opening file for writing: " << state_->tailwind_config_file_path_ << "\n\n";
            return;
        }
        file << "/* Import TailwindCSS base styles */\n";
        file << "@import \"tailwindcss\";\n\n";
        file << "/* Import custom CSS files for additional styles */\n";

        file << "\n";
        file << "/* Source additional templates and styles */\n";
        file << "@source \"../xml/\";\n";
        file << "@source \"../../../src/\";\n\n";

        file << "/* Import custom CSS files */\n";
        for (const auto &css_folder : std::filesystem::directory_iterator(state_->css_editor_data_.root_folder_path_))
        {
            if (css_folder.is_directory())
            {
                std::vector<std::string> files;
                for (const auto &css_file : std::filesystem::directory_iterator(css_folder.path().string()))
                {
                    if (css_file.is_regular_file())
                    {
                        file << "@import \"./css/" << css_folder.path().filename().string() << "/" << css_file.path().filename().string() << "\";\n";
                    }
                }
            }
        }

        file << "/* Define custom theme */\n";
        // file << state_->getFileText(state_->css_editor_data_.root_folder_path_ + config_files_combobox_->currentText().toUTF8()) << "\n\n";
        file << state_->getFileText(state_->tailwind_config_editor_data_.root_folder_path_ + "penguin.css") << "\n\n";
        file.close();

        auto session_id = Wt::WApplication::instance()->sessionId();
        Wt::WServer::instance()->ioService().post([this, session_id]()
                                                  {
            std::array<char, 128> buffer;
            std::string result;
            FILE* pipe = popen("cd ../../static/stylus-resources/tailwind4 && npm run build 2>&1", "r");
            if (pipe) {
                while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                    result += buffer.data();
                }
                pclose(pipe);
            }
            // Optionally, log or process 'result'
            std::cout << "npm run build output:\n" << result << "\n\n";
            std::string error_output;
            if (result.find("Error") != std::string::npos) {
                error_output = result.substr(result.find("Error"));
            }
            std::cout << "Error output:\n" << error_output << "\n";

            Wt::WServer::instance()->post(session_id, [this]() {
                Wt::WApplication::instance()->removeStyleSheet(prev_css_file_path_.toUTF8());
                Wt::WApplication::instance()->useStyleSheet(current_css_file_path_.toUTF8());
                prev_css_file_path_ = current_css_file_path_;
                // output_editor_->setEditorText("static/tailwind.css", state_->getFileText("../../static/tailwind.css"));
            }); });
        std::cout << "CSS file generated successfully.\n\n";
    }

}