# C++ Terminal Script Runner

A sophisticated terminal-based application for simultaneous execution and monitoring of scripts with real-time output visualization.

## Quick Start

```bash
# Build the application
mkdir build && cd build
cmake ..
make

# Run the script runner
./script_runner
```

## Overview

The Script Runner provides a powerful terminal interface for managing multiple script executions simultaneously. It features:

- **Dual Execution**: Run two scripts concurrently with live output display
- **Interactive Navigation**: Keyboard-driven interface with intuitive controls
- **Real-time Output**: Live streaming of script outputs in separate panes
- **Process Management**: Full control over script lifecycle (start, stop, monitor)
- **Auto-Discovery**: Automatically finds and lists executable scripts

## Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   ScriptRunner  │────│   UIManager     │────│ Terminal Display│
│  (Controller)   │    │    (View)       │    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │
         │              ┌─────────────────┐
         └──────────────│ ProcessManager  │
                        │    (Model)      │
                        └─────────────────┘
```

### Core Components

- **ScriptRunner**: Main application controller coordinating all subsystems
- **UIManager**: Terminal interface management using ncurses
- **ProcessManager**: Script execution and output capture engine
- **Logger**: Comprehensive logging system for debugging and monitoring

## User Interface

### Layout
```
┌─ Script List (30%) ─┬─ Output Pane 1 (35%) ─┬─ Output Pane 2 (35%) ─┐
│ [1] script1.py      │ $ python script1.py    │ $ bash script2.sh      │
│ [2] script2.sh      │ Starting process...     │ Initializing...        │
│ [3] build.sh        │ Output line 1          │ Processing files...     │
│ [4] test.py         │ Output line 2          │ Status: Running        │
│ [5] deploy.sh       │ ...                    │ ...                    │
├─────────────────────┼────────────────────────┼────────────────────────┤
│ Status: 2 running   │ Scroll: ↑↓            │ Scroll: ↑↓            │
└─────────────────────┴────────────────────────┴────────────────────────┘
```

### Keyboard Controls

#### Navigation
- `↑/k` - Move up in script list
- `↓/j` - Move down in script list
- `←/h` - Focus left pane
- `→/l` - Focus right pane
- `Tab` - Cycle through panes
- `Shift+Tab` - Reverse cycle

#### Execution
- `Enter` - Execute selected script in active pane
- `Space` - Execute script in available pane (auto-select)
- `Ctrl+C` - Terminate script in active pane
- `Ctrl+Shift+C` - Terminate all running scripts
- `r` - Refresh script list

#### View Control
- `PageUp/Ctrl+U` - Scroll output up
- `PageDown/Ctrl+D` - Scroll output down
- `Home` - Jump to beginning of output
- `End` - Jump to end of output
- `Ctrl+L` - Clear active output pane

#### Application
- `F1/?` - Show/hide help
- `Ctrl+Q/q` - Quit application
- `Ctrl+R` - Refresh UI (recovery mode)

## Features

### Script Discovery
- Automatically scans parent `scripts/` directory
- Detects executable files by permissions
- Supports multiple script types:
  - Python (`.py`)
  - Bash/Shell (`.sh`)
  - JavaScript (`.js`)
  - Ruby (`.rb`)
  - Perl (`.pl`)
  - Compiled executables

### Process Management
- **Concurrent Execution**: Run up to 2 scripts simultaneously
- **Real-time Output**: Live streaming with automatic scrolling
- **Process Control**: Start, stop, and monitor script lifecycle
- **Output History**: Scrollable buffer with configurable size
- **Error Handling**: Graceful failure recovery and status reporting

### Performance
- **Responsive UI**: < 16ms update cycles (60 FPS)
- **Low Latency**: < 10ms keyboard input response
- **Resource Efficient**: < 50MB memory usage under normal operation
- **Scalable**: Supports 100+ scripts in discovery list

## Technical Details

### Build Requirements
- **C++17** or later
- **CMake 3.16+**
- **ncurses** library
- **pthread** support

### Dependencies
- Standard C++ library
- ncurses for terminal UI
- POSIX threads for concurrency
- System process management APIs

### Threading Model
- **Main Thread**: UI event loop and user interaction
- **Output Threads**: One per running script for output capture
- **Cleanup Thread**: Periodic resource management

### Error Handling
- Exception-safe design with RAII principles
- Graceful recovery from UI corruption
- Clean shutdown on signal interruption
- Comprehensive logging for debugging

## Development

### Project Structure
```
scripts/script_runner/
├── .copilot-context.md     # Comprehensive specifications
├── README.md               # This file
├── CMakeLists.txt          # Build configuration
├── src/                    # Source code
│   ├── main.cpp           # Application entry point
│   ├── ScriptRunner.*     # Main controller
│   ├── UIManager.*        # Terminal interface
│   ├── ProcessManager.*   # Script execution
│   └── Logger.*           # Logging system
├── include/               # Header files
├── docs/                  # Documentation
│   ├── architecture.md   # System design
│   ├── api-reference.md  # API documentation
│   └── user-guide.md     # User manual
└── examples/             # Usage examples
```

### Building from Source

```bash
# Clone and navigate to project
cd scripts/script_runner

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the application
make -j$(nproc)

