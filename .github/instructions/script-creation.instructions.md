---
applyTo: "scripts/**"
---

# Script Creation Instructions - Wt CV Stylus Project

## Overview
This document provides comprehensive guidelines for creating new scripts in the Wt CV Stylus project. All scripts must follow these standards to ensure consistency, maintainability, and proper integration with the existing script ecosystem.

## Mandatory Script Structure Template

All scripts in the `scripts/` directory MUST follow this exact template:

```bash
#!/bin/bash
# Script to [description]
# Usage: ./scripts/script-name.sh [options]

set -e  # Exit on any error

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="$SCRIPT_DIR/output"
LOG_FILE="$OUTPUT_DIR/script-name.log"

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Clear the log file for this run
> "$LOG_FILE"

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Logging functions (REQUIRED in every script)
print_status() {
    local msg="$1"
    echo -e "${BLUE}[INFO]${NC} $msg"
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] [INFO] $msg" >> "$LOG_FILE"
}

print_success() {
    local msg="$1"
    echo -e "${GREEN}[SUCCESS]${NC} $msg"
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] [SUCCESS] $msg" >> "$LOG_FILE"
}

print_warning() {
    local msg="$1"
    echo -e "${YELLOW}[WARNING]${NC} $msg"
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] [WARNING] $msg" >> "$LOG_FILE"
}

print_error() {
    local msg="$1"
    echo -e "${RED}[ERROR]${NC} $msg"
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] [ERROR] $msg" >> "$LOG_FILE"
}

# Colorized help function (REQUIRED)
show_usage() {
    echo -e "${BOLD}${BLUE}Usage:${NC} $0 [options]"
    echo ""
    echo -e "${BOLD}${GREEN}Description:${NC}"
    echo "  [Describe what the script does]"
    echo ""
    echo -e "${BOLD}${YELLOW}Options:${NC}"
    echo -e "  ${CYAN}-h, --help${NC}    Show this help message"
    echo ""
    echo -e "${BOLD}${YELLOW}Examples:${NC}"
    echo -e "  ${GREEN}$0${NC}            # [Example usage]"
}

# Argument parsing (REQUIRED)
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    show_usage
    exit 0
fi

# Main script logic goes here
print_status "Starting [script name]..."
# ... implement functionality ...
print_success "[Script name] completed successfully!"
```

## Script Integration Philosophy

### Scripts Should Use Other Scripts
- **Primary Rule:** Always prefer calling existing scripts over duplicating functionality
- **Example:** `run.sh` calls `build.sh` when build is needed instead of implementing build logic
- **Benefits:** Consistency, maintainability, single source of truth

### Integration Patterns
```bash
# Calling other scripts - Full path required
"$SCRIPT_DIR/build.sh" "--debug"

# Passing arguments through
"$SCRIPT_DIR/other-script.sh" "$@"

# Conditional script calling
if ! check_condition; then
    if "$SCRIPT_DIR/setup-script.sh"; then
        print_success "Setup completed"
    else
        print_error "Setup failed"
        exit 1
    fi
fi
```

## Naming Conventions

- **Use kebab-case:** `script-name.sh`
- **Be descriptive:** `memory-analyzer.sh` not `mem.sh`
- **Use prefixes for related scripts:** `build-*.sh`, `test-*.sh`
- **Core scripts use simple names:** `build.sh`, `run.sh`

## Logging Requirements

- **ALL output MUST be logged** to `scripts/output/[script-name].log`
- **Log file named after script:** `build.sh` → `build.log`
- **Clear log file at start:** `> "$LOG_FILE"`
- **Use timestamped log entries:** `[YYYY-MM-DD HH:MM:SS] [LEVEL] message`
- **Support both console and file output** using the provided logging functions

## Color Standards

- **RED** (`\033[0;31m`): Errors and failures
- **GREEN** (`\033[0;32m`): Success messages
- **YELLOW** (`\033[1;33m`): Warnings and important info
- **BLUE** (`\033[0;34m`): General info/status
- **CYAN** (`\033[0;36m`): Commands, flags, parameters
- **BOLD** (`\033[1m`): Headers and emphasis

## Error Handling Standards

- **Use `set -e`** to exit on any error
- **Always check command success** with proper error messages
- **Return appropriate exit codes** (0=success, 1=error)
- **Log errors before exiting**
- **Provide helpful error messages** with suggested solutions

## Permission Handling Standards

### Core Principles
- **Always ask for permissions when needed:** Scripts should never fail due to missing permissions
- **Graceful sudo detection:** Try without sudo first, then with sudo if needed
- **Clear permission feedback:** Inform users when sudo is required and why
- **Smart permission handling:** Detect if commands need elevated privileges and adapt accordingly
- **User-friendly prompts:** Provide helpful messages when requesting password input

### Permission Handling Patterns

#### Docker Permission Detection
```bash
need_sudo_for_docker() {
    if docker info &> /dev/null; then
        return 1  # No sudo needed
    elif sudo docker info &> /dev/null 2>&1; then
        return 0  # Sudo needed
    else
        print_error "Cannot access Docker even with sudo. Please check Docker installation."
        exit 1
    fi
}

# Usage in Docker scripts
if need_sudo_for_docker; then
    print_warning "Docker requires sudo privileges for this user"
    print_status "You may be prompted for your password"
    DOCKER_CMD="sudo docker"
else
    DOCKER_CMD="docker"
fi

$DOCKER_CMD build -t image_name .
```

