#pragma once
#include <Wt/WContainerWidget.h>
#include <Wt/WTemplate.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WMenu.h>
#include <Wt/WMenuItem.h>
#include <vector>

#include "004-Dbo/Session.h"
#include "005-Auth/AuthWidget.h"

class Navigation : public Wt::WTemplate
{
    public:
        Navigation(Session& session);

        void addPage(const std::string &name, std::unique_ptr<Wt::WContainerWidget> page_widget, const std::string &icon_xml_id = "");
        
    private:
        // void setNavigation();
        void showUserPopupMenu();
        std::unique_ptr<Wt::WPopupMenu> popup_menu_;

        Wt::WTemplate *user_menu_temp_;
        Wt::WStackedWidget *stacked_widget_;
        Wt::WMenu *menu_;
        Session& session_;
        std::string menu_item_styles_ = "text-base font-medium font-title text-on-surface-strong";
        std::string menu_item_anchor_styles_ = "flex items-center flex-nowrap px-2 py-1.5 space-x-2 rounded-md";

};