# Optional: Install system-wide
sudo make install
```

### Development Guidelines

This project is designed for **GitHub Copilot-driven development**. The `.copilot-context.md` file contains extremely detailed specifications enabling context-free implementation of all components.

#### Key Principles
- **RAII**: Resource Acquisition Is Initialization throughout
- **Thread Safety**: Mutex protection for all shared data
- **Exception Safety**: Strong exception guarantees
- **Performance**: Sub-millisecond response times for UI operations

#### Code Style
- Modern C++17 features preferred
- Smart pointers for memory management
- STL containers and algorithms
- Comprehensive documentation with cross-references

## Integration

### Parent Project Integration
Add to your parent `CMakeLists.txt`:

```cmake
# Add script runner subdirectory
add_subdirectory(scripts/script_runner)

# Optional: Create run target
add_custom_target(run_scripts
    COMMAND $<TARGET_FILE:script_runner>
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Launch script runner"
)
```

### Usage in CI/CD
```bash
# Run specific scripts through the runner
./script_runner --execute build.sh test.py

# Batch mode for automation
./script_runner --batch --timeout 300 deploy.sh
```

## Troubleshooting

### Common Issues

**UI Corruption**
- Press `Ctrl+R` to refresh the interface
- Ensure terminal supports color and ncurses

**Script Not Found**
- Check script permissions (executable bit)
- Verify script is in parent scripts directory
- Use `r` to refresh script list

**Performance Issues**
- Monitor CPU/memory usage in status bar
- Reduce output buffer size if needed
- Check for memory leaks in long-running sessions

**Build Errors**
- Ensure ncurses development packages are installed
- Verify C++17 compiler support
- Check CMake version (3.16+ required)

### Debug Mode
Enable debug logging:
```bash
export SCRIPT_RUNNER_LOG_LEVEL=DEBUG
./script_runner
```

## Contributing

This project uses GitHub Copilot for development. When contributing:

1. **Read Specifications**: Review `.copilot-context.md` thoroughly
2. **Follow Architecture**: Maintain MVC pattern and threading model
3. **Add Documentation**: Update specifications for new features
4. **Test Thoroughly**: Include unit and integration tests
5. **Performance**: Maintain sub-millisecond UI response times

## License

[Add your license information here]

## Support

For issues, feature requests, or questions:
- Check the troubleshooting section above
- Review the comprehensive specifications in `.copilot-context.md`
- Examine the architecture documentation in `docs/`

---

**Note**: This application is designed for terminal environments and requires ncurses support. It has been optimized for development workflows and script automation tasks.
