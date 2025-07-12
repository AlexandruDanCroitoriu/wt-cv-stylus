#pragma once
#include <Wt/WContainerWidget.h>
#include "004-Dbo/Session.h"


class UserSettings : public Wt::WContainerWidget
{
public:
    UserSettings(Session& session);
private:
    Session& session_;
};