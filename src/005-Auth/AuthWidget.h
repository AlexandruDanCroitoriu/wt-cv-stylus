#pragma once
#include <Wt/Auth/AuthWidget.h>

using namespace Wt;

class Session;

class AuthWidget : public Auth::AuthWidget
{
public:
  AuthWidget(Session& session);

  /* We will use a custom registration view */
  virtual std::unique_ptr<WWidget> createRegistrationView(const Auth::Identity& id) override;
  virtual void createLoginView() override;
protected:
  virtual WDialog *showDialog(const WString& title, std::unique_ptr<WWidget> contents) override;


private:
  Session& session_;
  void createInitialData();
  std::string login_template_id_ = "Wt.Auth.template.login-v1"; // default template id
  // std::string login_template_id_ = "Wt.Auth.template.login"; // default template id
  std::unique_ptr<WDialog> dialog_;

};
