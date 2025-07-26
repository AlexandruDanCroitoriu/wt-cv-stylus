#pragma once
#include <Wt/WApplication.h>
#include "004-Dbo/Session.h"

#include "999-Stylus/Stylus.h"
#include "002-Theme/Theme.h"
#include "005-Auth/AuthWidget.h"

class App : public Wt::WApplication
{
public:
    App(const Wt::WEnvironment &env);

    Wt::Signal<bool> dark_mode_changed_;
    Wt::Signal<ThemeConfig> theme_changed_;
    Wt::WDialog* auth_dialog_;
    
private:
    Session session_;
    Stylus::Stylus* stylus_ = nullptr;
    void authEvent();
    Wt::WContainerWidget* app_content_;
    void createApp();
    AuthWidget* auth_widget_;
    Wt::WContainerWidget* app_root_;
};