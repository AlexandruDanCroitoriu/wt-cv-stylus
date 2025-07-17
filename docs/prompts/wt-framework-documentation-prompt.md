# Wt Framework Documentation Generation Prompt

Create comprehensive documentation for the Wt C++ Web Framework by analyzing source code in `./libs/wt-4.11-release/` and cross-referencing official documentation.

## Task Overview

Generate Wt Framework documentation using:
1. Source code analysis in `./libs/wt-4.11-release/`
2. **Official Wt documentation sources**:
   - Framework Overview: https://www.webtoolkit.eu/wt/doc/reference/html/overview.html
   - Getting Started: https://www.webtoolkit.eu/wt/doc/tutorial/wt.html
   - Database ORM: https://www.webtoolkit.eu/wt/doc/tutorial/dbo.html
   - Authentication: https://www.webtoolkit.eu/wt/doc/tutorial/auth.html
3. Practical examples and developer-focused content

## Documentation Structure

Create in `docs/wt-4.11-release/`:

1. **framework-overview.md** - Architecture and core concepts
2. **core-concepts.md** - Widgets, signals/slots, sessions
3. **widget-reference.md** - Widget catalog with examples
4. **layout-system.md** - Layout managers and CSS
5. **event-handling.md** - Signal/slot system
6. **session-management.md** - Application lifecycle
7. **database-integration.md** - Wt::Dbo ORM
8. **authentication-system.md** - Wt::Auth module
9. **deployment-guide.md** - Deployment and configuration
10. **best-practices.md** - Development patterns
11. **api-quick-reference.md** - Quick API lookup
12. **troubleshooting.md** - Common issues

Plus **examples/** subdirectory with documentation for all examples in `./libs/wt-4.11-release/examples/`.

## Key Content Areas

### Core Framework
- Widget hierarchy and rendering (HTML/JavaScript generation)
- Session management and lifecycle (keep-alive protocol)
- Signal/slot event handling
- Bootstrap methods and deployment options

### Widget System
- Basic widgets: WText, WLineEdit, WPushButton, WImage, WComboBox
- Container widgets: WContainerWidget, WStackedWidget
- Advanced widgets: WTable, WTree, custom widgets
- Styling: CSS classes, themes, decorations

### Database Integration (Wt::Dbo)
- ORM with persist() methods and field mapping
- Sessions, transactions, connection pools
- Relationships: One-to-One, One-to-Many, Many-to-Many
- Queries and collections

### Authentication (Wt::Auth)
- Service classes: AuthService, PasswordService, OAuthService
- UserDatabase and AuthInfo persistence
- UI components: AuthWidget, login/registration flows
- Security: password hashing, email verification, MFA

## Code Examples Requirements

Include practical examples for:
- Widget creation and signal/slot connections
- Database operations with Wt::Dbo
- Authentication setup and usage
- Layout implementation and custom widgets

## Examples Documentation

Document all examples in `./libs/wt-4.11-release/examples/`:
- Create index with descriptions
- Document purpose, features, and code structure
- Organize by categories (basic, widgets, features, applications)
- Extract configuration patterns

## Style Guidelines

- **Developer-focused**: Practical usage over theory
- **Example-driven**: Working code snippets
- **Cross-referenced**: Link related topics
- **Accurate**: Verify against official docs and source code

## Success Criteria

Enable developers to:
1. Understand Wt architecture and build web applications
2. Implement database persistence and authentication
3. Deploy applications and troubleshoot issues
4. Follow best practices and reference examples

**Start by studying official documentation, then analyze source code and examples to create comprehensive, practical documentation.**
