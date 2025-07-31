#!/bin/bash

# Script to clone external libraries without creating git submodules
# Usage: ./clone_libraries.sh

set -e  # Exit on any error

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
LIBS_DIR="$PROJECT_ROOT/libs"
OUTPUT_DIR="$SCRIPT_DIR/output"
LOG_FILE="$OUTPUT_DIR/clone-libraries.log"

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Clear the log file for this run
> "$LOG_FILE"

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
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

# Function to clone a library and remove .git directory
clone_library() {
    local repo_url="$1"
    local lib_name="$2"
    local branch="$3"
    
    print_status "Cloning $lib_name..."
    
    # Create libs directory if it doesn't exist
    mkdir -p "$LIBS_DIR"
    
    # Change to libs directory
    cd "$LIBS_DIR"
    
    # Check if library already exists
    if [ -d "$lib_name" ]; then
        print_warning "$lib_name already exists. Skipping..."
        return 0
    fi
    
    # Clone the repository
    if [ -n "$branch" ]; then
        git clone --branch "$branch" "$repo_url" "$lib_name" 2>&1 | tee -a "$LOG_FILE"
    else
        git clone "$repo_url" "$lib_name" 2>&1 | tee -a "$LOG_FILE"
    fi
    
    if [ ${PIPESTATUS[0]} -ne 0 ]; then
        print_error "Failed to clone $lib_name"
        return 1
    fi
    
    # Remove .git directory to prevent submodule issues
    if [ -d "$lib_name/.git" ]; then
        rm -rf "$lib_name/.git"
        print_success "Successfully cloned $lib_name and removed .git directory"
    else
        print_error "Failed to find .git directory in $lib_name"
        return 1
    fi
}

# Main function
main() {
    print_status "Starting library cloning process..."
    print_status "Log file: $LOG_FILE"
    print_status "Target directory: $LIBS_DIR"
    
    # Array of libraries to clone
    # Format: "repo_url|library_name|branch"
    # If no branch is specified, use empty string
    libraries=(
        "https://github.com/emweb/wt.git|wt-4.11-release|4.11-release"
        "https://github.com/ggml-org/whisper.cpp.git|whisper.cpp|"
        "https://github.com/leethomason/tinyxml2.git|tinyxml2|"
        "https://github.com/nlohmann/json.git|nlohmann-json|"
    )
    
    # Clone each library
    for library in "${libraries[@]}"; do
        IFS='|' read -r repo_url lib_name branch <<< "$library"
        clone_library "$repo_url" "$lib_name" "$branch"
        echo  # Add empty line for better readability
    done
    
    print_success "Library cloning process completed!"
    print_status "Libraries installed in: $LIBS_DIR"
    print_status "Log saved to: $LOG_FILE"
    
    # List the contents of libs directory
    if [ -d "$LIBS_DIR" ]; then
        print_status "Available libraries:"
        ls -la "$LIBS_DIR" | grep "^d" | awk '{print "  - " $9}' | grep -v "^\s*-\s*\.$" | grep -v "^\s*-\s*\.\.$"
    fi
}

# Check if git is available
if ! command -v git &> /dev/null; then
    print_error "git is not installed or not in PATH"
    exit 1
fi

# Run main function
main "$@"
