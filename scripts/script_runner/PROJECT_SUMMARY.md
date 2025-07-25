# Script Runner - Terminal-Based Script Execution Tool

A comprehensive C++ terminal application for discovering, managing, and executing scripts with real-time output monitoring. Built with ncurses for an interactive terminal interface.

## ✅ Project Status: COMPLETE

This project has been fully implemented and tested. All core features are working as designed.

## 🚀 Features

- **Script Discovery**: Automatically finds executable scripts in the current directory
- **Real-time Output**: Live capture and display of script output (stdout/stderr)
- **Terminal UI**: Three-pane ncurses interface with keyboard navigation
- **Process Management**: Safe script execution with proper cleanup
- **Logging System**: Comprehensive logging with multiple severity levels
- **Error Handling**: Robust exception handling and graceful error recovery

## 🏗️ Architecture

Built with modern C++17 using:
- **MVC Pattern**: Clear separation of concerns
- **Observer Pattern**: Real-time UI updates
- **RAII**: Automatic resource management
- **Thread Safety**: Concurrent script execution and output capture

### Core Components

- `ScriptRunner`: Main application controller
- `UIManager`: ncurses-based terminal interface
- `ProcessManager`: Script execution and output capture
- `Logger`: Thread-safe logging system
- `Exceptions`: Custom exception hierarchy

## 🔧 Build Instructions

```bash
# Prerequisites: CMake 3.16+, ncurses development libraries
sudo apt-get install cmake libncurses5-dev

# Build
mkdir build && cd build
cmake ..
make

# Run
./bin/script_runner
```

## 🎮 Usage

### Keyboard Controls
- `↑/↓` or `j/k`: Navigate script list
- `Enter`: Execute selected script
- `Tab`: Switch between panes
- `PageUp/PageDown`: Scroll output
- `r`: Refresh script list
- `c`: Clear output panes
- `h`: Show help
- `q`: Quit application

### Command Line Options
```bash
./script_runner --help     # Show help
./script_runner --version  # Show version
./script_runner -d DIR     # Use custom scripts directory
```

## 📁 Project Structure

```
├── src/                   # Source code
│   ├── main.cpp          # Application entry point
│   ├── ScriptRunner.cpp  # Main controller
│   ├── UIManager.cpp     # Terminal interface
│   ├── ProcessManager.cpp # Script execution
│   ├── Logger.cpp        # Logging system
│   └── Exceptions.cpp    # Error handling
├── include/              # Header files
├── examples/             # Sample scripts
├── docs/                 # Documentation
├── build/                # Build artifacts
└── CMakeLists.txt       # Build configuration
```

## 🧪 Testing

The application has been tested with:
- ✅ Command line argument parsing
- ✅ Terminal UI initialization and layout
- ✅ Script discovery and listing
- ✅ Real-time output capture
- ✅ Keyboard navigation and controls
- ✅ Process management and cleanup
- ✅ Error handling and recovery

## 📖 Documentation

- [API Reference](docs/api-reference.md) - Complete API documentation
- [Architecture Guide](docs/architecture.md) - System design and patterns
- [User Guide](docs/user-guide.md) - Detailed usage instructions

## 🎯 Use Cases

Perfect for:
- **Development Workflows**: Running build scripts, tests, and automation
- **System Administration**: Executing maintenance and monitoring scripts
- **Learning Environment**: Educational tool for script management
- **GitHub Copilot Development**: Enhanced terminal-based development workflow

## 🔮 Future Enhancements

Potential improvements:
- Configuration file support
- Script templates and creation wizard
- Output filtering and search
- Script scheduling and automation
- Remote script execution
- Plugin system for custom actions

## 📝 Implementation Notes

This project was developed as a comprehensive example of:
- Modern C++ best practices
- Terminal UI development with ncurses
- Concurrent programming and thread safety
- Cross-platform build systems with CMake
- GitHub Copilot-assisted development workflow

The codebase demonstrates clean architecture, comprehensive error handling, and production-ready code quality suitable for both educational and practical use.

---

**Built with ❤️ using C++17, ncurses, and GitHub Copilot**
