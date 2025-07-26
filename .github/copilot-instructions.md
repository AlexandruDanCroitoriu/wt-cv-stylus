---
applyTo: "**"
---
# Wt CV Stylus Project - Copilot Instructions

## Project Overview
This is a modern C++ web application built with the Wt framework, featuring:
- Personal portfolio and CV website
- TailwindCSS 4 styling with custom theme system
- Authentication and database integration (SQLite with Wt::Dbo)
- AI-powered speech-to-text via Whisper.cpp
- Component-based architecture with reusable UI elements
- Script-driven development workflow

## Core Technologies
- **Framework**: Wt 4.11+ (C++ web toolkit)
- **Styling**: TailwindCSS 4 with custom theme system
- **Database**: SQLite with Wt::Dbo ORM
- **AI**: Whisper.cpp for speech recognition
- **Build**: CMake 3.13+, C++17
- **External Libraries**: JsonCpp, TinyXML2, nlohmann/json, cpr

## Coding Standards

### Naming Conventions
- **Classes**: PascalCase (`ComponentsDisplay`, `VoiceRecorder`, `StylusState`)
- **Files**: PascalCase matching class names (`ComponentsDisplay.h`, `ComponentsDisplay.cpp`)
- **Variables/Fields**: snake_case with trailing underscore for members (`session_`, `current_theme_`, `xml_file_brains_`)
- **Functions/Methods**: camelCase (`createButtons()`, `handleInternalPath()`, `generateCssFile()`)
- **Constants/Enums**: PascalCase (`ThemeConfig::Arctic`, `PenguinUiWidgetTheme::BtnPrimary`)
- **Namespaces**: PascalCase (`Stylus`, `tinyxml2`)

### File Organization
- **Directory Structure**: Follow numbered prefix system (`000-Server/`, `001-App/`, `002-Theme/`)
- **Header Guards**: Use `#pragma once`
- **Include Order**: System headers, Wt headers, local project headers
- **Namespace Usage**: Use fully qualified names or selective using declarations

### Class Design Patterns
- **Inheritance**: Inherit from appropriate Wt widgets (`Wt::WContainerWidget`, `Wt::WDialog`)
- **Composition**: Prefer composition over deep inheritance hierarchies
- **RAII**: Use smart pointers and proper resource management
- **Singleton**: Use for system-wide services like `Server::getInstance()`

```cpp
// Example class structure
class ComponentsDisplay : public Wt::WContainerWidget {
public:
    ComponentsDisplay();
    
private:
};
```

### Widget Creation and Styling
- **Widget Creation**: Use `addNew<>()` or `std::make_unique<>()` consistently
- **Style Classes**: Use TailwindCSS classes, prefer utility-first approach
- **Responsive Design**: Include responsive classes (`col-lg-4 col-md-6 col-sm-12`)

```cpp
// Good widget creation
auto button = addNew<Button>("Click Me", "m-1.5", PenguinUiWidgetTheme::BtnPrimary);
button->setStyleClass("w-full md:w-auto");

// Good style class management
widget->addStyleClass("flex flex-col items-center");
widget->removeStyleClass("hidden");
```

### Database Patterns (Wt::Dbo)
- **Transaction Scope**: Always wrap database operations in transactions
- **Template Methods**: Implement `persist()` method for all entity classes
- **Relationships**: Use `hasMany()`, `hasOne()`, `belongsTo()` properly
- **Foreign Keys**: Use `Wt::Dbo::ptr<>` and `Wt::Dbo::weak_ptr<>` for relationships

```cpp
// Database entity example
class User : public Wt::Dbo::Dbo<User> {
public:
    std::string name_;
    bool ui_dark_mode_;
    Wt::Dbo::weak_ptr<AuthInfo> authInfo_;
    Wt::Dbo::collection<Wt::Dbo::ptr<Permission>> permissions_;

    template<class Action>
    void persist(Action& a) {
        Wt::Dbo::field(a, name_, "name");
        Wt::Dbo::field(a, ui_dark_mode_, "ui_dark_mode");
        Wt::Dbo::hasOne(a, authInfo_, "user");
        Wt::Dbo::hasMany(a, permissions_, Wt::Dbo::ManyToMany, "users_permissions");
    }
};

// Transaction usage
{
    Wt::Dbo::Transaction transaction(session);
    auto user = session.add(std::make_unique<User>());
    user.modify()->name_ = "John Doe";
    transaction.commit();
}
```

### Signal/Slot Event Handling
- **Lambda Connections**: Prefer lambdas for simple handlers
- **Member Functions**: Use for complex event handling
- **Signal Forwarding**: Chain signals when appropriate
- **Cleanup**: Disconnect signals when widgets are destroyed

```cpp
// Event handling patterns
button->clicked().connect([this]() {
    handleButtonClick();
});

button->clicked().connect(this, &MyClass::onButtonClicked);

// Signal forwarding
childWidget->signalChanged().connect(parentSignal_);
```

### Error Handling and Logging
- **Exceptions**: Use try/catch for database operations and external services
- **Logging**: Use consistent logging levels (`Debug`, `Info`, `Warning`, `Error`, `Fatal`)
- **Validation**: Validate user input and handle edge cases
- **Resource Management**: Ensure proper cleanup of resources

```cpp
// Error handling example
try {
    Wt::Dbo::Transaction transaction(session);
    // Database operations
    transaction.commit();
} catch (const std::exception& e) {
    LOG_ERROR("Database operation failed: " + std::string(e.what()));
    // Handle error appropriately
}
```

### Theme and Styling System
- **CSS Classes**: Maintain consistency with TailwindCSS utility classes

```cpp
// Theme integration
widget->addStyleClass("bg-surface text-on-surface");
widget->removeStyleClass("hidden");
```

## Component Development Guidelines

### Custom Widget Creation
- Inherit from appropriate Wt base class
- Implement constructor with proper initialization
- Separate UI creation from event handling
- Use composition for complex widgets

### Reusable Components
- Create components in `003-Components/` directory
- Make components configurable through constructor parameters
- Document public interface and usage examples

## Build and Development
- **Scripts**: Use scripts in `scripts/` directory for all operations
- **Build Types**: Support both debug and release builds
- **Dependencies**: Manage external libraries through CMake FetchContent

## Documentation Standards
- **Class Definition Comments**: All documentation should be done via comments in class definitions only
- **Header Comments**: Include brief description for classes and complex methods directly in header files
- **Inline Documentation**: Document public interfaces, parameters, and return values within class declarations
- **No External Documentation**: Avoid separate documentation files - keep all documentation co-located with code

```cpp
// Example class documentation
/**
 * @brief Display component for showcasing UI components and their usage
 * 
 * ComponentsDisplay creates interactive examples of buttons, voice recorder,
 * and Monaco editor with copy-to-clipboard functionality for code samples.
 */
class ComponentsDisplay : public Wt::WContainerWidget {
public:
    /**
     * @brief Constructor - initializes the component display with examples
     */
    ComponentsDisplay();
    
private:
    /**
     * @brief Creates button examples with different themes and copy actions
     */
    void createButtons();
};
```

## Security Considerations
- **Input Validation**: Validate all user inputs
- **SQL Injection**: Use Wt::Dbo ORM to prevent SQL injection

## Code Review Checklist
- [ ] Follows naming conventions
- [ ] Proper error handling implemented
- [ ] Database transactions used correctly
- [ ] Signals/slots connected properly
- [ ] Memory leaks prevented
- [ ] Security considerations addressed
- [ ] Code is well-documented