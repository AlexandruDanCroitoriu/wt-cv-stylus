#include "005-Auth/AuthWidget.h"
#include "RegistrationView.h"
#include "004-Dbo/Session.h"
#include "005-Auth/UserDetailsModel.h"
#include "004-Dbo/Tables/User.h"
#include "004-Dbo/Tables/Permission.h"
#include <Wt/Auth/PasswordService.h>
#include <Wt/WApplication.h>
#include <Wt/WRadioButton.h>
#include <Wt/WButtonGroup.h>

AuthWidget::AuthWidget(Session& session)
  : Auth::AuthWidget(Session::auth(), session.users(), session.login()),
    session_(session)
{ 
  	// setInternalBasePath("/user");

    // wApp->messageResourceBundle().use("../static/stylus-resources/xml/005-Auth/ovrwt-auth");
    // wApp->messageResourceBundle().use("../static/stylus-resources/xml/005-Auth/ovrwt-auth-login");
    // wApp->messageResourceBundle().use("../static/stylus-resources/xml/005-Auth/ovrwt-auth-strings");

    createInitialData();
    model()->addPasswordAuth(&Session::passwordAuth());
    model()->addOAuth(Session::oAuth());
    setRegistrationEnabled(true);

    // processEnvironment();
}

std::unique_ptr<WWidget> AuthWidget::createRegistrationView(const Auth::Identity& id)
{
  auto registrationView = std::make_unique<RegistrationView>(session_, this);
  std::unique_ptr<Auth::RegistrationModel> model = createRegistrationModel();

  if (id.isValid())
    model->registerIdentified(id);

  registrationView->setModel(std::move(model));
  return std::move(registrationView);
}

void AuthWidget::createLoginView()
{
  setTemplateText(tr(login_template_id_)); // default wt template
  // setTemplateText(tr("Wt.Auth.template.login-v0")); // v0 nothing but the data and some basic structure
  // setTemplateText(tr("Wt.Auth.template.login-v1")); // custom implementation v1


  // auto container = bindWidget("template-changer-widget", std::make_unique<Wt::WContainerWidget>());
  // container->setStyleClass("flex items-center justify-start space-x-2");
  // auto group = std::make_shared<Wt::WButtonGroup>();

  // auto default_tmp_btn = container->addNew<Wt::WRadioButton>("default");
  // group->addButton(default_tmp_btn);

  // auto v0_tmp_btn = container->addNew<Wt::WRadioButton>("v0");
  // group->addButton(v0_tmp_btn);

  // auto v1_tmp_btn = container->addNew<Wt::WRadioButton>("v1");
  // group->addButton(v1_tmp_btn);

  // if(login_template_id_.compare("Wt.Auth.template.login") == 0)
  // {
  //   group->setSelectedButtonIndex(0); // Select the first button by default.
  // }else if(login_template_id_.compare("Wt.Auth.template.login-v0") == 0)
  // {
  //   group->setSelectedButtonIndex(1); // Select the second button by default.
  // }else if(login_template_id_.compare("Wt.Auth.template.login-v1") == 0)
  // {
  //   group->setSelectedButtonIndex(2); // Select the third button by default.
  // }

  // group->checkedChanged().connect(this, [=](Wt::WRadioButton *button) {
  //   if(button == default_tmp_btn) {
  //     login_template_id_ = "Wt.Auth.template.login";
  //   } else if(button == v0_tmp_btn) {
  //     login_template_id_ = "Wt.Auth.template.login-v0";
  //   } else if(button == v1_tmp_btn) {
  //     login_template_id_ = "Wt.Auth.template.login-v1";
  //   }
  //   // setTemplateText(tr(login_template_id_));
  //   model()->reset();
  //   createLoginView(); // Recreate the login view with the new template
  // });

  createPasswordLoginView();
  createOAuthLoginView();
#ifdef WT_HAS_SAML
  createSamlLoginView();
#endif // WT_HAS_SAML_
}


void AuthWidget::createInitialData()
{
  Wt::Dbo::Transaction t(session_);
  

  
  std::string username = "maxuli";
  std::string email = "admin@example.com";
  std::string password = "asdfghj1";

  // Check if user already exists
  Wt::Dbo::ptr<User> existingUser = session_.find<User>().where("name = ?").bind(username).resultValue();
  if (existingUser) {
    wApp->log("info") << "User already exists: " << username;
    return; // User already exists, no need to create again
  }
  
  // STYLUS_FILES_MANAGER permission for admin user maxuli
  Wt::Dbo::ptr<Permission> permission = session_.find<Permission>().where("name = ?").bind("STYLUS").resultValue();
  if (!permission) {
    wApp->log("Error") << "Permission STYLUS not found, not creating admin user, (AuthWidget)";
    return;
  }

  Wt::Dbo::ptr<User> user = session_.add(std::make_unique<User>(username));
  user.modify()->permissions_.insert(permission);
  user.modify()->ui_dark_mode_ = true; // Set dark mode as default for the admin user
  user.modify()->ui_penguin_theme_name_ = "arctic"; // Set default theme for the admin user
  // permission.modify()->users_.insert(user);

  Wt::Auth::User authUser = session_.users().registerNew();
  authUser.addIdentity(Wt::Auth::Identity::LoginName, username);
  authUser.setEmail(email);
  session_.passwordAuth().updatePassword(authUser, password);
  
  Wt::Dbo::ptr<AuthInfo> authInfo = session_.find<AuthInfo>("where id = ?").bind(authUser.id());
  authInfo.modify()->setUser(user);
  
  t.commit();
}

WDialog *AuthWidget::showDialog(const WString& title, std::unique_ptr<WWidget> contents) 
{
  if (contents) {
    dialog_.reset(new WDialog(title));
    dialog_->contents()->addWidget(std::move(contents));
    dialog_->setMinimumSize(Wt::WLength(100, Wt::LengthUnit::ViewportWidth), Wt::WLength(100, Wt::LengthUnit::ViewportHeight));
    dialog_->setStyleClass("absolute top-0 left-0 right-0 bottom-0 w-screen h-screen");
    dialog_->titleBar()->removeFromParent();
    dialog_->escapePressed().connect([=]() { dialog_.reset(); });
    dialog_->contents()->setStyleClass("min-h-screen min-w-screen m-1 p-1 flex items-center justify-center");
    dialog_->contents()->childrenChanged().connect(this, [=]() { dialog_.reset(); });

    dialog_->footer()->hide();

    if (!WApplication::instance()->environment().ajax()) {
      /*
       * try to center it better, we need to set the half width and
       * height as negative margins.
       */
      dialog_->setMargin(WLength("-21em"), Side::Left); // .Wt-form width
      dialog_->setMargin(WLength("-200px"), Side::Top); // ???
    }

    dialog_->show();
  }

  return dialog_.get();
}
