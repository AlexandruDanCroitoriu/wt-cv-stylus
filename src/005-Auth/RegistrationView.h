#pragma once
#include <Wt/Auth/RegistrationWidget.h>


class Session;
class UserDetailsModel;

class RegistrationView : public Wt::Auth::RegistrationWidget
{
public:
  RegistrationView(Session& session, Wt::Auth::AuthWidget *authWidget = 0);

  /* specialize to create user details fields */
  virtual std::unique_ptr<Wt::WWidget> createFormWidget(Wt::WFormModel::Field field);

protected:
  /* specialize to also validate the user details */
  virtual bool validate();

  /* specialize to register user details */
  virtual void registerUserDetails(Wt::Auth::User& user);

private:
  Session& session_;

  std::unique_ptr<UserDetailsModel> detailsModel_;
};
