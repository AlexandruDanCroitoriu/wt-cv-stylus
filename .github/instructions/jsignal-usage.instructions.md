---
applyTo: "src/**"
---

# Wt::JSignal Usage Instructions - Wt CV Stylus Project

## Overview
Wt::JSignal is a powerful mechanism in the Wt framework that enables client-side JavaScript to trigger C++ methods on the server. This document provides comprehensive guidelines for implementing and using JSignals in the Wt CV Stylus project.

## What is JSignal?
JSignal is a server-side signal that can be triggered from client-side JavaScript code. It creates a bridge between the browser's JavaScript environment and your C++ application logic, enabling dynamic interactions without page reloads.

## Basic JSignal Implementation

### Header Declaration
```cpp
#pragma once
#include <Wt/WContainerWidget.h>
#include <Wt/JSignal.h>

class MyWidget : public Wt::WContainerWidget
{
public:
    MyWidget();
    
    // Declare JSignal with appropriate parameter types
    Wt::JSignal<std::string> &dataReceived() { return dataReceived_; }
    Wt::JSignal<int, double> &coordinatesChanged() { return coordinatesChanged_; }
    Wt::JSignal<> &buttonClicked() { return buttonClicked_; }

private:
    // JSignal instances
    Wt::JSignal<std::string> dataReceived_;
    Wt::JSignal<int, double> coordinatesChanged_;
    Wt::JSignal<> buttonClicked_;
    
    // Signal handlers
    void handleDataReceived(const std::string& data);
    void handleCoordinatesChanged(int x, double y);
    void handleButtonClick();
};
```

### Implementation
```cpp
#include "MyWidget.h"
#include <Wt/WApplication.h>
#include <Wt/WJavaScript.h>

MyWidget::MyWidget()
    : Wt::WContainerWidget(),
      dataReceived_(this, "dataReceived"),
      coordinatesChanged_(this, "coordinatesChanged"),
      buttonClicked_(this, "buttonClicked")
{
    // Connect JSignals to C++ handlers
    dataReceived_.connect(this, &MyWidget::handleDataReceived);
    coordinatesChanged_.connect(this, &MyWidget::handleCoordinatesChanged);
    buttonClicked_.connect(this, &MyWidget::handleButtonClick);
    
    // Set up the JavaScript environment
    setupJavaScript();
}

void MyWidget::setupJavaScript()
{
    // Add JavaScript code that will trigger the JSignals
    wApp->doJavaScript(
        "function sendData(data) {"
        "  " + dataReceived_.createCall({"data"}) + ";"
        "}"
        
        "function updateCoordinates(x, y) {"
        "  " + coordinatesChanged_.createCall({"x", "y"}) + ";"
        "}"
        
        "function notifyClick() {"
        "  " + buttonClicked_.createCall({}) + ";"
        "}"
    );
}

void MyWidget::handleDataReceived(const std::string& data)
{
    // Process the data received from JavaScript
    std::cout << "Received data: " << data << std::endl;
}

void MyWidget::handleCoordinatesChanged(int x, double y)
{
    // Handle coordinate updates
    std::cout << "Coordinates: (" << x << ", " << y << ")" << std::endl;
}

void MyWidget::handleButtonClick()
{
    // Handle button click from JavaScript
    std::cout << "Button clicked from JavaScript!" << std::endl;
}
```

## JSignal Parameter Types

### Supported Types
- **Basic types**: `int`, `double`, `bool`, `std::string`
- **Wt types**: `Wt::WString`
- **No parameters**: Use `Wt::JSignal<>` for signals without parameters

### Type Examples
```cpp
// Different parameter configurations
Wt::JSignal<> simpleSignal_;                    // No parameters
Wt::JSignal<std::string> textSignal_;           // Single string parameter
Wt::JSignal<int> numberSignal_;                 // Single integer parameter
Wt::JSignal<bool> booleanSignal_;               // Single boolean parameter
Wt::JSignal<double, double> coordinateSignal_;  // Two double parameters
Wt::JSignal<std::string, int, bool> complexSignal_; // Multiple mixed parameters
```

## JavaScript Integration Patterns

### 1. Event-Driven Triggers
```cpp
void MyWidget::setupEventTriggers()
{
    wApp->doJavaScript(
        // Trigger on DOM events
        "document.getElementById('myButton').addEventListener('click', function() {"
        "  " + buttonClicked_.createCall({}) + ";"
        "});"
        
        // Trigger on form submission
        "document.getElementById('myForm').addEventListener('submit', function(e) {"
        "  e.preventDefault();"
        "  var formData = new FormData(e.target);"
        "  " + dataReceived_.createCall({"formData.get('input')"}) + ";"
        "});"
    );
}
```

### 2. Timer-Based Triggers
```cpp
void MyWidget::setupTimerTriggers()
{
    wApp->doJavaScript(
        // Periodic updates
        "setInterval(function() {"
        "  var timestamp = Date.now();"
        "  " + dataReceived_.createCall({"timestamp.toString()"}) + ";"
        "}, 5000);" // Every 5 seconds
    );
}
```

### 3. User Interaction Triggers
```cpp
void MyWidget::setupInteractionTriggers()
{
    wApp->doJavaScript(
        // Mouse movement tracking
        "document.addEventListener('mousemove', function(e) {"
        "  " + coordinatesChanged_.createCall({"e.clientX", "e.clientY"}) + ";"
        "});"
        
        // Keyboard input
        "document.addEventListener('keydown', function(e) {"
        "  " + dataReceived_.createCall({"e.key"}) + ";"
        "});"
    );
}
```

## Advanced Usage Patterns

