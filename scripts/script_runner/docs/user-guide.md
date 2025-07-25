# User Guide - C++ Terminal Script Runner

## Table of Contents
1. [Getting Started](#getting-started)
2. [Installation](#installation)
3. [Basic Usage](#basic-usage)
4. [Interface Overview](#interface-overview)
5. [Keyboard Reference](#keyboard-reference)
6. [Advanced Features](#advanced-features)
7. [Configuration](#configuration)
8. [Troubleshooting](#troubleshooting)
9. [Tips and Best Practices](#tips-and-best-practices)

## Getting Started

### Quick Start
1. Navigate to the script runner directory
2. Build the application: `mkdir build && cd build && cmake .. && make`
3. Run: `./script_runner`
4. Use arrow keys to select a script, press Enter to execute

### System Requirements
- Linux/Unix terminal with ncurses support
- C++17 compatible compiler (GCC 7+, Clang 5+)
- CMake 3.16 or later
- ncurses development libraries
- Minimum 80x24 terminal size (recommended: 120x30+)

## Installation

### From Source
```bash
# Clone or navigate to project directory
cd scripts/script_runner

# Create build directory
mkdir build && cd build

# Configure build
cmake ..

# Compile (use -j for parallel compilation)
make -j$(nproc)

# Optional: Install system-wide
sudo make install
```

### Dependencies Installation

**Ubuntu/Debian**:
```bash
sudo apt update
sudo apt install build-essential cmake libncurses5-dev libncursesw5-dev
```

**RHEL/CentOS/Fedora**:
```bash
sudo dnf install gcc-c++ cmake ncurses-devel
# or for older versions: sudo yum install gcc-c++ cmake ncurses-devel
```

**macOS** (with Homebrew):
```bash
brew install cmake ncurses
```

### Verification
After installation, verify the build:
```bash
# Run basic functionality test
./script_runner --version

# Test script discovery
./script_runner --list-scripts
```

## Basic Usage

### Starting the Application
```bash
# Run from build directory
./script_runner

# Run from any directory (if installed)
script_runner

# Run with debug logging
SCRIPT_RUNNER_LOG_LEVEL=DEBUG ./script_runner
```

### First Steps
1. **Script Discovery**: The application automatically scans the parent `scripts/` directory
2. **Navigation**: Use arrow keys (↑↓) to browse available scripts
3. **Execution**: Press Enter to run the selected script in the active output pane
4. **Monitoring**: Watch real-time output in the right panes
5. **Control**: Use Ctrl+C to stop running scripts, Ctrl+Q to quit

### Basic Workflow
```
Select Script → Execute → Monitor Output → Control Process → Repeat
     ↑↓           Enter      Watch            Ctrl+C         q/Ctrl+Q
```

## Interface Overview

### Layout Structure
```
┌─ Script List ─────┬─ Output Pane 1 ─────┬─ Output Pane 2 ─────┐
│                   │                     │                     │
│ Available Scripts │   Active Process    │   Active Process    │
│ [Navigation]      │     [Output]        │     [Output]        │
│                   │                     │                     │
├───────────────────┼─────────────────────┼─────────────────────┤
│ Status Information│ Scroll Indicators   │ Scroll Indicators   │
└───────────────────┴─────────────────────┴─────────────────────┘
```

### Pane Functions

#### Script List Pane (Left, 30%)
- **Purpose**: Browse and select executable scripts
- **Content**: 
  - Script names with indices [1], [2], etc.
  - Execution status indicators
  - File type icons/extensions
- **Highlighting**: Selected script shown in yellow
- **Status Colors**:
  - White: Available
  - Green: Currently running
  - Red: Recently failed
  - Gray: Not executable

#### Output Panes (Right, 70% split)
- **Purpose**: Display real-time script output
- **Content**:
  - Command line shown at top
  - stdout and stderr combined
  - Timestamps for each line (optional)
  - Process status indicators
- **Scrolling**: Automatic scroll to bottom, manual scroll available
- **Colors**:
  - Normal output: White
  - Error messages: Red
  - System messages: Cyan
  - Timestamps: Gray

#### Status Bar (Bottom)
- **Left**: Script count, running processes, current time
- **Center**: Active process information, error notifications
- **Right**: System resources (memory, CPU), uptime

### Visual Indicators

#### Process Status
- `●` Running (green)
- `○` Idle (white)
- `✗` Error (red)
- `✓` Completed successfully (green)
- `⏸` Paused/Stopped (yellow)

#### Scroll Indicators
- `↑` Can scroll up
- `↓` Can scroll down
- `↕` Can scroll both directions
- `─` At boundary/no scroll needed

## Keyboard Reference

### Navigation Controls
| Key | Action | Description |
|-----|--------|-------------|
| `↑` or `k` | Move Up | Navigate up in script list |
| `↓` or `j` | Move Down | Navigate down in script list |
| `←` or `h` | Focus Left | Move focus to script list |
| `→` or `l` | Focus Right | Move focus to output panes |
| `Tab` | Cycle Forward | Script List → Output1 → Output2 → Script List |
| `Shift+Tab` | Cycle Backward | Reverse of Tab cycling |
| `Home` | First Item | Jump to first script or beginning of output |
| `End` | Last Item | Jump to last script or end of output |

### Execution Controls
| Key | Action | Description |
|-----|--------|-------------|
| `Enter` | Execute | Run selected script in active output pane |
| `Space` | Auto Execute | Run script in next available pane |
| `Ctrl+C` | Terminate | Stop script in active pane |
| `Ctrl+Shift+C` | Terminate All | Stop all running scripts |
| `r` or `F5` | Refresh | Refresh script list from filesystem |

### View Controls
| Key | Action | Description |
|-----|--------|-------------|
| `PageUp` or `Ctrl+U` | Scroll Up | Scroll output pane up |
| `PageDown` or `Ctrl+D` | Scroll Down | Scroll output pane down |
| `Ctrl+Home` | Top | Jump to beginning of output |
| `Ctrl+End` | Bottom | Jump to end of output |
| `Ctrl+L` | Clear | Clear active output pane |

### Application Controls
| Key | Action | Description |
|-----|--------|-------------|
| `F1` or `?` | Help | Show/hide help overlay |
| `Ctrl+Q` or `q` | Quit | Exit application |
| `Ctrl+R` | Refresh UI | Recover from UI corruption |
| `Ctrl+S` | Save Logs | Save current session logs |

### Advanced Controls
| Key | Action | Description |
|-----|--------|-------------|
| `1`-`9` | Direct Select | Jump to script by number |
| `Ctrl+1`, `Ctrl+2` | Switch Pane | Direct pane selection |
| `Ctrl+P` | Process List | Show running processes |
| `Ctrl+M` | Memory Info | Display memory usage |

## Advanced Features

### Script Discovery Rules

The application discovers scripts using these criteria:

1. **Location**: Searches parent `scripts/` directory recursively
2. **Permissions**: Must have executable bit set (`chmod +x`)
3. **File Types**: Recognizes common script extensions:
   - `.py` - Python scripts (executed with `python3`)
   - `.sh` - Shell scripts (executed with `bash`)
   - `.js` - JavaScript (executed with `node`)
   - `.rb` - Ruby scripts (executed with `ruby`)
   - `.pl` - Perl scripts (executed with `perl`)
   - No extension - Direct execution (compiled binaries)

### Automatic Command Building

The application automatically determines how to execute scripts:

```bash
# Python script
script.py → python3 script.py

# Shell script
script.sh → bash script.sh

# Executable binary
compiled_program → ./compiled_program

# With arguments (future feature)
script.py arg1 arg2 → python3 script.py arg1 arg2
```

### Output Management

#### Buffer Management
- **Size**: Configurable buffer size (default: 1000 lines per pane)
- **Overflow**: Oldest lines discarded when buffer full
- **Persistence**: Output saved to temporary files for session recovery

#### Real-time Updates
- **Streaming**: Live output display as scripts generate it
- **Rate Limiting**: UI updates throttled to maintain responsiveness
- **Batching**: Multiple output lines batched for efficient rendering

#### Search and Filter (Future)
- **Search**: Find text in output history
- **Filter**: Show only error lines, warnings, etc.
- **Export**: Save output to files

### Process Management

#### Concurrent Execution
- **Limit**: Maximum 2 scripts running simultaneously
- **Queueing**: Additional script requests queue until slot available
- **Priority**: User can specify execution priority

#### Process Control
- **Signals**: Graceful termination (SIGTERM) before force kill (SIGKILL)
- **Timeout**: Configurable maximum execution time
- **Resource Limits**: Memory and CPU usage monitoring

#### Status Tracking
- **Runtime**: Track execution time for each script
- **Exit Codes**: Display and log process exit status
- **Resource Usage**: Monitor CPU and memory consumption

## Configuration

### Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `SCRIPT_RUNNER_LOG_LEVEL` | Logging verbosity (DEBUG, INFO, WARNING, ERROR) | INFO |
| `SCRIPT_RUNNER_BUFFER_SIZE` | Output buffer lines per pane | 1000 |
| `SCRIPT_RUNNER_SCRIPTS_DIR` | Override script discovery directory | ../scripts |
| `SCRIPT_RUNNER_CONFIG_FILE` | Configuration file path | ~/.script_runner.conf |

### Command Line Options

```bash
# Display version information
./script_runner --version

# List discovered scripts without starting UI
./script_runner --list-scripts

# Execute specific script immediately
./script_runner --execute script.py

# Batch mode (no interactive UI)
./script_runner --batch script1.sh script2.py

# Debug mode with verbose logging
./script_runner --debug

# Custom scripts directory
./script_runner --scripts-dir /path/to/scripts
```

### Configuration File (Future)

Create `~/.script_runner.conf`:
```ini
[ui]
color_scheme=dark
refresh_rate=60
auto_scroll=true

[execution]
max_processes=2
default_timeout=300
auto_restart=false

[logging]
level=INFO
file_path=~/.script_runner.log
max_file_size=10MB
```

## Troubleshooting

### Common Issues

#### UI Display Problems

**Symptom**: Garbled display, missing borders, incorrect colors
**Solutions**:
1. Check terminal color support: `echo $TERM`
2. Try different terminal: xterm, gnome-terminal, konsole
3. Refresh UI with `Ctrl+R`
4. Restart application

**Symptom**: Text overlap, incorrect layout
**Solutions**:
1. Ensure minimum terminal size (80x24)
2. Resize terminal and press `Ctrl+R`
3. Check for terminal font issues

#### Script Discovery Problems

**Symptom**: Scripts not appearing in list
**Solutions**:
1. Verify script location: `ls -la ../scripts/`
2. Check execute permissions: `chmod +x script.py`
3. Refresh list with `r` key
4. Check parent directory structure

**Symptom**: Wrong interpreter used
**Solutions**:
1. Verify shebang line: `#!/usr/bin/env python3`
2. Check file extension matches content
3. Make script directly executable

#### Performance Issues

**Symptom**: Slow UI response, high CPU usage
**Solutions**:
1. Reduce output buffer size
2. Check for runaway processes
3. Monitor memory usage in status bar
4. Restart application

**Symptom**: Output lag, missing lines
**Solutions**:
1. Check output rate of scripts
2. Increase buffer size if needed
3. Use output redirection in scripts
4. Monitor system resources

#### Process Execution Problems

**Symptom**: Scripts fail to start
**Solutions**:
1. Check script syntax: `python3 -m py_compile script.py`
2. Verify dependencies installed
3. Check file permissions
4. Review error messages in output pane

**Symptom**: Processes don't terminate
**Solutions**:
1. Use `Ctrl+Shift+C` to force terminate all
2. Check for background processes
3. Review script cleanup procedures
4. Restart application if needed

### Debug Mode

Enable comprehensive debugging:
```bash
# Set debug environment
export SCRIPT_RUNNER_LOG_LEVEL=DEBUG

# Run with debug output
./script_runner 2>debug.log

# Monitor debug output in another terminal
tail -f debug.log
```

### Log Analysis

Check log files for detailed information:
```bash
# Application logs
tail -f ~/.script_runner.log

# System logs for process information
journalctl -f | grep script_runner

# Check for memory leaks
valgrind --leak-check=full ./script_runner
```

### Recovery Procedures

#### UI Corruption Recovery
1. Press `Ctrl+R` to refresh interface
2. If unsuccessful, quit with `Ctrl+Q` and restart
3. Check terminal compatibility
4. Try different terminal emulator

#### Process Cleanup
1. List running processes: `ps aux | grep script_runner`
2. Kill orphaned processes: `kill -TERM <pid>`
3. Clean temporary files: `rm -f /tmp/script_runner_*`
4. Reset terminal: `reset`

## Tips and Best Practices

### Efficient Workflow

#### Script Organization
- Use descriptive script names
- Group related scripts in subdirectories
- Add comments and documentation
- Use consistent naming conventions

#### Monitoring Multiple Scripts
- Use different panes for related tasks
- Monitor resource usage in status bar
- Use meaningful output messages
- Implement proper error handling

#### Performance Optimization
- Limit output verbosity in scripts
- Use buffering for high-volume output
- Clean up temporary files in scripts
- Monitor memory usage regularly

### Script Development

#### Output Best Practices
```bash
# Good: Informative messages
echo "Processing file: $filename"
echo "Progress: $current/$total ($percent%)"

# Good: Error handling
if ! command_here; then
    echo "ERROR: Command failed" >&2
    exit 1
fi

# Good: Status updates
echo "$(date): Starting backup process"
```

#### Error Handling
```python
#!/usr/bin/env python3
import sys

try:
    # Script logic here
    pass
except Exception as e:
    print(f"ERROR: {e}", file=sys.stderr)
    sys.exit(1)
```

#### Resource Management
```bash
#!/bin/bash
# Set timeout for long-running operations
timeout 300 long_running_command || {
    echo "ERROR: Command timed out after 5 minutes"
    exit 1
}
```

### Advanced Usage

#### Keyboard Efficiency
- Learn vi-style navigation (`hjkl`)
- Use number keys for quick script selection
- Master pane switching with `Tab`/`Shift+Tab`
- Use `Ctrl+C` → `Enter` for quick restart

#### Monitoring Techniques
- Watch status bar for resource usage
- Use scroll indicators to navigate large outputs
- Monitor multiple related scripts simultaneously
- Use clear/refresh commands to manage clutter

#### Integration with Development Workflow
- Organize scripts by project phase (build, test, deploy)
- Use script runner for CI/CD pipeline visualization
- Monitor long-running builds and tests
- Quick script iteration and testing

### Customization Ideas (Future)

#### Color Schemes
- Create custom color configurations
- Terminal-specific optimizations
- High-contrast modes for accessibility

#### Layout Modifications
- Adjust pane ratios for specific use cases
- Custom status bar information
- Floating windows for temporary information

#### Script Integration
- Auto-execute dependent scripts
- Script parameter input
- Output post-processing and filtering

---

This user guide provides comprehensive information for effectively using the C++ Terminal Script Runner. For technical details, refer to the architecture documentation and API reference.
