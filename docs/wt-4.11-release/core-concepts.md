# Core Concepts

This document covers the fundamental concepts that form the foundation of Wt application development. Understanding these concepts is essential for building effective web applications with Wt.

## Table of Contents

1. [Widget Hierarchy and Lifecycle](#widget-hierarchy-and-lifecycle)
2. [Signal/Slot Event System](#signalslot-event-system)
3. [Session Management](#session-management)
4. [Rendering and Updates](#rendering-and-updates)
5. [Memory Management](#memory-management)
6. [Application Lifecycle](#application-lifecycle)

## Widget Hierarchy and Lifecycle

### Widget Tree Structure

Every Wt application is organized as a hierarchical tree of widgets, with `WApplication::root()` serving as the root container:

```cpp
class MyApplication : public WApplication {
public:
    MyApplication(const WEnvironment& env) : WApplication(env) {
        // Root container - automatically created
        auto container = root()->addNew<WContainerWidget>();
        
        // Child widgets form a tree structure
        auto header = container->addNew<WText>("<h1>Welcome</h1>");
        auto content = container->addNew<WContainerWidget>();
        
        // Nested hierarchy
        auto form = content->addNew<WContainerWidget>();
        auto nameEdit = form->addNew<WLineEdit>();
        auto button = form->addNew<WPushButton>("Submit");
    }
};
```

### Parent-Child Relationships

Widgets establish parent-child relationships that govern:

- **Ownership**: Parent widgets own their children
- **Memory Management**: Deleting a parent automatically deletes all children
- **Rendering**: Children are rendered within their parent's bounds
- **Event Propagation**: Events can bubble up the widget hierarchy

```cpp
// Adding widgets to containers
auto container = std::make_unique<WContainerWidget>();
auto button = container->addNew<WPushButton>("Click Me");

// Alternative method
auto text = addNew<WText>("Hello");
container->addWidget(std::move(text));

// Removing widgets
container->removeWidget(button);  // Removes but doesn't delete
container->clear();               // Removes and deletes all children
```

### Widget Lifecycle

1. **Construction**: Widget is created in memory
2. **Addition to Tree**: Widget becomes part of the application
3. **Rendering**: Widget is rendered to HTML/JavaScript
4. **Active Phase**: Widget responds to events and updates
5. **Removal**: Widget is removed from tree
6. **Destruction**: Widget is destroyed and memory freed

```cpp
// Widget lifecycle example
class CustomWidget : public WContainerWidget {
public:
    CustomWidget() {
        // Constructor - setup initial state
        setupUI();
    }
    
    ~CustomWidget() {
        // Destructor - cleanup resources
        cleanup();
    }
    
protected:
    void render(WFlags<RenderFlag> flags) override {
        // Called during rendering phase
        WContainerWidget::render(flags);
        
        if (flags.test(RenderFlag::RenderFull)) {
            // First-time rendering
            doFullRender();
        }
    }
    
private:
    void setupUI() { /* Initialize widget content */ }
    void cleanup() { /* Release resources */ }
    void doFullRender() { /* Custom rendering logic */ }
};
```

## Signal/Slot Event System

### Basic Signal/Slot Mechanism

Wt uses a type-safe signal/slot system for event handling, similar to Qt but adapted for web applications:

```cpp
class EventExample : public WContainerWidget {
public:
    EventExample() {
        auto button = addNew<WPushButton>("Click Me");
        auto text = addNew<WText>("Not clicked yet");
        
        // Connect signal to slot
        button->clicked().connect([=] {
            text->setText("Button was clicked!");
        });
        
        // Connect to member function
        button->clicked().connect(this, &EventExample::handleClick);
        
        // Connect with arguments
        auto edit = addNew<WLineEdit>();
        edit->textInput().connect([=] {
            text->setText("Typing: " + edit->text().toUTF8());
        });
    }
    
    // Custom signal
    Signal<std::string>& dataChanged() { return dataChanged_; }
    
private:
    Signal<std::string> dataChanged_;
    
    void handleClick() {
        // Emit custom signal
        dataChanged_.emit("Button clicked at " + 
                         WDateTime::currentDateTime().toString().toUTF8());
    }
};
```

### Event Types

Wt provides various event types for different interactions:

```cpp
void setupEventHandlers() {
    auto widget = addNew<WLineEdit>();
    
    // Mouse events
    widget->clicked().connect([=] { /* Handle click */ });
    widget->doubleClicked().connect([=] { /* Handle double-click */ });
    widget->mousePressed().connect([=](const WMouseEvent& e) {
        if (e.button() == MouseButton::Left) {
            // Handle left mouse press
        }
    });
    
    // Keyboard events
    widget->keyPressed().connect([=](const WKeyEvent& e) {
        if (e.key() == Key::Enter) {
            // Handle Enter key
        }
    });
    
    // Focus events
    widget->focussed().connect([=] { /* Widget gained focus */ });
    widget->blurred().connect([=] { /* Widget lost focus */ });
    
    // Widget-specific events
    widget->textInput().connect([=] { /* Text changed */ });
    widget->enterPressed().connect([=] { /* Enter key in edit */ });
}
```

### Advanced Signal/Slot Features

```cpp
class AdvancedEvents : public WContainerWidget {
public:
    AdvancedEvents() {
        setupAdvancedHandlers();
    }
    
private:
    void setupAdvancedHandlers() {
        auto button = addNew<WPushButton>("Advanced");
        
        // Multiple connections to same signal
        button->clicked().connect(this, &AdvancedEvents::logClick);
        button->clicked().connect(this, &AdvancedEvents::updateUI);
        button->clicked().connect(this, &AdvancedEvents::notifyOthers);
        
        // Disconnect signals
        auto connection = button->clicked().connect([=] {
            // This will only fire once
            button->clicked().disconnect(connection);
        });
        
        // Signal forwarding
        button->clicked().connect(actionPerformed_);
    }
    
    void logClick() { /* Log the click event */ }
    void updateUI() { /* Update user interface */ }
    void notifyOthers() { /* Notify other components */ }
    
    Signal<> actionPerformed_;
};
```

### Client-Side Optimization

Wt can optimize event handling by generating client-side JavaScript:

```cpp
class OptimizedEvents : public WContainerWidget {
public:
    OptimizedEvents() {
        auto button = addNew<WPushButton>("Optimized");
        auto text = addNew<WText>("Count: 0");
        
        // Stateless slot - can be optimized to client-side
        button->clicked().connect(
            button, &WPushButton::setText, std::string("Clicked!"));
        
        // Client-side JavaScript slot
        button->clicked().connect(WJavaScript(
            "function() { alert('Client-side alert!'); }"));
    }
};
```

## Session Management

### WApplication and Sessions

Each user session corresponds to a `WApplication` instance:

```cpp
class MyApplication : public WApplication {
public:
    MyApplication(const WEnvironment& env) : WApplication(env) {
        // Access session information
        std::string userAgent = env.userAgent();
        std::string clientAddress = env.clientAddress();
        bool hasJavaScript = env.javaScript();
        bool hasAjax = env.ajax();
        
        // Set application properties
        setTitle("My Application");
        setLocale("en");
        
        // Handle session-specific setup
        setupForSession();
    }
    
    ~MyApplication() {
        // Session cleanup
        cleanupSession();
    }
    
private:
    void setupForSession() {
        // Initialize session-specific resources
        // Set up user preferences
        // Connect to databases
    }
    
    void cleanupSession() {
        // Save session data
        // Close database connections
        // Release resources
    }
};
```

### Session State Management

```cpp
class SessionManager : public WObject {
public:
    static SessionManager& instance() {
        auto app = WApplication::instance();
        auto manager = app->findChild<SessionManager*>("session-manager");
        if (!manager) {
            manager = app->addChild(
                std::make_unique<SessionManager>("session-manager"));
        }
        return *manager;
    }
    
    // Session-wide data storage
    void setUserData(const std::string& key, const std::string& value) {
        userData_[key] = value;
    }
    
    std::string getUserData(const std::string& key) const {
        auto it = userData_.find(key);
        return it != userData_.end() ? it->second : std::string();
    }
    
private:
    std::map<std::string, std::string> userData_;
    
    SessionManager(const std::string& name) {
        setObjectName(name);
    }
};
```

### Keep-Alive Protocol

Wt maintains session connectivity through a keep-alive mechanism:

```cpp
void configureSessionLifetime() {
    // Access through WApplication
    auto app = WApplication::instance();
    
    // Session will timeout after 10 minutes of inactivity
    // (configured in wt_config.xml)
    
    // Handle session timeout
    app->sessionIdChanged().connect([=] {
        // Session ID changed (security measure)
        log("info") << "Session ID renewed for security";
    });
    
    // Detect when session becomes inactive
    app->idleTimeout().connect([=] {
        // User has been idle too long
        showIdleWarning();
    });
}

void showIdleWarning() {
    auto dialog = root()->addChild(
        std::make_unique<WMessageBox>(
            "Session Timeout",
            "Your session will expire soon. Continue working?",
            Icon::Warning,
            StandardButton::Yes | StandardButton::No));
    
    dialog->buttonClicked().connect([=](StandardButton button) {
        if (button == StandardButton::Yes) {
            // Refresh session
            WApplication::instance()->refresh();
        } else {
            // Let session expire
            WApplication::instance()->quit();
        }
        dialog->accept();
    });
    
    dialog->show();
}
```

## Rendering and Updates

### Rendering Process

Wt employs a sophisticated rendering system that adapts to browser capabilities:

```cpp
class RenderingExample : public WContainerWidget {
protected:
    void render(WFlags<RenderFlag> flags) override {
        // Call base class implementation first
        WContainerWidget::render(flags);
        
        if (flags.test(RenderFlag::RenderFull)) {
            // Full rendering - first time or major update
            setupCompleteUI();
        } else {
            // Incremental update
            updateChangedElements();
        }
    }
    
private:
    void setupCompleteUI() {
        // Complete UI setup
        log("debug") << "Performing full render";
    }
    
    void updateChangedElements() {
        // Update only changed parts
        log("debug") << "Performing incremental update";
    }
};
```

### Update Optimization

```cpp
class OptimizedUpdates : public WContainerWidget {
public:
    OptimizedUpdates() {
        setupUI();
    }
    
    void updateData(const std::vector<std::string>& newData) {
        // Batch updates for efficiency
        {
            WApplication::UpdateLock lock(WApplication::instance());
            
            // Multiple updates within lock are batched
            for (size_t i = 0; i < newData.size() && i < dataWidgets_.size(); ++i) {
                dataWidgets_[i]->setText(newData[i]);
            }
            
            // Update timestamp
            timestampText_->setText("Updated: " + 
                WDateTime::currentDateTime().toString().toUTF8());
        }
        // All updates sent to client as single batch
    }
    
private:
    std::vector<WText*> dataWidgets_;
    WText* timestampText_;
    
    void setupUI() {
        // Create data display widgets
        for (int i = 0; i < 10; ++i) {
            dataWidgets_.push_back(addNew<WText>("Data " + std::to_string(i)));
        }
        timestampText_ = addNew<WText>("Not updated yet");
    }
};
```

### Progressive Enhancement

Wt supports progressive enhancement, providing optimal experience for different browser capabilities:

```cpp
void adaptToEnvironment() {
    auto app = WApplication::instance();
    auto& env = app->environment();
    
    if (env.ajax()) {
        // Rich Ajax interface
        setupAjaxInterface();
        
        if (env.webSockets()) {
            // Enhanced with WebSockets
            enableRealTimeFeatures();
        }
    } else {
        // Plain HTML fallback
        setupBasicInterface();
    }
    
    // Mobile-specific adaptations
    if (env.agent() == UserAgent::MobileWebKit) {
        addStyleClass("mobile-optimized");
        enableTouchSupport();
    }
}

void setupAjaxInterface() {
    // Rich interactive features
    auto tree = root()->addNew<WTreeView>();
    tree->setModel(createComplexModel());
    
    // Real-time validation
    auto edit = root()->addNew<WLineEdit>();
    edit->textInput().connect([=] {
        validateInput(edit->text());
    });
}

void setupBasicInterface() {
    // Simple form-based interface
    auto form = root()->addNew<WTemplate>(
        "<form method='post'>"
        "${input} ${submit}"
        "</form>");
    
    form->bindWidget("input", std::make_unique<WLineEdit>());
    form->bindWidget("submit", std::make_unique<WPushButton>("Submit"));
}
```

## Memory Management

### Automatic Widget Management

Wt provides automatic memory management through parent-child relationships:

```cpp
class MemoryManagementExample : public WContainerWidget {
public:
    MemoryManagementExample() {
        // Widgets added to container are automatically managed
        auto text = addNew<WText>("Managed automatically");
        auto button = addNew<WPushButton>("Also managed");
        
        // Manual widget creation
        auto manual = std::make_unique<WLineEdit>();
        auto manualPtr = manual.get();
        addWidget(std::move(manual));  // Now managed by container
        
        // Widgets are deleted when container is destroyed
    }
    
    ~MemoryManagementExample() {
        // All child widgets automatically deleted
        // No explicit cleanup needed
    }
};
```

### Resource Management

```cpp
class ResourceManager : public WObject {
public:
    ResourceManager() {
        // Acquire resources
        fileHandle_ = openFile("data.txt");
        connection_ = connectToDatabase();
    }
    
    ~ResourceManager() {
        // Release resources in destructor
        closeFile(fileHandle_);
        disconnectFromDatabase(connection_);
    }
    
    // RAII pattern for temporary resources
    class ScopedResource {
    public:
        ScopedResource() : resource_(acquireResource()) {}
        ~ScopedResource() { releaseResource(resource_); }
        
        ResourceType* get() const { return resource_; }
        
    private:
        ResourceType* resource_;
    };
    
private:
    FileHandle fileHandle_;
    DatabaseConnection connection_;
};
```

## Application Lifecycle

### Application Startup

```cpp
// Application entry point
int main(int argc, char **argv) {
    try {
        // Initialize application server
        return WRun(argc, argv, [](const WEnvironment& env) {
            // Create new application instance for each session
            return std::make_unique<MyApplication>(env);
        });
    } catch (const WServer::Exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
}

// Alternative with WServer for more control
int main(int argc, char **argv) {
    WServer server(argc, argv, WTHTTP_CONFIGURATION);
    
    server.addEntryPoint(EntryPointType::Application, [](const WEnvironment& env) {
        return std::make_unique<MyApplication>(env);
    });
    
    // Configure server
    server.addResource(std::make_shared<CustomResource>(), "/api");
    
    if (server.start()) {
        WServer::waitForShutdown();
        server.stop();
    }
    
    return 0;
}
```

### Application Initialization

```cpp
class MyApplication : public WApplication {
public:
    MyApplication(const WEnvironment& env) : WApplication(env) {
        // Phase 1: Basic initialization
        setTitle("My Application");
        setLocale("en");
        
        // Phase 2: Load resources
        useStyleSheet("styles/app.css");
        messageResourceBundle().use(appRoot() + "messages/app");
        
        // Phase 3: Setup UI
        createMainInterface();
        
        // Phase 4: Initialize services
        initializeServices();
        
        // Phase 5: Handle URL routing
        internalPathChanged().connect(this, &MyApplication::handlePathChange);
        handlePathChange(internalPath());
    }
    
private:
    void createMainInterface() {
        root()->addStyleClass("application-root");
        
        auto layout = std::make_unique<WVBoxLayout>();
        layout->addWidget(createHeader());
        layout->addWidget(createContent(), 1);  // Expanding
        layout->addWidget(createFooter());
        
        root()->setLayout(std::move(layout));
    }
    
    void initializeServices() {
        // Initialize session-specific services
        authService_ = std::make_unique<AuthService>();
        dataService_ = std::make_unique<DataService>();
        
        // Setup service connections
        authService_->loginChanged().connect(
            this, &MyApplication::handleLoginChange);
    }
    
    void handlePathChange(const std::string& path) {
        // Route to appropriate content based on URL path
        if (path == "/home") {
            showHomePage();
        } else if (path == "/profile") {
            showProfilePage();
        } else {
            setInternalPath("/home", true);  // Redirect to home
        }
    }
    
    std::unique_ptr<WWidget> createHeader() { /* Implementation */ }
    std::unique_ptr<WWidget> createContent() { /* Implementation */ }
    std::unique_ptr<WWidget> createFooter() { /* Implementation */ }
    
    void handleLoginChange() { /* Handle authentication state change */ }
    void showHomePage() { /* Show home content */ }
    void showProfilePage() { /* Show profile content */ }
    
    std::unique_ptr<AuthService> authService_;
    std::unique_ptr<DataService> dataService_;
};
```

## Best Practices

### 1. Widget Organization

```cpp
// Good: Logical widget hierarchies
class UserProfileWidget : public WContainerWidget {
    // Groups related functionality
    // Encapsulates user profile logic
    // Reusable across application
};

// Good: Composition over inheritance
class DashboardWidget : public WContainerWidget {
public:
    DashboardWidget() {
        addWidget(std::make_unique<UserProfileWidget>());
        addWidget(std::make_unique<ActivityFeedWidget>());
        addWidget(std::make_unique<QuickActionsWidget>());
    }
};
```

### 2. Event Handling

```cpp
// Good: Clear event handler organization
class FormWidget : public WContainerWidget {
public:
    FormWidget() {
        setupUI();
        connectEvents();
    }
    
private:
    void setupUI() { /* Create widgets */ }
    
    void connectEvents() {
        submitButton_->clicked().connect(this, &FormWidget::handleSubmit);
        cancelButton_->clicked().connect(this, &FormWidget::handleCancel);
        
        // Validate on input change
        for (auto& field : formFields_) {
            field->changed().connect(this, &FormWidget::validateForm);
        }
    }
    
    void handleSubmit() { /* Process form submission */ }
    void handleCancel() { /* Handle cancellation */ }
    void validateForm() { /* Validate form data */ }
};
```

### 3. Resource Management

```cpp
// Good: RAII pattern for resources
class DatabaseWidget : public WContainerWidget {
public:
    DatabaseWidget() : session_(createDatabaseSession()) {
        loadData();
    }
    
    ~DatabaseWidget() {
        // Session automatically closed by RAII
    }
    
private:
    DatabaseSession session_;  // RAII object
    
    void loadData() {
        try {
            auto data = session_.query("SELECT * FROM users");
            displayData(data);
        } catch (const DatabaseException& e) {
            showError("Failed to load data: " + std::string(e.what()));
        }
    }
};
```

---

*This covers the core concepts essential for Wt development. Next, explore the [Widget Reference](widget-reference.md) for specific widget usage patterns.*
