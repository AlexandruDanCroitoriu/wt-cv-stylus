# Script Runner Examples

This directory contains example code and usage scenarios for the C++ Terminal Script Runner.

## Quick Start Example

### Basic Usage
```bash
# Build and run the script runner
cd script_runner
mkdir build && cd build
cmake ..
make
./script_runner
```

### Example Scripts Directory Structure
```
parent_project/
├── scripts/
│   ├── script_runner/          # This application
│   ├── build.sh               # Build script
│   ├── test.py                # Test script
│   ├── deploy.sh              # Deployment script
│   └── monitoring/
│       ├── health_check.py
│       └── log_analyzer.sh
```

## Example Integration

### CMake Integration
Add to your parent project's CMakeLists.txt:
```cmake
# Add script runner as subdirectory
add_subdirectory(scripts/script_runner)

# Create convenience target
add_custom_target(scripts
    COMMAND $<TARGET_FILE:script_runner>
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Launch script runner interface"
    DEPENDS script_runner
)
```

### Environment Setup
```bash
# Set up environment for script runner
export SCRIPT_RUNNER_LOG_LEVEL=INFO
export SCRIPT_RUNNER_BUFFER_SIZE=2000
export SCRIPT_RUNNER_SCRIPTS_DIR=../scripts

# Run with custom configuration
./script_runner
```

## Development Workflow Examples

### 1. Build and Test Workflow
Use the script runner to monitor:
- `build.sh` in one pane
- `test.py` in another pane
- Watch both outputs simultaneously

### 2. CI/CD Pipeline Monitoring
Monitor multiple deployment stages:
- `deploy_staging.sh`
- `run_integration_tests.py`
- Real-time status updates

### 3. Log Analysis and Monitoring
Run analysis scripts:
- `log_analyzer.sh` for error detection
- `performance_monitor.py` for metrics
- `health_check.py` for system status

## Sample Scripts

See the `sample_scripts/` directory for example scripts that work well with the runner:
- Python data processing scripts
- Shell automation scripts
- Monitoring and health check scripts
- Build and deployment scripts

## Best Practices

1. **Script Output**: Use informative logging with timestamps
2. **Error Handling**: Implement proper exit codes and error messages
3. **Resource Management**: Clean up temporary files and processes
4. **Progress Reporting**: Provide status updates for long-running operations

## Troubleshooting Examples

### UI Recovery
```bash
# If UI becomes corrupted, try:
# 1. Press Ctrl+R to refresh
# 2. If that fails, restart application
# 3. Check terminal compatibility
```

### Process Management
```bash
# Check for orphaned processes
ps aux | grep script_runner

# Clean up if needed
killall -TERM script_runner
```

For more detailed examples, see the individual files in this directory.
