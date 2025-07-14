# API Reference and Code Patterns

## Server Class API

### Server (`000-Server/Server.h`)
```cpp
class Server {
public:
    static Server& getInstance();
    
    // Service management
    void initialize();
    void shutdown();
    
    // Configuration
    void loadConfiguration();
    
private:
    Server() = default;
    // Singleton pattern implementation
};
```

## App Class API

### App (`001-App/App.h`)
```cpp
class App : public Wt::WApplication {
public:
    App(const Wt::WEnvironment& env);
    
    // Navigation
    void handleInternalPath(const std::string& internalPath);
    
    // Theme integration
    void setTheme(const std::string& themeName);
    
private:
    void setupUI();
    void setupNavigation();
    std::unique_ptr<Navigation> navigation_;
    std::unique_ptr<Wt::WStackedWidget> mainStack_;
};
```

## Theme System API

### Theme (`002-Theme/Theme.h`)
```cpp
class Theme {
public:
    enum class Mode { Light, Dark, Auto };
    
    void setMode(Mode mode);
    Mode getMode() const;
    
    // CSS management
    void applyCssClasses(Wt::WWidget* widget, const std::string& component);
    void refreshTheme();
    
    // Signals
    Wt::Signal<Mode>& themeChanged() { return themeChanged_; }
    
private:
    Mode currentMode_;
    Wt::Signal<Mode> themeChanged_;
};
```

### DarkModeToggle (`002-Theme/DarkModeToggle.h`)
```cpp
class DarkModeToggle : public Wt::WContainerWidget {
public:
    DarkModeToggle();
    
    // State management
    void setDarkMode(bool dark);
    bool isDarkMode() const;
    
    // Signals
    Wt::Signal<bool>& toggled() { return toggled_; }
    
private:
    Wt::WCheckBox* toggle_;
    Wt::Signal<bool> toggled_;
    void onToggle();
};
```

## Component API Patterns

### Button (`003-Components/Button.h`)
```cpp
class Button : public Wt::WPushButton {
public:
    enum class Style { Primary, Secondary, Success, Warning, Danger };
    enum class Size { Small, Medium, Large };
    
    Button(const Wt::WString& text = "");
    
    // Styling
    void setStyle(Style style);
    void setSize(Size size);
    void setLoading(bool loading);
    
    // State management
    void setDisabled(bool disabled);
    
private:
    Style currentStyle_;
    Size currentSize_;
    void updateClasses();
};
```

### MonacoEditor (`003-Components/MonacoEditor.h`)
```cpp
class MonacoEditor : public Wt::WContainerWidget {
public:
    MonacoEditor();
    
    // Content management
    void setContent(const std::string& content);
    std::string getContent() const;
    
    // Configuration
    void setLanguage(const std::string& language);
    void setTheme(const std::string& theme);
    void setReadOnly(bool readOnly);
    
    // Signals
    Wt::Signal<std::string>& contentChanged() { return contentChanged_; }
    
private:
    std::string editorId_;
    Wt::Signal<std::string> contentChanged_;
    void initializeEditor();
};
```

## Database API Patterns

### Session (`004-Dbo/Session.h`)
```cpp
class Session {
public:
    Session();
    ~Session();
    
    // Session management
    void configureAuth();
    Wt::Auth::AbstractUserDatabase& users();
    Wt::Auth::Login& login() { return login_; }
    
    // Database operations
    template<class T>
    Wt::Dbo::ptr<T> add(std::unique_ptr<T> object);
    
    template<class T>
    Wt::Dbo::collection<Wt::Dbo::ptr<T>> find();
    
private:
    std::unique_ptr<Wt::Dbo::backend::Sqlite3> sqlite3_;
    std::unique_ptr<Wt::Dbo::Session> session_;
    Wt::Auth::Login login_;
};
```

### User Model (`004-Dbo/Tables/User.cpp`)
```cpp
class User {
public:
    // Properties
    std::string name;
    std::string email;
    Wt::WDateTime createdAt;
    
    // Relationships
    Wt::Dbo::collection<Wt::Dbo::ptr<Permission>> permissions;
    
    // Methods
    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, name, "name");
        Wt::Dbo::field(a, email, "email");
        Wt::Dbo::field(a, createdAt, "created_at");
        Wt::Dbo::hasMany(a, permissions, Wt::Dbo::ManyToOne, "user");
    }
};
```

## Authentication API

