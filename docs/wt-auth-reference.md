# Wt::Auth Reference Guide

Wt::Auth is a C++ authentication module that provides comprehensive user authentication, registration, and session management functionality. It implements security best practices and can be used independently or integrated with Wt::Dbo for database persistence.

## Table of Contents

1. [Core Concepts](#core-concepts)
2. [Module Organization](#module-organization)
3. [Database Setup](#database-setup)
4. [Service Configuration](#service-configuration)
5. [User Interface](#user-interface)
6. [Authentication Features](#authentication-features)
7. [Multi-Factor Authentication (MFA)](#multi-factor-authentication-mfa)
8. [Advanced Topics](#advanced-topics)
9. [Security Best Practices](#security-best-practices)

## Core Concepts

### Key Components
- **Authentication Service** - Core authentication logic and configuration
- **Password Service** - Password-based authentication with hashing and validation
- **OAuth Service** - Third-party identity provider integration
- **User Database** - Storage abstraction for user and authentication data
- **Login Object** - Session-bound object tracking current user state
- **Auth Widget** - Complete UI for login, registration, and account management

### Namespace and Includes
```cpp
#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/OAuthService.h>
#include <Wt/Auth/Login.h>
#include <Wt/Auth/AuthWidget.h>
#include <Wt/Auth/Dbo/UserDatabase.h>
#include <Wt/Auth/Dbo/AuthInfo.h>

// For convenience
namespace Auth = Wt::Auth;
```

### Main Features
- Password authentication with strong cryptographic hashing (bcrypt)
- Remember-me functionality with secure token management
- Email verification and lost password recovery
- OAuth 2.0 integration (Google, Facebook, OpenID Connect)
- SAML authentication support
- Multi-Factor Authentication (MFA) including TOTP
- Password strength validation and attempt throttling
- Registration with identity merging
- Comprehensive security features

## Module Organization

### Service Classes (Shared across sessions)
```cpp
// Global/application-level services
Auth::AuthService authService;
Auth::PasswordService passwordService(authService);
std::vector<std::unique_ptr<Auth::OAuthService>> oauthServices;
```

### Session-bound Classes
```cpp
// Per-session objects
Auth::Login login;                    // Current login state
Auth::Dbo::UserDatabase userDatabase; // Database access
```

### Transient Classes
```cpp
// UI-related, temporary objects
Auth::AuthWidget authWidget;         // Main authentication UI
Auth::RegistrationWidget regWidget;  // Registration form
Auth::LoginWidget loginWidget;       // Login form
```

## Database Setup

### User and AuthInfo Classes
```cpp
// Forward declaration
class User;
using AuthInfo = Wt::Auth::Dbo::AuthInfo<User>;

// Application user class
class User {
public:
    std::string firstName;
    std::string lastName;
    // Add your application-specific fields

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, firstName, "first_name");
        dbo::field(a, lastName, "last_name");
        // Map other fields
    }
};

// Required macro for Dbo
DBO_EXTERN_TEMPLATES(User)
```

### Session Class with Database Integration
```cpp
#include <Wt/Auth/Login.h>
#include <Wt/Auth/Dbo/UserDatabase.h>
#include <Wt/Dbo/Session.h>

using UserDatabase = Wt::Auth::Dbo::UserDatabase<AuthInfo>;

class Session : public dbo::Session {
public:
    explicit Session(const std::string& sqliteDb);

    // Database access
    Wt::Auth::AbstractUserDatabase& users();
    
    // Login state
    Wt::Auth::Login& login() { return login_; }
    
    // Service access
    static const Wt::Auth::AuthService& auth();
    static const Wt::Auth::PasswordService& passwordAuth();
    static std::vector<const Wt::Auth::OAuthService*> oAuth();
    
    // Configuration
    static void configureAuth();

private:
    std::unique_ptr<UserDatabase> users_;
    Wt::Auth::Login login_;
};
```

### Session Implementation
```cpp
Session::Session(const std::string& sqliteDb) {
    // Setup database connection
    auto connection = std::make_unique<dbo::backend::Sqlite3>(sqliteDb);
    setConnection(std::move(connection));

    // Map persistence classes
    mapClass<User>("user");
    mapClass<AuthInfo>("auth_info");
    mapClass<AuthInfo::AuthIdentityType>("auth_identity");
    mapClass<AuthInfo::AuthTokenType>("auth_token");

    // Create schema if needed
    try {
        createTables();
        std::cerr << "Created database.\n";
    } catch (Wt::Dbo::Exception& e) {
        std::cerr << "Using existing database: " << e.what() << '\n';
    }

    // Initialize user database
    users_ = std::make_unique<UserDatabase>(*this);
}

Wt::Auth::AbstractUserDatabase& Session::users() {
    return *users_;
}
```

### Generated Database Schema
The authentication system creates these tables:
```sql
-- User application data
CREATE TABLE "user" (
    "id" INTEGER PRIMARY KEY AUTOINCREMENT,
    "version" INTEGER NOT NULL
);

-- Authentication information
CREATE TABLE "auth_info" (
    "id" INTEGER PRIMARY KEY AUTOINCREMENT,
    "version" INTEGER NOT NULL,
    "user_id" BIGINT,
    "password_hash" VARCHAR(100) NOT NULL,
    "password_method" VARCHAR(20) NOT NULL,
    "password_salt" VARCHAR(20) NOT NULL,
    "status" INTEGER NOT NULL,
    "failed_login_attempts" INTEGER NOT NULL,
    "last_login_attempt" TEXT,
    "email" VARCHAR(256) NOT NULL,
    "unverified_email" VARCHAR(256) NOT NULL,
    "email_token" VARCHAR(64) NOT NULL,
    "email_token_expires" TEXT,
    "email_token_role" INTEGER NOT NULL,
    FOREIGN KEY ("user_id") REFERENCES "user" ("id")
);

-- Remember-me and other tokens
CREATE TABLE "auth_token" (
    "id" INTEGER PRIMARY KEY AUTOINCREMENT,
    "version" INTEGER NOT NULL,
    "auth_info_id" BIGINT,
    "value" VARCHAR(64) NOT NULL,
    "expires" TEXT,
    FOREIGN KEY ("auth_info_id") REFERENCES "auth_info" ("id")
);

-- User identities (username, OAuth accounts, MFA)
CREATE TABLE "auth_identity" (
    "id" INTEGER PRIMARY KEY AUTOINCREMENT,
    "version" INTEGER NOT NULL,
    "auth_info_id" BIGINT,
    "provider" VARCHAR(64) NOT NULL,
    "identity" VARCHAR(512) NOT NULL,
    FOREIGN KEY ("auth_info_id") REFERENCES "auth_info" ("id")
);
```

## Service Configuration

### Basic Authentication Service
```cpp
namespace {
    Wt::Auth::AuthService myAuthService;
    Wt::Auth::PasswordService myPasswordService(myAuthService);
    std::vector<std::unique_ptr<Wt::Auth::OAuthService>> myOAuthServices;
}

void Session::configureAuth() {
    // Enable remember-me tokens
    myAuthService.setAuthTokensEnabled(true, "logincookie");
    
    // Email verification
    myAuthService.setEmailVerificationEnabled(true);
    myAuthService.setEmailVerificationRequired(true);
    
    // Configure password service
    setupPasswordService();
    
    // Setup OAuth providers
    setupOAuthServices();
}
```

### Password Service Configuration
```cpp
void setupPasswordService() {
    // Password hashing with bcrypt
    auto verifier = std::make_unique<Wt::Auth::PasswordVerifier>();
    verifier->addHashFunction(std::make_unique<Wt::Auth::BCryptHashFunction>(7));
    myPasswordService.setVerifier(std::move(verifier));
    
    // Password attempt throttling
    myPasswordService.setPasswordThrottle(
        std::make_unique<Wt::Auth::AuthThrottle>());
    
    // Password strength validation
    myPasswordService.setStrengthValidator(
        std::make_unique<Wt::Auth::PasswordStrengthValidator>());
}
```

### OAuth Service Configuration
```cpp
void setupOAuthServices() {
    // Google OAuth
    if (Wt::Auth::GoogleService::configured()) {
        myOAuthServices.push_back(
            std::make_unique<Wt::Auth::GoogleService>(myAuthService));
    }
    
    // Facebook OAuth
    if (Wt::Auth::FacebookService::configured()) {
        myOAuthServices.push_back(
            std::make_unique<Wt::Auth::FacebookService>(myAuthService));
    }
    
    // Generate OAuth redirect endpoints
    for (const auto& oAuthService : myOAuthServices) {
        oAuthService->generateRedirectEndpoint();
    }
}

// Service accessors
const Wt::Auth::AuthService& Session::auth() {
    return myAuthService;
}

const Wt::Auth::PasswordService& Session::passwordAuth() {
    return myPasswordService;
}

std::vector<const Wt::Auth::OAuthService*> Session::oAuth() {
    std::vector<const Wt::Auth::OAuthService*> result;
    result.reserve(myOAuthServices.size());
    for (const auto& auth : myOAuthServices) {
        result.push_back(auth.get());
    }
    return result;
}
```

### Advanced Service Configuration
```cpp
void configureAdvancedAuth() {
    // Custom token validity periods
    myAuthService.setAuthTokenValidity(7 * 24 * 60); // 7 days in minutes
    
    // Email token validity
    myAuthService.setEmailTokenValidity(3 * 24 * 60); // 3 days
    
    // Custom password policy
    auto strengthValidator = std::make_unique<Wt::Auth::PasswordStrengthValidator>();
    strengthValidator->setMinimumLength(Wt::Auth::PasswordStrengthValidator::MinimumLength, 8);
    strengthValidator->setMinimumLength(Wt::Auth::PasswordStrengthValidator::OneCharClass, 6);
    myPasswordService.setStrengthValidator(std::move(strengthValidator));
    
    // Custom throttling policy
    auto throttle = std::make_unique<Wt::Auth::AuthThrottle>();
    // Configure custom delay function if needed
    myPasswordService.setPasswordThrottle(std::move(throttle));
}
```

## User Interface

### Basic AuthWidget Usage
```cpp
class AuthApplication : public Wt::WApplication {
public:
    explicit AuthApplication(const Wt::WEnvironment& env)
        : Wt::WApplication(env),
          session_(appRoot() + "auth.db")
    {
        // Listen to login changes
        session_.login().changed().connect(this, &AuthApplication::authEvent);
        
        // Create authentication widget
        auto authWidget = std::make_unique<Wt::Auth::AuthWidget>(
            Session::auth(), session_.users(), session_.login());
        
        // Configure authentication methods
        authWidget->model()->addPasswordAuth(&Session::passwordAuth());
        authWidget->model()->addOAuth(Session::oAuth());
        authWidget->setRegistrationEnabled(true);
        
        // Process environment (handles email tokens, OAuth callbacks)
        authWidget->processEnvironment();
        
        root()->addWidget(std::move(authWidget));
    }
    
private:
    Session session_;
    
    void authEvent() {
        if (session_.login().loggedIn()) {
            const Wt::Auth::User& u = session_.login().user();
            log("notice") << "User " << u.id() 
                         << " (" << u.identity(Wt::Auth::Identity::LoginName) << ")"
                         << " logged in.";
            // Handle login event
            handleUserLogin(u);
        } else {
            log("notice") << "User logged out.";
            // Handle logout event
            handleUserLogout();
        }
    }
    
    void handleUserLogin(const Wt::Auth::User& user) {
        // Customize UI for logged-in user
        // Load user preferences, show user-specific content, etc.
    }
    
    void handleUserLogout() {
        // Clean up user-specific UI
        // Reset to anonymous state
    }
};
```

### Custom AuthWidget
```cpp
class CustomAuthWidget : public Wt::Auth::AuthWidget {
public:
    CustomAuthWidget(Session& session)
        : Wt::Auth::AuthWidget(Session::auth(), session.users(), session.login()),
          session_(session)
    {
        model()->addPasswordAuth(&Session::passwordAuth());
        model()->addOAuth(Session::oAuth());
        setRegistrationEnabled(true);
    }

protected:
    // Override to customize views
    std::unique_ptr<Wt::WWidget> createLoginView() override {
        auto widget = Wt::Auth::AuthWidget::createLoginView();
        // Customize login view
        widget->addStyleClass("custom-login");
        return widget;
    }
    
    std::unique_ptr<Wt::WWidget> createRegistrationView() override {
        auto widget = Wt::Auth::AuthWidget::createRegistrationView();
        // Customize registration view
        widget->addStyleClass("custom-registration");
        return widget;
    }

private:
    Session& session_;
};
```

### Separate Login and Registration Widgets
```cpp
// Use individual widgets instead of combined AuthWidget
void createSeparateAuthUI() {
    // Login widget
    auto loginWidget = std::make_unique<Wt::Auth::LoginWidget>(
        session_.login(), root());
    loginWidget->model()->addPasswordAuth(&Session::passwordAuth());
    loginWidget->model()->addOAuth(Session::oAuth());
    
    // Registration widget  
    auto regWidget = std::make_unique<Wt::Auth::RegistrationWidget>(
        session_.login(), root());
    regWidget->model()->addPasswordAuth(&Session::passwordAuth());
    regWidget->model()->addOAuth(Session::oAuth());
    
    // Add to layout
    auto layout = std::make_unique<Wt::WHBoxLayout>();
    layout->addWidget(std::move(loginWidget));
    layout->addWidget(std::move(regWidget));
    root()->setLayout(std::move(layout));
}
```

## Authentication Features

### Password Authentication
```cpp
// Check user authentication
void checkUserAuth() {
    if (session_.login().loggedIn()) {
        const Wt::Auth::User& user = session_.login().user();
        
        // Get user information
        std::string email = user.email();
        std::string identity = user.identity(Wt::Auth::Identity::LoginName);
        
        // Check user status
        if (user.status() == Wt::Auth::AccountStatus::Normal) {
            // User account is active
        } else if (user.status() == Wt::Auth::AccountStatus::Disabled) {
            // Account is disabled
        }
        
        // Get application user data
        dbo::Transaction t(session_);
        dbo::ptr<User> appUser = session_.user();
        if (appUser) {
            std::string firstName = appUser->firstName;
            std::string lastName = appUser->lastName;
        }
    }
}

// Programmatic login/logout
void programmaticAuth() {
    // Login user by ID
    Wt::Auth::User user = session_.users().findWithId("user123");
    if (user.isValid()) {
        session_.login().login(user);
    }
    
    // Logout
    session_.login().logout();
}
```

### Email Verification
```cpp
// Check email verification status
void checkEmailVerification() {
    if (session_.login().loggedIn()) {
        const Wt::Auth::User& user = session_.login().user();
        
        if (user.emailStatus() == Wt::Auth::EmailStatus::Verified) {
            // Email is verified
        } else {
            // Email needs verification
            // Send verification email
            session_.auth().sendEmailVerification(user, user.unverifiedEmail());
        }
    }
}

// Handle email verification token
void handleEmailToken(const std::string& token) {
    Wt::Auth::EmailTokenResult result = session_.auth().processEmailToken(token);
    
    switch (result.state()) {
        case Wt::Auth::EmailTokenState::Valid:
            // Token is valid, user verified
            break;
        case Wt::Auth::EmailTokenState::Invalid:
            // Invalid token
            break;
        case Wt::Auth::EmailTokenState::Expired:
            // Token expired
            break;
    }
}
```

### Password Reset
```cpp
// Initiate password reset
void initiatePasswordReset(const std::string& email) {
    Wt::Auth::User user = session_.users().findWithEmail(email);
    if (user.isValid()) {
        session_.passwordAuth().lostPassword(email);
        // Password reset email sent
    }
}

// Handle password reset token
void handlePasswordResetToken(const std::string& token) {
    Wt::Auth::User user = session_.users().findWithEmailToken(token);
    if (user.isValid()) {
        // Show password reset form
        auto resetWidget = std::make_unique<Wt::Auth::UpdatePasswordWidget>(
            user, session_.passwordAuth(), session_.login());
        // Add to UI
    }
}
```

### OAuth Integration
```cpp
// Handle OAuth authentication
void handleOAuthAuth() {
    // OAuth process is handled automatically by AuthWidget
    // Listen to login changes to detect OAuth success
    
    session_.login().changed().connect([this]() {
        if (session_.login().loggedIn()) {
            const Wt::Auth::User& user = session_.login().user();
            
            // Check which identity was used
            auto identities = user.identities();
            for (const auto& identity : identities) {
                if (identity.provider() == "google") {
                    // User logged in with Google
                } else if (identity.provider() == "facebook") {
                    // User logged in with Facebook
                }
            }
        }
    });
}

// Add custom OAuth provider
void addCustomOAuthProvider() {
    auto customOAuth = std::make_unique<Wt::Auth::OAuthService>(myAuthService);
    
    // Configure custom provider
    customOAuth->setAuthEndpoint("https://provider.com/oauth/authorize");
    customOAuth->setTokenEndpoint("https://provider.com/oauth/token");
    customOAuth->setClientId("your-client-id");
    customOAuth->setClientSecret("your-client-secret");
    customOAuth->setRedirectEndpoint("/oauth/custom/callback");
    
    // Add scopes
    customOAuth->setScope("profile email");
    
    myOAuthServices.push_back(std::move(customOAuth));
}
```

## Multi-Factor Authentication (MFA)

### Basic TOTP Configuration
```cpp
void configureMFA() {
    // Enable MFA with TOTP
    myAuthService.setMfaProvider(Wt::Auth::Identity::MultiFactor);
    
    // Require MFA for all users
    myAuthService.setMfaRequired(true);
    
    // Or configure per-user MFA (see custom implementation)
}

// Custom AuthModel to control MFA per user
class CustomAuthModel : public Wt::Auth::AuthModel {
public:
    CustomAuthModel(Session& session)
        : Wt::Auth::AuthModel(session.auth(), session.users()),
          session_(session) {}

protected:
    bool hasMfaStep(const Wt::Auth::User& user) const override {
        // Custom logic to determine if user needs MFA
        dbo::Transaction t(session_);
        dbo::ptr<User> appUser = session_.find<User>()
            .where("id = (SELECT user_id FROM auth_info WHERE id = ?)")
            .bind(user.id());
        
        return appUser && appUser->requiresMfa;
    }

private:
    Session& session_;
};
```

### Custom MFA Process
```cpp
// Custom PIN-based MFA (for demonstration)
class PinMfaProcess : public Wt::Auth::Mfa::AbstractMfaProcess {
public:
    PinMfaProcess(const Wt::Auth::AuthService& authService,
                  Wt::Auth::AbstractUserDatabase& users,
                  Wt::Auth::Login& login)
        : AbstractMfaProcess(authService, users, login) {}

    std::unique_ptr<Wt::WWidget> createSetupView() override {
        auto view = std::make_unique<Wt::WTemplate>(tr("pin-setup-template"));
        
        // Generate and display PIN
        generatePin();
        view->bindString("pin", currentPin_);
        
        // Create input field
        auto input = std::make_unique<Wt::WLineEdit>();
        input->enterPressed().connect([this]() { validatePin(); });
        view->bindWidget("pin-input", std::move(input));
        
        // Create submit button
        auto submit = std::make_unique<Wt::WPushButton>(tr("verify"));
        submit->clicked().connect([this]() { validatePin(); });
        view->bindWidget("submit", std::move(submit));
        
        return view;
    }
    
    std::unique_ptr<Wt::WWidget> createInputView() override {
        auto view = std::make_unique<Wt::WTemplate>(tr("pin-input-template"));
        
        // Create input field (PIN not displayed)
        auto input = std::make_unique<Wt::WLineEdit>();
        input->enterPressed().connect([this]() { validatePin(); });
        view->bindWidget("pin-input", std::move(input));
        
        return view;
    }

private:
    std::string currentPin_;
    
    void generatePin() {
        // Generate random PIN
        std::srand(std::time(0));
        currentPin_ = std::to_string(std::rand() % 100000);
        currentPin_ = std::string(5 - currentPin_.length(), '0') + currentPin_;
    }
    
    void validatePin() {
        // Get stored PIN or use current generated PIN
        std::string storedPin = userIdentity().empty() ? currentPin_ : userIdentity();
        std::string enteredPin = /* get from input field */;
        
        if (enteredPin == storedPin) {
            // Success
            if (currentPin_ == storedPin) {
                createUserIdentity(currentPin_); // Store PIN on first setup
            }
            
            login().login(login().user());
            authenticated().emit(Wt::Auth::Mfa::AuthenticationResult(
                Wt::Auth::Mfa::AuthenticationStatus::Success));
        } else {
            // Failure
            authenticated().emit(Wt::Auth::Mfa::AuthenticationResult(
                Wt::Auth::Mfa::AuthenticationStatus::Failure, "Invalid PIN"));
        }
    }
};
```

### Using Custom MFA in AuthWidget
```cpp
class CustomAuthWidget : public Wt::Auth::AuthWidget {
public:
    CustomAuthWidget(Session& session)
        : Wt::Auth::AuthWidget(Session::auth(), session.users(), session.login()) {}

protected:
    std::unique_ptr<Wt::Auth::Mfa::AbstractMfaProcess> createMfaProcess() override {
        auto process = std::make_unique<PinMfaProcess>(
            *model()->baseAuth(), model()->users(), login());
        
        process->authenticated().connect([this](Wt::Auth::Mfa::AuthenticationResult result) {
            if (result.status() == Wt::Auth::Mfa::AuthenticationStatus::Success) {
                createLoggedInView();
            } else {
                // Handle MFA failure
            }
        });
        
        return process;
    }
};
```

### TOTP Configuration Details
```cpp
void configureTOTP() {
    // Default TOTP settings (can be customized)
    // - Time step: 30 seconds
    // - Code length: 6 digits  
    // - Secret key length: 32 characters
    
    // Enable TOTP MFA
    myAuthService.setMfaProvider(Wt::Auth::Identity::MultiFactor);
    
    // Optional: Custom TOTP settings would require custom implementation
    // of AbstractMfaProcess similar to TotpProcess
}
```

## Advanced Topics

### Custom User Database
```cpp
// Implement custom user database (not using Dbo)
class CustomUserDatabase : public Wt::Auth::AbstractUserDatabase {
public:
    Wt::Auth::User findWithId(const std::string& id) const override {
        // Custom implementation
    }
    
    Wt::Auth::User findWithIdentity(const std::string& provider,
                                    const WT_USTRING& identity) const override {
        // Custom implementation  
    }
    
    void addIdentity(const Wt::Auth::User& user,
                     const std::string& provider,
                     const WT_USTRING& identity) override {
        // Custom implementation
    }
    
    // Implement other required methods...
};
```

### Custom Password Verifier
```cpp
class CustomPasswordVerifier : public Wt::Auth::PasswordVerifier {
public:
    CustomPasswordVerifier() {
        // Add custom hash functions
        addHashFunction(std::make_unique<Wt::Auth::BCryptHashFunction>(10));
        addHashFunction(std::make_unique<Wt::Auth::SHA1HashFunction>());
    }
    
    bool needsUpdate(const Wt::Auth::User& user) const override {
        // Custom logic to determine if password needs rehashing
        return Wt::Auth::PasswordVerifier::needsUpdate(user);
    }
};
```

### Session Management
```cpp
void manageUserSessions() {
    // Get current login state
    Wt::Auth::LoginState state = session_.login().state();
    
    switch (state) {
        case Wt::Auth::LoginState::LoggedOut:
            // User not logged in
            break;
        case Wt::Auth::LoginState::Weak:
            // Logged in but may need stronger authentication
            break;
        case Wt::Auth::LoginState::Strong:
            // Fully authenticated
            break;
        case Wt::Auth::LoginState::RequiresMfa:
            // Needs MFA step
            break;
    }
    
    // Force stronger authentication
    if (state == Wt::Auth::LoginState::Weak) {
        session_.login().requireStrongLogin();
    }
}
```

### Event Handling
```cpp
void setupAuthEventHandlers() {
    // Login state changes
    session_.login().changed().connect([this]() {
        if (session_.login().loggedIn()) {
            onUserLogin();
        } else {
            onUserLogout();
        }
    });
    
    // Authentication widget events
    authWidget_->authenticated().connect([this](const Wt::Auth::User& user) {
        onUserAuthenticated(user);
    });
    
    authWidget_->registrationRequested().connect([this]() {
        onRegistrationRequested();
    });
}

void onUserLogin() {
    const Wt::Auth::User& user = session_.login().user();
    
    // Log login event
    log("info") << "User logged in: " << user.identity(Wt::Auth::Identity::LoginName);
    
    // Update UI for logged-in state
    updateUIForLoggedInUser(user);
    
    // Load user preferences
    loadUserPreferences(user);
}

void onUserLogout() {
    // Log logout event
    log("info") << "User logged out";
    
    // Clean up user-specific data
    clearUserData();
    
    // Reset UI to anonymous state
    resetUIToAnonymous();
}
```

## Security Best Practices

### Password Security
```cpp
void configurePasswordSecurity() {
    // Use strong hash function
    auto verifier = std::make_unique<Wt::Auth::PasswordVerifier>();
    verifier->addHashFunction(std::make_unique<Wt::Auth::BCryptHashFunction>(12)); // High cost
    
    // Strong password requirements
    auto strengthValidator = std::make_unique<Wt::Auth::PasswordStrengthValidator>();
    strengthValidator->setMinimumLength(Wt::Auth::PasswordStrengthValidator::MinimumLength, 10);
    strengthValidator->setMinimumLength(Wt::Auth::PasswordStrengthValidator::OneCharClass, 8);
    
    // Enable password attempt throttling
    auto throttle = std::make_unique<Wt::Auth::AuthThrottle>();
    myPasswordService.setPasswordThrottle(std::move(throttle));
    
    myPasswordService.setVerifier(std::move(verifier));
    myPasswordService.setStrengthValidator(std::move(strengthValidator));
}
```

### Session Security
```cpp
void configureSessionSecurity() {
    // Short-lived auth tokens
    myAuthService.setAuthTokenValidity(24 * 60); // 24 hours
    
    // Require email verification
    myAuthService.setEmailVerificationRequired(true);
    
    // Enable MFA for sensitive operations
    myAuthService.setMfaProvider(Wt::Auth::Identity::MultiFactor);
    
    // Secure cookie settings (set in wt_config.xml)
    // <property name="session-id-cookie">
    //   <property name="http-only">true</property>
    //   <property name="secure">true</property>
    //   <property name="same-site">strict</property>
    // </property>
}
```

### Input Validation
```cpp
// Custom registration validator
class SecureRegistrationValidator : public Wt::WValidator {
public:
    Wt::WValidator::Result validate(const Wt::WString& input) const override {
        std::string text = input.toUTF8();
        
        // Check for suspicious patterns
        if (containsSuspiciousContent(text)) {
            return Wt::WValidator::Result(Wt::ValidationState::Invalid, 
                                         "Invalid characters detected");
        }
        
        // Check length limits
        if (text.length() > 100) {
            return Wt::WValidator::Result(Wt::ValidationState::Invalid,
                                         "Input too long");
        }
        
        return Wt::WValidator::Result(Wt::ValidationState::Valid);
    }
    
private:
    bool containsSuspiciousContent(const std::string& text) const {
        // Implement security checks
        return false;
    }
};
```

### Database Security
```cpp
void configureDatabaseSecurity() {
    // Use prepared statements (automatic with Wt::Dbo)
    // Enable foreign key constraints
    auto sqlite = dynamic_cast<dbo::backend::Sqlite3*>(session_.connection());
    if (sqlite) {
        sqlite->setProperty("foreign-keys", "true");
    }
    
    // Encrypt sensitive data in database
    // (implement custom sql_value_traits for encrypted fields)
}
```

---

*This reference guide covers the comprehensive authentication features of Wt::Auth. The module provides enterprise-grade security features while remaining flexible and customizable for specific application needs.*