#### System Package Installation
```bash
check_sudo() {
    if [ "$EUID" -eq 0 ]; then
        print_status "Running as root user"
        return 0
    fi
    
    if ! sudo -n true 2>/dev/null; then
        print_status "This script requires sudo privileges for package installation."
        print_status "Please enter your password when prompted."
        sudo -v || {
            print_error "Cannot obtain sudo privileges. Exiting."
            exit 1
        }
    fi
}

# Usage in installation scripts
check_sudo
sudo apt-get update
sudo apt-get install -y package-name
```

#### Environment Detection
```bash
# Smart environment detection for Docker containers vs host systems
detect_environment() {
    if [ -f /.dockerenv ] || grep -q 'docker\|lxc' /proc/1/cgroup 2>/dev/null; then
        echo "docker"
    else
        echo "host"
    fi
}

# Adapt behavior based on environment
ENVIRONMENT=$(detect_environment)
if [ "$ENVIRONMENT" = "docker" ]; then
    # Docker containers typically run as root
    APT_CMD="apt-get"
else
    # Host systems need sudo
    check_sudo
    APT_CMD="sudo apt-get"
fi
```

## Build/Run Script Patterns

### For Build Scripts
```bash
# Check dependencies
if ! command -v cmake &> /dev/null; then
    print_error "cmake is not installed"
    exit 1
fi

# Use parallel builds
cpu_cores=$(nproc)
make -j"$cpu_cores"
```

### For Run Scripts
```bash
# Check if app is built
if [ ! -f "$BUILD_DIR/app" ]; then
    print_warning "Application not built. Building now..."
    "$SCRIPT_DIR/build.sh" "--debug"
fi

# Use make run from build directory
cd "$BUILD_DIR"
make run
```

### For Port Management
```bash
# Kill existing instances on same port
if lsof -ti:9020 >/dev/null 2>&1; then
    print_warning "Killing existing process on port 9020"
    lsof -ti:9020 | xargs kill -9
fi
```

## Application Settings and Integration Points

### Directory Structure
- **All scripts in:** `scripts/`
- **All logs in:** `scripts/output/`
- **Build directories:** `build/debug/`, `build/release/`
- **Configuration:** Project root `wt_config.xml`

### Application Settings
- **Default port:** 9020
- **Default address:** 0.0.0.0
- **Document root:** `../../` (relative to build directory)
- **Config file:** `../../wt_config.xml` (relative to build directory)

### Script Dependencies
- `run.sh` → `build.sh` (auto-build functionality)
- `docker-r-wt-cv-stylus.sh` → `docker-b-wt-builder.sh` (dependency image)
- All scripts → logging to `scripts/output/`
- All scripts → color standards and error handling
- Build scripts → CMake, make
- Run scripts → `make run` target
- Docker scripts → Docker daemon, Dockerfiles in `dockerfiles/`

## Script Categories and Examples

### Core Build and Run Scripts
- **Purpose:** Primary development workflow
- **Examples:** `build.sh`, `run.sh`
- **Characteristics:** Simple names, essential functionality, called by other scripts

### Utility Scripts
- **Purpose:** Supporting development tasks
- **Examples:** `memory-analyzer.sh`, `clone-libraries.sh`
- **Characteristics:** Descriptive names, specific functionality, logging to output

### System Setup Scripts
- **Purpose:** Environment preparation and dependency management
- **Examples:** `ubuntu-install-dependencies.sh`
- **Characteristics:** Permission-aware, environment detection, dry-run support

### Docker Scripts
- **Purpose:** Container management and deployment
- **Examples:** `docker-b-wt-builder.sh`, `docker-r-wt-cv-stylus.sh`
- **Characteristics:** Smart permission detection, dependency checking, image lifecycle

## Best Practices

### Script Reusability
- **Design scripts to be called by other scripts**
- **Use clear, consistent argument patterns**
- **Provide both interactive and non-interactive modes when appropriate**
- **Log operations for debugging and audit trails**

### Maintenance
- **Keep `.copilot-context.md` updated when adding new scripts**
- **Regular review of script interdependencies**
- **Consistent naming and structure across all scripts**
- **Document any breaking changes or new requirements**

### Security
- **Never hardcode sensitive information**
- **Validate input parameters**
- **Use appropriate permission levels**
- **Log security-relevant operations**

## Development Workflow

1. **Copy template:** Start with the standard template above
2. **Customize:** Update script name, description, and logic
3. **Integrate:** Use existing scripts where possible instead of reimplementing
4. **Test:** Verify logging, error handling, and help output
5. **Document:** Update `.copilot-context.md` if adding new core functionality
6. **Make executable:** `chmod +x script-name.sh`

## Validation Checklist

Before submitting a new script, ensure:

- [ ] Follows the mandatory template structure
- [ ] Implements all required logging functions
- [ ] Has proper error handling with `set -e`
- [ ] Includes colorized help function
- [ ] Uses appropriate permission handling if needed
- [ ] Logs to `scripts/output/[script-name].log`
- [ ] Uses existing scripts instead of reimplementing functionality
- [ ] Follows naming conventions
- [ ] Is executable (`chmod +x`)
- [ ] Documentation updated in `.copilot-context.md` if needed

This comprehensive guide ensures all new scripts maintain consistency with the established patterns and integrate seamlessly with the existing script ecosystem.
