#pragma once
#include <003-Components/Button.h>
#include "004-Dbo/Session.h"
#include <Wt/WPopupMenu.h>

class ThemeSwitcher : public Button
{
public:
    ThemeSwitcher(Session& session);
private:
    void showPopup(const Wt::WMouseEvent& event);
    void hidePopup();
    Session& session_;
    std::unique_ptr<Wt::WPopupMenu> popup_menu_;
    std::unique_ptr<Wt::WMenuItem> getMenuItem(const std::string& theme_name);

};