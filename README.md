# Wt CV Stylus

A modern C++ web application built with the Wt framework and styled using TailwindCSS 4. It features authentication, database integration, a multi-theme switcher, and showcases various project implementations in one cohesive application.

---

## 🚀 Project Overview

- **TailwindCSS 4 styling** with a custom theme system
- **Authentication & database integration** via Wt::Dbo and Wt::Auth
- **AI-powered speech-to-text** using Whisper.cpp
- **Component-based architecture** with reusable UI elements
- **Script-driven development workflow**

---

## 📚 External Libraries

This project utilizes several high-quality open-source libraries:

- **[Wt Framework](https://github.com/emweb/wt)** — Modern C++ web toolkit
- **[Whisper.cpp](https://github.com/ggerganov/whisper.cpp)** — High-performance speech recognition
- **[TinyXML2](https://github.com/leethomason/tinyxml2)** — Lightweight XML parser
- **[nlohmann/json](https://github.com/nlohmann/json)** — Modern JSON library for C++

---

## 🛠️ Quick Start

All operations are performed through unified scripts in the `scripts/` directory, following a **script-driven development philosophy**.

---

### 📁 Source Code Structure

Organized as a component-based architecture with numbered directories for clear hierarchy:

```
src/
├── main.cpp                    # Application entry point
├── 000-Server/                 # Core server management (includes Whisper integration / not active at the moment)
├── 001-App/                    # Main application class
├── 002-Theme/                  # Theme system (light/dark mode, theme switcher)
├── 003-Components/             # UI components library (buttons, editors, widgets)
├── 004-Dbo/                    # Database models and sessions (Wt::Dbo)
├── 005-Auth/                   # Authentication system (Wt::Auth)
├── 006-Navigation/             # Navigation components and routing
├── 007-UserSettings/           # User preference management
├── 008-ComponentsDisplay/      # Component display and showcase
├── 101-Examples/               # Examples and demonstrations
├── 999-ExternalServices/       # Integrations with external services
└── 999-Stylus/                 # Stylus for managing all static assets
```

---

### 🖼️ Static Assets

Static assets are managed by Stylus and located in:

```
static/stylus-resources/
```
