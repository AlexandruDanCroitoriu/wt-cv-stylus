#include "004-Dbo/Tables/User.h"

#include <Wt/Dbo/Impl.h>
#include <Wt/Auth/Dbo/AuthInfo.h>

DBO_INSTANTIATE_TEMPLATES(User)

User::User(const std::string& name)
  : name_(name), ui_dark_mode_(false), ui_penguin_theme_name_("arctic")
{
}

bool User::hasPermission(const Wt::Dbo::ptr<Permission> permission) const
{
  for (const auto& perm : permissions_) {
    if (perm->name_.compare(permission->name_) == 0) {
      return true;
    }
  }
  return false;
}
