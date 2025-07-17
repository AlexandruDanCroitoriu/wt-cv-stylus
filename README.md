# Wt CV Stylus Copilot

A modern C++ web application built with Wt framework, and styled using TailwindCss4, featuring authentication, database integration, and AI-powered speech to text.

The pooject is my personal portfolio and CV website, showcasing diferent project implementations all in one.

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

### 1. Clone and Setup
```bash
git clone https://github.com/AlexandruDanCroitoriu/wt-cv-stylus-copilot.git
cd wt-cv-stylus-copilot

# Clone external libraries
./scripts/clone_libraries.sh

# Download Whisper models
./download_models.sh
```

### 2. Build
```bash
# For development (debug build)
./scripts/build-debug.sh

# For production (release build)
./scripts/build-release.sh

# Or use the universal development script
./scripts/dev.sh build --debug     # Debug build
./scripts/dev.sh build --release   # Release build
```

### 3. Run
```bash
# Run debug version
./scripts/run-debug.sh

# Run release version  
./scripts/run-release.sh

# Or use the universal development script
./scripts/dev.sh run --debug       # Run debug version
./scripts/dev.sh run --release     # Run release version
```

### 4. Build and Run in One Command
```bash
# Build and run debug version
./scripts/build-and-run.sh debug

# Build and run release version
./scripts/build-and-run.sh release

# Or use the universal development script (recommended)
./scripts/dev.sh dev --debug       # Build and run debug
./scripts/dev.sh dev --release     # Build and run release
```

Access the application at: http://localhost:9020

## 📜 Script Commands

The `scripts/` directory contains utility scripts for development and monitoring. **All scripts automatically save their output with timestamps to log files in `scripts/output/` for debugging and audit purposes.**

### Library Management
```bash
# Clone all external libraries without creating git submodules
./scripts/clone_libraries.sh
```

### Build Scripts
```bash
# Build application in debug mode
./scripts/build-debug.sh [clean]

# Build application in release mode  
./scripts/build-release.sh [clean]

# Examples:
./scripts/build-debug.sh clean    # Clean debug build
./scripts/build-release.sh        # Incremental release build
```

### Run Scripts
```bash
# Run debug version of the application
./scripts/run-debug.sh [additional_args...]

# Run release version of the application
./scripts/run-release.sh [additional_args...]

# Examples:
./scripts/run-debug.sh                    # Run with default settings
./scripts/run-release.sh --http-port 8080 # Override port
```

### Combined Build and Run Scripts
```bash
# Build and run in one command
./scripts/build-and-run.sh [debug|release] [clean] [-- app_args...]

# Universal development script (recommended)
./scripts/dev.sh <command> [options] [-- app_args...]

# Examples:
./scripts/build-and-run.sh debug clean           # Clean debug build and run
./scripts/build-and-run.sh release -- --threads 8 # Release with custom args

./scripts/dev.sh build --release --clean         # Clean release build
./scripts/dev.sh run --debug                     # Run debug version
./scripts/dev.sh dev --release                   # Build and run release
./scripts/dev.sh status                          # Show project status
./scripts/dev.sh clean                           # Clean all builds
```

### Memory Monitoring
```bash
# Comprehensive memory analysis for a specific process
./scripts/memory_analyzer.sh <PID>

# Continuous memory monitoring (updates every second)
./scripts/memory_monitor.sh <PID>

# Legacy memory analyzer (project root)
./memory_analyzer.sh
```

### Model Management
```bash
# Download Whisper models for speech recognition
./download_models.sh
```

### Script Logs
All script executions are automatically logged with comprehensive output capture:
- **Build logs**: `scripts/output/build-debug-YYYYMMDD_HHMMSS.log`
- **Run logs**: `scripts/output/run-release-YYYYMMDD_HHMMSS.log`
- **Combined logs**: `scripts/output/build-and-run-YYYYMMDD_HHMMSS.log`
- **Development logs**: `scripts/output/dev-command-buildtype-YYYYMMDD_HHMMSS.log`
- **Library logs**: `scripts/output/clone-libraries-YYYYMMDD_HHMMSS.log`

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

### Building for Development
```bash
# Use the provided script for easy debug builds
./scripts/build-debug.sh clean

# Or manually:
mkdir build/debug && cd build/debug
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
make -j$(nproc)
```

### Building for Production
```bash
# Use the provided script for optimized release builds
./scripts/build-release.sh clean

# Or manually:
mkdir build/release && cd build/release
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Running the Application
```bash
# Development with debugging symbols
./scripts/run-debug.sh

# Production optimized version
./scripts/run-release.sh

# With custom arguments
./scripts/run-debug.sh --http-port 8080 -v 3
```

### Memory Profiling
Use the included memory monitoring tools to profile your application:
```bash
# Start your application
./scripts/run-debug.sh &
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