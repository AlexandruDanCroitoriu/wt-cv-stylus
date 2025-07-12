#include "005-Auth/UserDetailsModel.h"
#include "004-Dbo/Tables/User.h"
#include "004-Dbo/Session.h"
#include "002-Theme/Theme.h"


#include <Wt/Auth/Identity.h>
#include <Wt/WApplication.h>

// const Wt::WFormModel::Field
// UserDetailsModel::FavouritePetField = "favourite-pet";

UserDetailsModel::UserDetailsModel(Session& session)
  : WFormModel(),
    session_(session)
{
  // addField(FavouritePetField, Wt::WString::tr("Auth:favourite-pet-info"));
}

void UserDetailsModel::save(const Wt::Auth::User& authUser)
{
  Wt::Dbo::ptr<User> user = session_.user(authUser);
  // user.modify()->favouritePet_ = valueText(FavouritePetField).toUTF8();
  user.modify()->name_ = authUser.identity(Wt::Auth::Identity::LoginName).toUTF8();
  user.modify()->ui_dark_mode_ = wApp->htmlClass().find("dark") != std::string::npos;
  auto theme = dynamic_cast<Theme *>(wApp->theme().get());
  user.modify()->ui_penguin_theme_name_ = theme->getThemeName(theme->current_theme_);
}
