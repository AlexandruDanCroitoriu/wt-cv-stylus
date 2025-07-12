#pragma once
#include <Wt/Dbo/Types.h>
#include <Wt/WGlobal.h>
#include "004-Dbo/Tables/Permission.h"

class User;
using AuthInfo = Wt::Auth::Dbo::AuthInfo<User>;

class User : public Wt::Dbo::Dbo<User>
{
public:
  User() = default;
  explicit User(const std::string& name);

  std::string name_;
  bool ui_dark_mode_;
  std::string ui_penguin_theme_name_;
  Wt::Dbo::weak_ptr<AuthInfo> authInfo_;
  Wt::Dbo::collection< Wt::Dbo::ptr<Permission> > permissions_;

  bool hasPermission(const Wt::Dbo::ptr<Permission> permission) const;

  template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::field(a, name_, "name");
    Wt::Dbo::field(a, ui_dark_mode_, "ui_dark_mode");
    Wt::Dbo::field(a, ui_penguin_theme_name_, "ui_penguin_theme_name");
    Wt::Dbo::hasOne(a, authInfo_, "user");
    Wt::Dbo::hasMany(a, permissions_, Wt::Dbo::ManyToMany, "users_permissions");
  }
private:
};



DBO_EXTERN_TEMPLATES(User)