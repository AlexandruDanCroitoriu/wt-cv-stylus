# WT CV Stylus - Component Reference

## Core Application Components

### Server (`000-Server/`)
**Purpose**: Central server management and application lifecycle
- `Server.h/cpp`: Main server class handling initialization, configuration, and service management
- Manages application-wide services and resources
- Handles server startup, shutdown, and configuration

### App (`001-App/`)
**Purpose**: Main application class and entry point
- `App.h/cpp`: Core Wt::WApplication derived class
- Handles application initialization, routing, and main UI setup
- Manages application-wide state and navigation

### Theme System (`002-Theme/`)
**Purpose**: Comprehensive theming and styling system

#### Theme.h/cpp
- Core theme management class
- Handles theme switching between light/dark modes
- Manages CSS class application and theme persistence

#### ThemeSwitcher.h/cpp
- UI component for theme selection
- Dropdown or button interface for theme changes
- Integrates with Theme class for actual switching

#### DarkModeToggle.h/cpp
- Simple toggle switch for dark/light mode
- Boolean toggle interface
- Quick theme switching functionality

### Components (`003-Components/`)
**Purpose**: Reusable UI components library

#### Button.h/cpp
- Enhanced button widget extending Wt::WPushButton
- Custom styling, states, and behaviors
- Standardized button patterns across the application

#### MonacoEditor.h/cpp
- Integration with Monaco Editor (VS Code editor)
- Code editing capabilities within the web interface
- Syntax highlighting, autocomplete, and editor features

#### ComponentsDisplay.h/cpp
- Showcase and testing interface for all components
- Component gallery and demonstration
- Development tool for testing component behaviors

### Database (`004-Dbo/`)
**Purpose**: Database abstraction and ORM integration

#### Session.h/cpp
- Database session management using Wt::Dbo
- Connection handling, transaction management
- Session lifecycle and cleanup

#### Tables/User.cpp
- User entity model for authentication system
- User data persistence and retrieval
- Relationship management with other entities

#### Tables/Permission.cpp
- Permission and role-based access control
- User permission management
- Authorization logic integration

### Authentication (`005-Auth/`)
**Purpose**: User authentication and authorization system

#### AuthWidget.h/cpp
- Main authentication interface
- Login/logout forms and session management
- Integration with Wt authentication system

#### RegistrationView.h/cpp
- User registration interface
- Account creation forms and validation
- New user onboarding process

#### UserDetailsModel.h/cpp
- User profile data management
- User information editing and persistence
- Model for user-related data operations

### Navigation (`006-Navigation/`)
**Purpose**: Application navigation and routing

#### Navigation.h/cpp
- Main navigation component
- Menu systems, breadcrumbs, and routing
- Internal path management and navigation state

### User Settings (`007-UserSettings/`)
**Purpose**: User preference and configuration management

#### UserSettings.h/cpp
- User preference interface
- Settings persistence and retrieval
- Configuration options for personalization

### About Me (`008-AboutMe/`)
**Purpose**: Profile and information pages

#### AboutMe.h/cpp
- User profile display and editing
- About page content and layout
- Personal information management

### Voice Recorder (`010-VoiceRecorder/`)
**Purpose**: Audio recording and file upload functionality

#### VoiceRecorder.h/cpp
- Browser-based audio recording using MediaRecorder API
- File upload interface for audio files
- Recording controls (start, stop, upload)
- Status feedback and error handling
- Integration with Wt::WFileUpload for file management

### Star Wars API (`101-StarWarsApi/`)
**Purpose**: External API integration example

#### StarWarsApi.h/cpp
- Demonstrates HTTP API consumption
- Uses cpr library for HTTP requests
- JSON response parsing with nlohmann/json
- Async operations and UI updates
- Error handling for network operations

## Component Interaction Patterns

### Widget Hierarchy
Most components follow the Wt widget hierarchy:
```cpp
WApplication
├── WContainerWidget (main layout)
├── Navigation (navigation menu)
├── WStackedWidget (page content)
│   ├── ComponentsDisplay
│   ├── UserSettings
│   ├── AboutMe
│   └── VoiceRecorder
└── AuthWidget (authentication overlay)
```

### Signal/Slot Communication
Components communicate through Wt's signal/slot system:
- Theme changes propagate through custom signals
- Authentication state changes notify relevant components
- Navigation updates trigger content changes

### Database Integration
Database-aware components use the Session class:
- User data persistence through User model
- Permission checking through Permission model
- Transaction management for data consistency

### Styling Integration
All components integrate with the theme system:
- CSS class application through Theme manager
- Dynamic styling based on current theme
- Responsive design patterns

## Development Guidelines

### Adding New Components
1. Create header/source files in appropriate numbered directory
2. Inherit from appropriate Wt widget base class
3. Implement setupLayout() and connectSignals() methods
4. Add to CMakeLists.txt SOURCES list
5. Register with navigation system if needed

### Component Dependencies
- All components can access Server instance for shared services
- Database components should use Session for data access
- UI components should integrate with Theme system
- Navigation-aware components should register routes

### Testing Components
Use ComponentsDisplay for component testing and showcase:
- Add new components to the display gallery
- Test different states and configurations
- Verify theme integration and responsiveness
