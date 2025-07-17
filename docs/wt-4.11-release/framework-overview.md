# Wt Framework Overview

## Introduction

Wt (pronounced "witty") is a C++ web application framework that enables developers to create modern, interactive web applications using familiar desktop application development patterns. Version 4.11 represents a mature, feature-rich framework that abstracts away the complexities of web development while providing powerful tools for building scalable applications.

## Key Philosophy

Wt's design philosophy centers around **widget-centric development**, where applications are built as hierarchical trees of widgets that encapsulate both visual appearance and behavior. This approach offers several advantages:

- **Type Safety**: Full C++ static typing eliminates runtime errors common in dynamically typed web technologies
- **Code Reusability**: Widgets are truly reusable, customizable, and extensible building blocks
- **Automatic Optimization**: The framework handles Ajax/WebSocket optimization, graceful degradation, and security automatically
- **Single Codebase**: One C++ codebase works for both Ajax-enabled and plain HTML sessions

## Architecture Overview

### Widget Hierarchy

The foundation of every Wt application is a **widget tree** rooted at `WApplication::root()`. Each widget:

- Corresponds to a rectangular region in the user interface
- Manages its own content and behavior
- Owns its child widgets (automatic memory management)
- Can emit signals and connect to slots for event handling

```cpp
// Widget tree example
auto container = root()->addNew<WContainerWidget>();
auto text = container->addNew<WText>("Hello World");
auto button = container->addNew<WPushButton>("Click Me");
```

### Rendering System

Wt employs a sophisticated rendering system that:

- **Progressive Enhancement**: Starts with HTML-compatible rendering, enhances with JavaScript when available
- **Incremental Updates**: Only renders changed portions of the widget tree
- **Automatic Optimization**: Hidden widgets are preloaded in the background for responsive user experience
- **Multi-Backend Support**: Renders to HTML5, with fallbacks for older browsers

### Session Management

Each user session corresponds to a `WApplication` instance that:

- Maintains session state throughout the user's interaction
- Uses keep-alive protocol to detect session termination
- Provides automatic cleanup when sessions end
- Supports both dedicated and shared process models

## Core Components

### 1. Application Framework

- **WApplication**: Central application object managing the widget tree and session
- **WEnvironment**: Provides access to browser capabilities and request information  
- **WServer**: Manages multiple application sessions and deployment configuration

### 2. Widget System

- **Base Widgets**: `WText`, `WLineEdit`, `WPushButton`, `WImage`, etc.
- **Container Widgets**: `WContainerWidget`, `WStackedWidget`, `WGroupBox`
- **Layout Managers**: `WBoxLayout`, `WGridLayout`, `WBorderLayout`
- **Advanced Widgets**: `WTable`, `WTree`, `WCalendar`, `WFileUpload`

### 3. Event Handling

- **Signal/Slot System**: Type-safe event communication between widgets
- **Event Types**: Mouse, keyboard, focus, and widget-specific events
- **Client-Side Optimization**: Stateless slots and JavaScript generation
- **Form Processing**: Automatic form data handling and validation

### 4. Styling and Themes

- **CSS Integration**: Full CSS support with style classes and external stylesheets
- **Built-in Themes**: Bootstrap 2/3/5 themes with customization options
- **Responsive Design**: Mobile-friendly layouts and adaptive styling
- **Custom Themes**: Extensible theming system for brand-specific designs

## Deployment Models

### Built-in HTTP Server (wthttp)

```bash
./myapp --docroot . --http-address 0.0.0.0 --http-port 8080
```

- Self-contained web server with HTTP/HTTPS and WebSocket support
- Ideal for development and standalone deployments
- Supports static file serving and SSL/TLS

### FastCGI Integration (wtfcgi)

```bash
# Apache/Nginx configuration
spawn-fcgi -p 9090 -n ./myapp
```

- Integrates with existing web servers (Apache, Nginx, Lighttpd)
- Scalable process management
- Professional deployment option

