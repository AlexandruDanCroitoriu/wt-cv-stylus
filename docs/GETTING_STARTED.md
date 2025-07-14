# WT Library - Server-Side Rendering Development Guide

This repository contains comprehensive instructions and examples for developing server-side rendering web applications using the WT (Web Toolkit) library in C++.

## Overview

WT is a C++ library that enables you to build modern, interactive web applications using familiar desktop application programming patterns. It provides:

- **Widget-based architecture** similar to Qt or other GUI toolkits
- **Server-side rendering** with automatic client-side updates
- **Built-in HTTP server** for easy deployment
- **Ajax/WebSocket support** for real-time interactivity
- **Comprehensive widget library** for forms, layouts, and UI components

### 1. Application Structure

Every WT application follows this pattern:

```cpp
class MyApp : public Wt::WApplication {
public:
    MyApp(const Wt::WEnvironment& env) : Wt::WApplication(env) {
        setTitle("My App");
        setupUI();
    }
private:
    void setupUI() {
        root()->addWidget(std::make_unique<Wt::WText>("Hello World"));
    }
};

int main(int argc, char **argv) {
    return Wt::WRun(argc, argv, [](const Wt::WEnvironment& env) {
        return std::make_unique<MyApp>(env);
    });
}
```

### 2. Widget Inheritance Patterns

**Container Widget (most common):**
```cpp
class MyWidget : public Wt::WContainerWidget {
public:
    MyWidget() {
        auto layout = setLayout(std::make_unique<Wt::WVBoxLayout>());
        layout->addWidget(std::make_unique<Wt::WText>("Content"));
    }
};
```

**Composite Widget (for complex widgets):**
```cpp
class MyCompositeWidget : public Wt::WCompositeWidget {
public:
    MyCompositeWidget() {
        auto impl = setImplementation(std::make_unique<Wt::WTemplate>(...));
        // Bind widgets to template
    }
};
```

**Extending Existing Widgets:**
```cpp
class CustomButton : public Wt::WPushButton {
public:
    CustomButton(const std::string& text) : Wt::WPushButton(text) {
        addStyleClass("custom-button");
        clicked().connect(this, &CustomButton::onClicked);
    }
private:
    void onClicked() { /* custom behavior */ }
};
```

### 3. Signal and Event Handling

```cpp
// Basic signal connection
button->clicked().connect(this, &MyClass::onButtonClicked);

// Lambda connection
button->clicked().connect([this]() {
    // Handle click
});

// Custom signals
class MyWidget : public Wt::WContainerWidget {
public:
    Wt::Signal<std::string>& itemSelected() { return itemSelected_; }
private:
    Wt::Signal<std::string> itemSelected_;
};
```

### 4. Layout Management

**CSS-based (recommended):**
```cpp
auto container = addWidget(std::make_unique<Wt::WContainerWidget>());
container->addStyleClass("my-flex-container");
// Define layout in CSS
```

**Layout managers:**
```cpp
auto layout = setLayout(std::make_unique<Wt::WVBoxLayout>());
layout->addWidget(std::make_unique<Wt::WText>("Header"));
layout->addWidget(std::make_unique<Wt::WText>("Content"), 1); // stretch
```

## Common WT Widgets

| Widget | Purpose | Example |
|--------|---------|---------|
| `WText` | Text display | `auto text = std::make_unique<Wt::WText>("Hello");` |
| `WLineEdit` | Text input | `auto input = std::make_unique<Wt::WLineEdit>();` |
| `WPushButton` | Button | `auto btn = std::make_unique<Wt::WPushButton>("Click");` |
| `WContainerWidget` | Container | `auto container = std::make_unique<Wt::WContainerWidget>();` |
| `WVBoxLayout` | Vertical layout | `setLayout(std::make_unique<Wt::WVBoxLayout>());` |
| `WHBoxLayout` | Horizontal layout | `setLayout(std::make_unique<Wt::WHBoxLayout>());` |
| `WTemplate` | HTML template | `auto tmpl = std::make_unique<Wt::WTemplate>("${content}");` |
| `WStackedWidget` | Page container | `auto stack = std::make_unique<Wt::WStackedWidget>();` |

## Development Workflow

1. **Create Application Class**: Inherit from `Wt::WApplication`
2. **Design Widget Hierarchy**: Use containers and layouts
3. **Implement Custom Widgets**: Inherit from appropriate base classes
4. **Handle Events**: Connect signals to slots/lambdas
5. **Style with CSS**: Add style classes and CSS files
6. **Deploy**: Use built-in HTTP server or FastCGI

## Best Practices

1. **Use smart pointers** for widget creation: `std::make_unique<Widget>()`
2. **Store weak references** to child widgets: `Widget* childRef_;`
3. **Connect signals safely** using member function pointers
4. **Prefer CSS layouts** over layout managers when possible
5. **Validate forms** using built-in validators
6. **Handle async operations** with proper locking

## Building and Deployment

### CMakeLists.txt Template

```cmake
cmake_minimum_required(VERSION 3.5)
project(MyWtApp)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost REQUIRED COMPONENTS system filesystem program_options)
find_package(PkgConfig REQUIRED)
pkg_check_modules(WT REQUIRED wt wthttp)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ${WT_LIBRARIES} ${Boost_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${WT_INCLUDE_DIRS})
```

### Running Applications

```bash
# Development
./myapp --http-port=8080 --docroot=. --http-address=0.0.0.0

# Production with SSL
./myapp --https-port=8443 --ssl-certificate=cert.pem --ssl-private-key=key.pem
```

## Resources

- **WT Official Documentation**: https://www.webtoolkit.eu/wt/doc/tutorial/wt.html
- **API Reference**: https://www.webtoolkit.eu/wt/doc/reference/html/
- **Examples in this repo**: See `examples/` directory
- **WT Source Examples**: See `wt/examples/` directory

## Support

For questions about WT library usage:
1. Check the comprehensive guide in [README.md](README.md)
2. Review patterns in [QUICK_REFERENCE.md](QUICK_REFERENCE.md)
3. Study the working examples in `examples/`
4. Refer to the official WT documentation and tutorials

This guide provides everything you need to start building server-side rendering applications with WT!