### 1. Conditional JavaScript Availability
```cpp
void MyWidget::setupConditionalJavaScript()
{
    // Check if JavaScript is available
    if (Wt::WApplication::instance()->environment().ajax()) {
        // JavaScript is available - use JSignals
        setupJavaScript();
        addStyleClass("js-enabled");
    } else {
        // JavaScript not available - provide alternative
        setupAlternativeInterface();
        addStyleClass("no-js");
    }
}

void MyWidget::setupAlternativeInterface()
{
    // Provide form-based alternatives when JavaScript is disabled
    auto button = addNew<Wt::WPushButton>("Submit");
    button->clicked().connect(this, &MyWidget::handleButtonClick);
}
```

### 2. Error Handling in JavaScript
```cpp
void MyWidget::setupErrorHandling()
{
    wApp->doJavaScript(
        "function safeCallSignal(signalCall) {"
        "  try {"
        "    eval(signalCall);"
        "  } catch(error) {"
        "    console.error('JSignal error:', error);"
        "    " + errorSignal_.createCall({"error.message"}) + ";"
        "  }"
        "}"
    );
}
```

### 3. Data Validation
```cpp
void MyWidget::handleDataReceived(const std::string& data)
{
    // Validate incoming data
    if (data.empty()) {
        std::cerr << "Warning: Empty data received from JavaScript" << std::endl;
        return;
    }
    
    if (data.length() > 1000) {
        std::cerr << "Warning: Data too long, truncating" << std::endl;
        processData(data.substr(0, 1000));
    } else {
        processData(data);
    }
}
```

## Integration with Wt CV Stylus Project

### 1. Theme Integration
```cpp
void MyWidget::setupWithTheme()
{
    // Apply project styling
    addStyleClass("bg-surface-alt text-on-surface-alt");
    
    // JavaScript respecting theme
    wApp->doJavaScript(
        "function getThemeAwareColor() {"
        "  var theme = document.documentElement.getAttribute('data-theme');"
        "  return theme === 'dark' ? '#1a1a1a' : '#ffffff';"
        "}"
        
        "function notifyThemeChange() {"
        "  var currentTheme = getThemeAwareColor();"
        "  " + themeChanged_.createCall({"currentTheme"}) + ";"
        "}"
    );
}
```

### 2. Component Integration
```cpp
// In a numbered widget directory (e.g., src/XXX-JSignalWidget/)
class JSignalWidget : public Wt::WContainerWidget
{
public:
    JSignalWidget(const std::string& content = "", 
                  const std::string& styleClasses = "");
    
    // JSignal for external JavaScript integration
    Wt::JSignal<std::string> &externalDataReceived() { return externalDataReceived_; }
    
private:
    Wt::JSignal<std::string> externalDataReceived_;
    
    void setupProjectIntegration();
    void handleExternalData(const std::string& data);
};
```

## Best Practices

### 1. Naming Conventions
- **JSignal names**: Use camelCase, be descriptive (`dataReceived`, `coordinatesChanged`)
- **Handler methods**: Prefix with `handle` (`handleDataReceived`, `handleButtonClick`)
- **JavaScript functions**: Use camelCase, avoid conflicts with existing code

### 2. Performance Considerations
- **Limit frequency**: Don't trigger JSignals too frequently (e.g., on every mousemove)
- **Batch data**: Combine multiple small updates into larger ones
- **Debounce rapid events**: Use JavaScript debouncing for high-frequency events

### 3. Security Considerations
- **Validate all input**: Always validate data received through JSignals
- **Sanitize strings**: Be careful with string data that might contain malicious content
- **Limit data size**: Implement reasonable limits on data size

### 4. Error Handling
- **Graceful degradation**: Provide alternatives when JavaScript is disabled
- **Error logging**: Log errors both client-side and server-side
- **User feedback**: Inform users when operations fail

## Common Use Cases in Wt CV Stylus

### 1. Real-time Data Updates
```cpp
// For receiving real-time sensor data, user analytics, etc.
Wt::JSignal<double, double, std::string> sensorData_;
```

### 2. User Interface Events
```cpp
// For custom UI interactions not covered by standard Wt widgets
Wt::JSignal<int, int> customGestureSignal_;
```

### 3. External API Integration
```cpp
// For handling responses from external JavaScript APIs
Wt::JSignal<std::string> apiResponseSignal_;
```

### 4. File Upload Progress
```cpp
// For custom file upload progress tracking
Wt::JSignal<int> uploadProgressSignal_;
```

## Debugging JSignals

### 1. JavaScript Console Logging
```cpp
void MyWidget::setupDebugging()
{
    wApp->doJavaScript(
        "console.log('JSignal setup complete');"
        "window.debugJSignal = function(data) {"
        "  console.log('Triggering JSignal with:', data);"
        "  " + debugSignal_.createCall({"data"}) + ";"
        "};"
    );
}
```

### 2. Server-Side Logging
```cpp
void MyWidget::handleDebugSignal(const std::string& data)
{
    std::cout << "[DEBUG] JSignal triggered with: " << data << std::endl;
    // Add to project logging system if available
}
```

## CMakeLists.txt Integration

When creating widgets with JSignal functionality, ensure proper CMakeLists.txt integration:

```cmake
# Add to MAIN_APP_SOURCES in CMakeLists.txt
${SOURCE_DIR}/XXX-JSignalWidget/JSignalWidget.cpp
```

## Documentation Requirements

When implementing JSignals:
1. **Document the signal purpose** in header comments
2. **Specify parameter types and meanings**
3. **Include usage examples** in code comments
4. **Note any JavaScript dependencies**
5. **Document fallback behavior** when JavaScript is disabled

This comprehensive guide ensures JSignal implementations in the Wt CV Stylus project are consistent, secure, and maintainable while following established project patterns.