### Windows IIS (wtisapi)

- ISAPI plugin for Microsoft IIS
- Windows-native deployment
- Enterprise integration capabilities

## Development Workflow

### 1. Application Structure

```cpp
class MyApplication : public WApplication {
public:
    MyApplication(const WEnvironment& env) : WApplication(env) {
        setTitle("My Application");
        createUI();
    }
    
private:
    void createUI() {
        // Build widget tree
        // Connect signals and slots
        // Set up navigation
    }
};
```

### 2. Widget Development

```cpp
class CustomWidget : public WContainerWidget {
public:
    CustomWidget() {
        addNew<WText>("Custom Content");
        auto button = addNew<WPushButton>("Action");
        button->clicked().connect(this, &CustomWidget::handleClick);
    }
    
    Signal<>& actionPerformed() { return actionPerformed_; }

private:
    Signal<> actionPerformed_;
    
    void handleClick() {
        // Handle user interaction
        actionPerformed_.emit();
    }
};
```

### 3. Application Lifecycle

```cpp
int main(int argc, char **argv) {
    return WRun(argc, argv, [](const WEnvironment& env) {
        return std::make_unique<MyApplication>(env);
    });
}
```

## Key Features

### Security

- **Built-in Protection**: CSRF, XSS, and session hijacking protection
- **Secure Defaults**: Safe-by-default configuration
- **Authentication**: Comprehensive auth module with OAuth2/SAML support
- **SSL/TLS**: Native HTTPS support

### Performance

- **Lazy Loading**: Widgets loaded on-demand
- **Client-Side Caching**: Automatic resource caching
- **Incremental Updates**: Minimal bandwidth usage
- **Stateless Slots**: Client-side event handling optimization

### Internationalization

- **Message Bundles**: XML-based localization system
- **Unicode Support**: Full UTF-8 support throughout
- **Locale-Aware Widgets**: Date, time, and number formatting
- **RTL Support**: Right-to-left language support

### Integration

- **Database ORM**: Wt::Dbo for database integration
- **Authentication**: Wt::Auth for user management
- **Charts**: Advanced charting library
- **Maps**: Google Maps and Leaflet integration

## Browser Compatibility

Wt applications work across all modern browsers:

- **Desktop**: Chrome, Firefox, Safari, Edge, IE11+
- **Mobile**: iOS Safari, Android Chrome, mobile browsers
- **Accessibility**: Screen reader support, keyboard navigation
- **Graceful Degradation**: Works without JavaScript

## Comparison with Other Frameworks

### Advantages over JavaScript Frameworks

- **Type Safety**: Compile-time error detection
- **Performance**: Optimized server-side rendering
- **Security**: Built-in protection mechanisms
- **Maintainability**: Object-oriented design patterns

### Advantages over Server-Side Frameworks

- **Rich Interactions**: Desktop-like user experience
- **Automatic Ajax**: No manual AJAX programming required
- **Widget Reusability**: Component-based architecture
- **Single Language**: C++ for both logic and UI

## Next Steps

To continue learning about Wt:

1. **[Core Concepts](core-concepts.md)** - Understand widgets, signals/slots, and sessions
2. **[Widget Reference](widget-reference.md)** - Comprehensive widget catalog
3. **[Database Integration](database-integration.md)** - Learn Wt::Dbo ORM
4. **[Authentication System](authentication-system.md)** - Implement user management
5. **[Examples](examples/)** - Practical implementation patterns

## Resources

- **Official Documentation**: https://www.webtoolkit.eu/wt/doc/
- **API Reference**: https://www.webtoolkit.eu/wt/doc/reference/html/
- **Community Forum**: https://redmine.webtoolkit.eu/projects/wt/boards
- **Source Code**: https://github.com/emweb/wt

---

*This documentation is based on Wt 4.11 and reflects the current state of the framework's capabilities and best practices.*
