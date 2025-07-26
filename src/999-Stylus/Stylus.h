
#pragma once
#include <Wt/WDialog.h>
#include <Wt/WDoubleSpinBox.h>

#include "004-Dbo/Session.h"

#include "999-Stylus/000-Utils/StylusState.h"
// #include "101-Stylus/001-XmlFilesManager/XmlFilesManager.h"
// #include "101-Stylus/002-CssFilesManager/CssFilesManager.h"
// #include "101-Stylus/003-JsFilesManager/JsFilesManager.h"
// #include "101-Stylus/004-TailwindConfigManager/TailwindConfigManager.h"
#include "999-Stylus/001-TailwindCss/TailwindCss.h"
#include "999-Stylus/005-ImagesManager/ImagesManager.h"
// #include "101-Stylus/006-Settings/Settings.h"
#include "002-Theme/DarkModeToggle.h"
#include "002-Theme/ThemeSwitcher.h"

namespace Stylus {

    
    class Stylus : public Wt::WDialog
    {
    public:
        Stylus(Session& session);
            
        // XmlFilesManager* xml_files_manager_ ;
        // CssFilesManager* css_files_manager_;
        // JsFilesManager* js_files_manager_;
        // TailwindConfigManager* tailwind_config_;
        TailwindCss* tailwind_css_;
        ImagesManager* images_manager_;
        // Settings* settings_;
        
        Wt::WContainerWidget* navbar_wrapper_;
        
        Wt::WMenu* menu_;
        Wt::WStackedWidget* content_stack_;

        // Wt::WMenuItem* xml_file_manager_menu_item_;
        // Wt::WMenuItem* css_menu_item_;
        // Wt::WMenuItem* javascript_menu_item_;
        // Wt::WMenuItem* tailwind_menu_item_;
        Wt::WMenuItem* tailwind_css_menu_item_;
        Wt::WMenuItem* images_menu_item_;
        // Wt::WMenuItem* settings_menu_item_;

        DarkModeToggle* dark_mode_toggle_;
        ThemeSwitcher* theme_switcher_;

    private:
        std::shared_ptr<StylusState> state_;
        // void generateCssFile();
        Session& session_;
        void setupKeyboardShortcuts();

};
}