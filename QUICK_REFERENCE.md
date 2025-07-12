# WT Library Quick Reference and Patterns

## Common Widget Inheritance Patterns

### 1. Container Widget Pattern
When you need a widget that contains other widgets:

```cpp
class MyContainerWidget : public Wt::WContainerWidget {
public:
    MyContainerWidget() {
        setupLayout();
        connectSignals();
    }
    
private:
    void setupLayout();
    void connectSignals();
};
```

### 2. Composite Widget Pattern
When you want to encapsulate a complex widget hierarchy:

```cpp
class MyCompositeWidget : public Wt::WCompositeWidget {
public:
    MyCompositeWidget() {
        auto impl = setImplementation(std::make_unique<Wt::WTemplate>(...));
        // Bind widgets to template
    }
};
```

### 3. Inheriting from Existing Widgets
When you want to extend existing widget functionality:

```cpp
class CustomLineEdit : public Wt::WLineEdit {
public:
    CustomLineEdit() : Wt::WLineEdit() {
        // Add custom validation, styling, behavior
    }
    
    // Override virtual methods as needed
    virtual void setValueText(const Wt::WString& value) override;
};
```

## Essential Widget Classes

### Core Widgets
- `Wt::WApplication` - Main application class
- `Wt::WContainerWidget` - Container for other widgets
- `Wt::WTemplate` - HTML template widget

### Form Widgets
- `Wt::WLineEdit` - Single-line text input
- `Wt::WTextArea` - Multi-line text input
- `Wt::WComboBox` - Dropdown selection
- `Wt::WCheckBox` - Checkbox input
- `Wt::WRadioButton` - Radio button input
- `Wt::WSpinBox` - Numeric input with spinners
- `Wt::WPushButton` - Clickable button

### Display Widgets
- `Wt::WText` - Text display
- `Wt::WImage` - Image display
- `Wt::WTable` - Table display
- `Wt::WTree` - Tree display
- `Wt::WGroupBox` - Grouped content with border

### Layout Widgets
- `Wt::WVBoxLayout` - Vertical box layout
- `Wt::WHBoxLayout` - Horizontal box layout
- `Wt::WGridLayout` - Grid layout
- `Wt::WBorderLayout` - Border layout
- `Wt::WStackedWidget` - Stacked widget container

## Signal and Slot Patterns

### Basic Signal Connection
```cpp
button->clicked().connect(this, &MyClass::onButtonClicked);
```

### Lambda Connection
```cpp
button->clicked().connect([this]() {
    // Handle click
});
```

### Signal with Parameters
```cpp
widget->dataChanged().connect([this](const Data& data) {
    handleDataChange(data);
});
```

### Custom Signal Declaration
```cpp
class MyWidget : public Wt::WContainerWidget {
public:
    Wt::Signal<std::string>& itemSelected() { return itemSelected_; }
    
private:
    Wt::Signal<std::string> itemSelected_;
};
```

## Layout Management Best Practices

### CSS-Based Layout (Preferred)
```cpp
auto container = addWidget(std::make_unique<Wt::WContainerWidget>());
container->addStyleClass("my-container");
// Define layout in CSS
```

### Layout Managers
```cpp
auto layout = setLayout(std::make_unique<Wt::WVBoxLayout>());
layout->addWidget(std::make_unique<Wt::WText>("Header"));
layout->addWidget(std::make_unique<Wt::WText>("Content"), 1); // stretch factor
layout->addWidget(std::make_unique<Wt::WText>("Footer"));
```

## Event Handling Patterns

### Form Validation
```cpp
void setupValidation() {
    auto validator = std::make_shared<Wt::WRegExpValidator>("pattern");
    validator->setMandatory(true);
    lineEdit->setValidator(validator);
    
    lineEdit->changed().connect([this]() {
        if (lineEdit->validate() == Wt::ValidationState::Valid) {
            // Handle valid input
        }
    });
}
```

### Async Operations
```cpp
void startAsyncOperation() {
    Wt::WApplication::instance()->deferRendering();
    
    // Start async work
    std::thread([this]() {
        // Do work...
        
        // Update UI in main thread
        Wt::WApplication::UpdateLock lock(Wt::WApplication::instance());
        updateUI();
        Wt::WApplication::instance()->triggerUpdate();
    }).detach();
}
```

## Styling Patterns

### CSS Classes
```cpp
widget->addStyleClass("primary-button");
widget->addStyleClass("large");
```

### Conditional Styling
```cpp
void updateState(bool isActive) {
    if (isActive) {
        widget->addStyleClass("active");
        widget->removeStyleClass("inactive");
    } else {
        widget->addStyleClass("inactive");
        widget->removeStyleClass("active");
    }
}
```

### Inline Styles (Use Sparingly)
```cpp
widget->decorationStyle().setBackgroundColor(Wt::WColor(255, 0, 0));
widget->decorationStyle().setMargin(10);
```

## Navigation Patterns

### Internal Paths
```cpp
class MyApp : public Wt::WApplication {
public:
    MyApp(const Wt::WEnvironment& env) : Wt::WApplication(env) {
        internalPathChanged().connect(this, &MyApp::handlePathChange);
    }
    
private:
    void handlePathChange(const std::string& path) {
        if (path == "/page1") {
            showPage1();
        } else if (path == "/page2") {
            showPage2();
        }
    }
};
```

### Stacked Widget Navigation
```cpp
void navigateToPage(int pageIndex) {
    stackedWidget->setCurrentIndex(pageIndex);
    setInternalPath("/page" + std::to_string(pageIndex), true);
}
```

## Resource Management

### Adding Stylesheets
```cpp
useStyleSheet("css/app.css");
useStyleSheet("https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.4/css/all.min.css");
```

### Adding JavaScript
```cpp
require("js/app.js");
doJavaScript("initializeComponents();");
```

### Static Resources
```cpp
// Place files in docroot directory
auto image = std::make_unique<Wt::WImage>("images/logo.png");
```

## Common Deployment Commands

### Development Server
```bash
./myapp --http-port=8080 --docroot=. --http-address=0.0.0.0
```

### Production with SSL
```bash
./myapp --https-port=8443 --ssl-certificate=cert.pem --ssl-private-key=key.pem --docroot=/var/www/myapp
```

### FastCGI Deployment
```bash
./myapp --fcgi-socket=/tmp/myapp.sock
```

## Performance Tips

1. **Use Hidden Widgets for Preloading**:
   ```cpp
   auto widget = addWidget(std::make_unique<MyWidget>());
   widget->hide(); // Preloaded but not visible
   ```

2. **Defer Rendering for Heavy Operations**:
   ```cpp
   Wt::WApplication::instance()->deferRendering();
   // Heavy operation
   Wt::WApplication::instance()->resumeRendering();
   ```

3. **Use CSS for Layout Instead of Layout Managers When Possible**

4. **Minimize Signal Connections in Loops**

5. **Use Smart Pointers for Widget Management**

## Debugging Tips

1. **Enable Debug Logging**:
   ```cpp
   Wt::WApplication::instance()->log("debug") << "Debug message";
   ```

2. **Use Browser Developer Tools** to inspect generated HTML

3. **Check Network Tab** for resource loading issues

4. **Validate HTML Output** to ensure proper structure

5. **Use CSS Selectors** to target specific widget types in stylesheets
