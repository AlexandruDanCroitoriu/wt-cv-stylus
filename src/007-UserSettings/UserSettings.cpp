#include "007-UserSettings/UserSettings.h"
#include <Wt/WText.h>

UserSettings::UserSettings(Session& session)
    : Wt::WContainerWidget(), 
    session_(session)
{
    addNew<Wt::WText>("user settings");
}