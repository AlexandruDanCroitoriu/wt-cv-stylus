# WT CV Stylus Project Overview

## Project Description
A modern C++ web application built with the Wt framework, featuring a comprehensive component system, authentication, theming, and audio recording capabilities.

## Architecture

### Core Framework
- **Framework**: Wt (Web Toolkit) C++ web framework
- **Build System**: CMake with FetchContent for dependencies
- **Database**: SQLite with Wt::Dbo ORM
- **Authentication**: Built-in Wt authentication system
- **Styling**: CSS with Bootstrap-like theming support

### Project Structure
```
src/
├── main.cpp                    # Application entry point
├── 000-Server/                 # Core server management
├── 001-App/                    # Main application class
├── 002-Theme/                  # Theme system (light/dark mode)
├── 003-Components/             # UI components library
├── 004-Dbo/                    # Database models and sessions
├── 005-Auth/                   # Authentication system
├── 006-Navigation/             # Navigation components
├── 007-UserSettings/           # User preference management
├── 008-AboutMe/                # About/profile pages
├── 010-VoiceRecorder/          # Audio recording functionality
└── 101-StarWarsApi/            # External API integration example
```

## Key Features

### 1. Component System (`003-Components/`)
- **Button**: Enhanced button widget with custom styling
- **MonacoEditor**: Code editor integration
- **ComponentsDisplay**: Component showcase and testing

### 2. Theme System (`002-Theme/`)
- **Theme**: Core theme management
- **ThemeSwitcher**: Theme selection UI
- **DarkModeToggle**: Dark/light mode toggle

### 3. Authentication (`005-Auth/`)
- **AuthWidget**: Login/logout interface
- **RegistrationView**: User registration
- **UserDetailsModel**: User data management

### 4. Audio Recording (`010-VoiceRecorder/`)
- Browser-based audio recording using MediaRecorder API
- File upload functionality for audio files
- Real-time status updates and error handling

### 5. Database Integration (`004-Dbo/`)
- **Session**: Database session management
- **User**: User model with permissions
- **Permission**: Role-based access control

## Dependencies

### External Libraries
- **nlohmann/json**: JSON parsing and serialization
- **cpr**: HTTP client library for REST API calls
- **Wt**: Core web framework with widgets, HTTP server, and database ORM

### Build Requirements
- CMake 3.13+
- C++17 compatible compiler
- Wt framework installed
- SQLite development libraries

## Development Patterns

### Widget Creation Pattern
```cpp
// Standard widget creation
auto widget = addWidget(std::make_unique<Wt::WWidget>());

// Custom widget inheritance
class CustomWidget : public Wt::WContainerWidget {
public:
    CustomWidget() {
        setupLayout();
        connectSignals();
    }
private:
    void setupLayout();
    void connectSignals();
};
```

### Signal/Slot Pattern
```cpp
// Lambda connections
button->clicked().connect([this]() {
    handleButtonClick();
});

// Member function connections
button->clicked().connect(this, &MyClass::onButtonClicked);
```

### Database Pattern
```cpp
// Transaction pattern
Wt::Dbo::Transaction transaction(session);
auto user = session.add(std::make_unique<User>());
user.modify()->setName("Example");
transaction.commit();
```

### Theme Integration
```cpp
// Adding theme-aware styling
widget->addStyleClass("primary-button");
widget->addStyleClass("theme-aware");
```

## API Integration
The project includes examples of external API integration through the StarWars API component, demonstrating:
- HTTP requests using cpr library
- JSON response parsing
- Async UI updates
- Error handling patterns

## Deployment
The application supports multiple deployment modes:
- Development server (HTTP)
- Production server (HTTPS with SSL)
- FastCGI deployment
- Custom run configurations via CMake targets

## File Upload System
Supports file uploads through Wt::WFileUpload with:
- File size validation
- MIME type checking
- Progress feedback
- Error handling

This project serves as a comprehensive example of modern C++ web development using the Wt framework, showcasing best practices for component architecture, theming, authentication, and multimedia handling.
