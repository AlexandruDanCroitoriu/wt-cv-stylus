# Wt CV Stylus

A modern C++ web application built with Wt framework and styled using TailwindCSS 4, featuring authentication, database integration, and AI-powered speech-to-text.

The project is a personal portfolio and CV website, showcasing different project implementations all in one cohesive application.

## 📚 External Libraries

This project utilizes several high-quality open-source libraries:

### Core Dependencies
- **[Wt Framework](https://github.com/emweb/wt)** - Modern C++ web toolkit
- **[Whisper.cpp](https://github.com/ggerganov/whisper.cpp)** - High-performance speech recognition
- **[JsonCpp](https://github.com/open-source-parsers/jsoncpp)** - JSON parsing and manipulation
- **[TinyXML2](https://github.com/leethomason/tinyxml2)** - Lightweight XML parser
- **[nlohmann/json](https://github.com/nlohmann/json)** - Modern JSON library for C++
- **[cpr](https://github.com/libcpr/cpr)** - C++ HTTP requests library

### Build Requirements
- CMake 3.13+
- C++17 compatible compiler (GCC 7+, Clang 7+)
- Boost libraries (for Wt)
- OpenSSL (for HTTPS/WSS support)

## 🛠️ Quick Start

This project follows a **script-driven development philosophy** where all operations are performed through unified scripts in the `scripts/` directory.

### 1. Clone and Setup
```bash
git clone https://github.com/AlexandruDanCroitoriu/wt-cv-stylus.git
cd wt-cv-stylus

# Clone external libraries
./scripts/clone_libraries.sh

# Download Whisper models
./download_models.sh
```

### 2. Build
```bash
# Build the application (defaults to debug mode)
./scripts/build.sh

# Build with explicit mode selection
./scripts/build.sh --debug      # Debug build
./scripts/build.sh --release    # Release build

# Clean builds
./scripts/build.sh --debug clean
./scripts/build.sh --release clean
```

### 3. Run
```bash
# Run the application (defaults to debug mode)
# Automatically builds if needed
./scripts/run.sh

# Run with explicit mode selection
./scripts/run.sh --debug      # Run debug version
./scripts/run.sh --release    # Run release version

# Run with custom server arguments
./scripts/run.sh --debug -- --http-port=8080 --docroot=.
```

### 4. Development Workflow
```bash
# Standard development cycle
./scripts/build.sh --debug clean  # Clean build
./scripts/run.sh --debug          # Run with auto-rebuild if needed

# Quick rebuild and run (for iterative development)
./scripts/run.sh                  # Auto-rebuilds only if needed
```

Access the application at: http://localhost:9020

## 📜 Script Commands

The `scripts/` directory contains utility scripts for development and monitoring following a **script-driven development philosophy**. **All scripts automatically save their output with timestamps to log files in `scripts/output/` for debugging and audit purposes.**

### Core Development Scripts

#### Build Script
```bash
# Usage: ./scripts/build.sh [--debug|--release|-d|-r] [clean]

# Default build (debug mode)
./scripts/build.sh

# Explicit build type
./scripts/build.sh --debug           # Debug build
./scripts/build.sh --release         # Release build
./scripts/build.sh -d                # Debug build (short option)
./scripts/build.sh -r                # Release build (short option)

# Clean builds
./scripts/build.sh clean             # Clean debug build
./scripts/build.sh --release clean   # Clean release build

# Get help
./scripts/build.sh --help
```

#### Run Script
```bash
# Usage: ./scripts/run.sh [--debug|--release|-d|-r] [-- app_args...]

# Default run (debug mode, auto-builds if needed)
./scripts/run.sh

# Explicit run mode
./scripts/run.sh --debug             # Run debug build
./scripts/run.sh --release           # Run release build
./scripts/run.sh -d                  # Run debug build (short option)
./scripts/run.sh -r                  # Run release build (short option)

# With application arguments
./scripts/run.sh -- --http-port=8080 --docroot=.

# Get help
./scripts/run.sh --help
```

### Utility Scripts

#### Library Management
```bash
# Clone all external dependencies
./scripts/clone_libraries.sh
```

#### Memory Analysis
```bash
# Analyze memory usage of running application
./scripts/memory_analyzer.sh <PID>

# Monitor memory usage in real-time
./scripts/memory_monitor.sh <PID>
```

#### Script Help
```bash
# Display overview of all available scripts
./scripts/README.sh
```

#### Development Helper Script
```bash
# Multi-purpose development helper
./scripts/dev.sh <command> [options]
```

### Model Management
```bash
# Download Whisper models for speech recognition
./download_models.sh
```

### Script Logs
All script executions are automatically logged with comprehensive output capture:
- **Build logs**: `scripts/output/build.log`
- **Run logs**: `scripts/output/run.log`
- **Library logs**: `scripts/output/clone-libraries.log`
- **Memory analysis logs**: `scripts/output/memory-analyzer.log`
- **Memory monitoring logs**: `scripts/output/memory-monitor.log`
- **Development logs**: `scripts/output/dev.log`

**Enhanced Logging Features:**
- **Complete output capture**: Both CMake configuration and build output are fully logged
- **Timestamped entries**: All log entries include precise timestamps
- **Color-coded terminal output**: Enhanced readability with status indicators
- **Structured sections**: Logs are organized with clear section markers
- **Error tracking**: Failed operations are clearly marked and logged

## 📁 Project Structure

### Root Directory (`./`)
```
├── CMakeLists.txt           # Main build configuration
├── wt_config.xml           # Wt framework configuration
├── download_models.sh      # Whisper model downloader
├── memory_analyzer.sh      # Legacy memory analysis tool
├── prolonged_analyses.md   # Performance analysis documentation
└── WHISPER_INTEGRATION_SUMMARY.md  # Whisper integration details
```

### Source Code (`src/`)
Organized component-based architecture with numbered directories for clear hierarchy:

```
src/
├── main.cpp                # Application entry point
├── 000-Server/            # Core server setup and configuration
├── 001-App/               # Main application class and bootstrapping
├── 002-Theme/             # Custom theming and styling components
├── 003-Components/        # Reusable UI components and widgets
├── 004-Dbo/              # Database models and ORM configuration
├── 005-Auth/             # Authentication and user management
├── 006-Navigation/       # Navigation components and routing
├── 007-UserSettings/     # User preferences and settings
├── 008-AboutMe/          # About/profile page components
├── 009-ContactInfo/      # Contact information management
├── 010-StarWarsApi/      # External API integration example
├── 011-Chat/             # Real-time chat with Whisper integration
└── 101-Examples/         # Code examples and demonstrations
```

### Documentation (`docs/`)
Comprehensive documentation and reference materials:

```
docs/
├── GETTING_STARTED.md         # Quick start guide
├── QUICK_REFERENCE.md         # Command reference
├── project-overview.md        # High-level project overview
├── development_guide.md       # Development guidelines
├── build-deployment.md        # Build and deployment instructions
├── api-reference.md          # API documentation
├── component-reference.md     # Component usage guide
├── wt-dbo-reference.md       # Wt::Dbo ORM reference
├── wt-auth-reference.md      # Wt::Auth system reference
├── whisper-thread-memory-analysis.md  # Whisper performance analysis
├── wt-4.11-release/          # Wt framework documentation
├── whispercpp/               # Whisper.cpp integration docs
└── prompts/                  # AI/Copilot context prompts
```

### Static Assets (`static/`)
Web assets and styling resources:

```
static/
├── favicon.svg               # Application favicon
├── tailwind.css             # Tailwind CSS framework
├── stylus/                  # Stylus preprocessor source files
└── stylus-resources/        # Stylus compilation assets
```

### Additional Directories

- **`libs/`** - External libraries (Wt, Whisper.cpp, JsonCpp, TinyXML2)
- **`models/`** - Whisper AI models for speech recognition
- **`resources/`** - Wt framework resources (themes, icons, CSS)
- **`build/`** - Build artifacts and compiled binaries
- **`audio-files/`** - Audio samples for testing Whisper integration
- **`simplechat/`** - Simple chat example implementation
- **`broadcase-example/`** - Broadcasting functionality example

## 🔧 Development

### Script-Driven Development Philosophy

This project follows a **script-driven development approach** where all common operations are handled through standardized scripts in the `scripts/` directory. This ensures consistency, proper logging, and simplified workflows.

### Building for Development
```bash
# Use the unified build script
./scripts/build.sh --debug clean

# Build with compiler optimizations disabled and debug symbols
./scripts/build.sh --debug
```

### Building for Production
```bash
# Use the unified build script with release flag
./scripts/build.sh --release clean

# Build with compiler optimizations and no debug symbols
./scripts/build.sh --release
```

### Running the Application
```bash
### Running the Application
```bash
# Development mode (auto-builds if needed)
./scripts/run.sh --debug

# Production mode (auto-builds if needed)
./scripts/run.sh --release

# With custom arguments
./scripts/run.sh --debug -- --http-port=8080 --docroot=.
```

### Memory Profiling
Use the included memory monitoring tools to profile your application:
```bash
# Start your application
./scripts/run.sh --debug &
APP_PID=$!

# Monitor memory usage
./scripts/memory_monitor.sh $APP_PID
```
```

### Memory Profiling
Use the included memory monitoring tools to profile your application:
```bash
# Start your application using scripts
./scripts/run.sh --debug &
APP_PID=$!

# Monitor memory usage
./scripts/memory_monitor.sh $APP_PID
```

### Database Management
The application uses SQLite with automatic schema creation. Database files are stored in `build/dbo.db`.

## 📖 Documentation

- [Getting Started Guide](docs/GETTING_STARTED.md)
- [API Reference](docs/api-reference.md)
- [Component Reference](docs/component-reference.md)
- [Wt::Dbo ORM Guide](docs/wt-dbo-reference.md)
- [Wt::Auth System Guide](docs/wt-auth-reference.md)
- [Development Guide](docs/development_guide.md)

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [Wt Team](https://www.webtoolkit.eu/) for the excellent C++ web framework
- [Georgi Gerganov](https://github.com/ggerganov) for Whisper.cpp
- All contributors to the open-source libraries used in this project