### AuthWidget (`005-Auth/AuthWidget.h`)
```cpp
class AuthWidget : public Wt::Auth::AuthWidget {
public:
    AuthWidget(Session& session);
    
    // Authentication state
    bool isLoggedIn() const;
    std::string currentUser() const;
    
    // UI customization
    void setRegistrationEnabled(bool enabled);
    void setPasswordRecoveryEnabled(bool enabled);
    
    // Signals
    Wt::Signal<>& userLoggedIn() { return userLoggedIn_; }
    Wt::Signal<>& userLoggedOut() { return userLoggedOut_; }
    
private:
    Session& session_;
    Wt::Signal<> userLoggedIn_;
    Wt::Signal<> userLoggedOut_;
};
```

## Audio Recording API

### VoiceRecorder (`010-VoiceRecorder/VoiceRecorder.h`)
```cpp
class VoiceRecorder : public Wt::WContainerWidget {
public:
    VoiceRecorder();
    
    // Recording controls
    void startRecording();
    void stopRecording();
    bool isRecording() const;
    
    // File upload
    void uploadFile();
    void setMaxFileSize(int maxSizeMB);
    
    // Status and feedback
    void setStatusText(const std::string& status);
    
    // Signals
    Wt::Signal<>& recordingStarted() { return recordingStarted_; }
    Wt::Signal<>& recordingStopped() { return recordingStopped_; }
    Wt::Signal<std::string>& fileUploaded() { return fileUploaded_; }
    
private:
    Wt::WPushButton* recordButton_;
    Wt::WPushButton* stopButton_;
    Wt::WPushButton* uploadButton_;
    Wt::WFileUpload* fileUpload_;
    Wt::WText* statusText_;
    
    bool recording_;
    Wt::Signal<> recordingStarted_;
    Wt::Signal<> recordingStopped_;
    Wt::Signal<std::string> fileUploaded_;
    
    void setupRecordingUI();
    void setupFileUpload();
    void onFileUploaded();
    void onFileTooLarge();
};
```

## Navigation API

### Navigation (`006-Navigation/Navigation.h`)
```cpp
class Navigation : public Wt::WContainerWidget {
public:
    Navigation();
    
    // Route management
    void addRoute(const std::string& path, const std::string& title);
    void setCurrentPath(const std::string& path);
    
    // UI updates
    void updateNavigationState();
    void setUserLoggedIn(bool loggedIn);
    
    // Signals
    Wt::Signal<std::string>& pathChanged() { return pathChanged_; }
    
private:
    std::map<std::string, std::string> routes_;
    std::string currentPath_;
    Wt::Signal<std::string> pathChanged_;
    
    void createNavigationMenu();
    void onNavigationClick(const std::string& path);
};
```

## External API Integration

### StarWarsApi (`101-StarWarsApi/StarWarsApi.h`)
```cpp
class StarWarsApi : public Wt::WContainerWidget {
public:
    StarWarsApi();
    
    // API operations
    void fetchCharacter(int characterId);
    void fetchPlanet(int planetId);
    void searchCharacters(const std::string& query);
    
    // Data display
    void displayCharacterInfo(const nlohmann::json& character);
    void displayError(const std::string& error);
    
    // Signals
    Wt::Signal<nlohmann::json>& dataReceived() { return dataReceived_; }
    Wt::Signal<std::string>& errorOccurred() { return errorOccurred_; }
    
private:
    Wt::WLineEdit* searchInput_;
    Wt::WPushButton* searchButton_;
    Wt::WText* resultDisplay_;
    Wt::WText* statusText_;
    
    Wt::Signal<nlohmann::json> dataReceived_;
    Wt::Signal<std::string> errorOccurred_;
    
    void makeApiRequest(const std::string& url);
    void handleApiResponse(const cpr::Response& response);
};
```

## Common Usage Patterns

### Widget Creation
```cpp
// Standard widget creation
auto widget = addWidget(std::make_unique<MyWidget>());

// Widget with initial configuration
auto button = addWidget(std::make_unique<Button>("Click Me"));
button->setStyle(Button::Style::Primary);
button->setSize(Button::Size::Large);
```

### Signal Connection
```cpp
// Lambda connection
widget->signal().connect([this](auto value) {
    handleSignal(value);
});

// Member function connection
widget->signal().connect(this, &MyClass::handleSignal);
```

### Database Operations
```cpp
// Transaction pattern
{
    Wt::Dbo::Transaction transaction(*session_);
    auto user = session_->add(std::make_unique<User>());
    user.modify()->name = "John Doe";
    transaction.commit();
}
```

### Async Operations
```cpp
// Defer rendering for heavy operations
Wt::WApplication::instance()->deferRendering();

// Perform async operation
std::thread([this]() {
    // Heavy work...
    
    // Update UI in main thread
    Wt::WApplication::UpdateLock lock(Wt::WApplication::instance());
    updateUI();
    Wt::WApplication::instance()->triggerUpdate();
}).detach();
```
