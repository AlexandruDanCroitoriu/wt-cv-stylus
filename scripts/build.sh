#!/bin/bash

# Unified script to build the application
# Usage: ./scripts/build.sh [--debug|-d|--release|-r] [clean]

set -e  # Exit on any error

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="$SCRIPT_DIR/output"

# Default configuration
BUILD_TYPE="debug"
CLEAN_BUILD=false

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Set log file and clear it for this run
LOG_FILE="$OUTPUT_DIR/build.log"
> "$LOG_FILE"

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
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

# Function to show usage
show_usage() {
    echo -e "${BLUE}Usage:${NC} $0 [--debug|-d|--release|-r] [clean]"
    echo ""
    echo -e "${GREEN}Build the Wt application with CMake${NC}"
    echo ""
    echo -e "${YELLOW}Arguments:${NC}"
    echo -e "  ${CYAN}--debug, -d${NC}     Build debug version (default)"
    echo -e "  ${CYAN}--release, -r${NC}   Build release version"
    echo -e "  ${CYAN}clean${NC}           Clean build from scratch"
    echo ""
    echo -e "${YELLOW}Examples:${NC}"
    echo -e "  ${GREEN}$0${NC}                    # Build debug version"
    echo -e "  ${GREEN}$0 --release${NC}          # Build release version"
    echo -e "  ${GREEN}$0 --debug clean${NC}      # Clean debug build"
    echo -e "  ${GREEN}$0 -r clean${NC}           # Clean release build"
}

# Function to get number of CPU cores
get_cpu_cores() {
    if command -v nproc >/dev/null 2>&1; then
        nproc
    elif [ -r /proc/cpuinfo ]; then
        grep -c ^processor /proc/cpuinfo
    else
        echo "4"  # fallback
    fi
}

# Function to build the application
build_application() {
    local build_type="$1"
    local clean_build="$2"
    
    BUILD_DIR="$PROJECT_ROOT/build/$build_type"
    
    print_status "Starting $build_type build process..."
    print_status "Build directory: $BUILD_DIR"
    print_status "Log file: $LOG_FILE"
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Clean build if requested
    if [ "$clean_build" = true ]; then
        print_status "Performing clean build..."
        if [ -f "Makefile" ]; then
            make clean 2>&1 | tee -a "$LOG_FILE" || true
        fi
        # Remove CMake cache files
        rm -f CMakeCache.txt
        rm -rf CMakeFiles/
    fi
    
    # Set CMAKE_BUILD_TYPE
    local cmake_build_type
    if [ "$build_type" = "debug" ]; then
        cmake_build_type="Debug"
    else
        cmake_build_type="Release"
    fi
    
    # Configure with CMake
    print_status "Configuring CMake for $build_type build..."
    if cmake -DCMAKE_BUILD_TYPE="$cmake_build_type" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON "$PROJECT_ROOT" 2>&1 | tee -a "$LOG_FILE"; then
        print_success "CMake configuration completed successfully"
    else
        print_error "CMake configuration failed! Check log file: $LOG_FILE"
        return 1
    fi
    
    # Build the application
    local cpu_cores
    cpu_cores=$(get_cpu_cores)
    print_status "Building with $cpu_cores parallel jobs..."
    
    if make -j"$cpu_cores" 2>&1 | tee -a "$LOG_FILE"; then
        print_success "$build_type build completed successfully!"
        print_status "Executable location: $BUILD_DIR/app"
    else
        print_error "Build failed! Check log file: $LOG_FILE"
        return 1
    fi
    
    # Check if executable exists and is executable
    if [ ! -f "$BUILD_DIR/app" ]; then
        print_error "Application binary not found at: $BUILD_DIR/app"
        return 1
    fi
    
    if [ ! -x "$BUILD_DIR/app" ]; then
        print_error "Application binary is not executable: $BUILD_DIR/app"
        return 1
    fi
    
    print_success "Application binary created successfully"
    print_status "Binary size: $(du -h "$BUILD_DIR/app" | cut -f1)"
    
    return 0
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug|-d)
            BUILD_TYPE="debug"
            shift
            ;;
        --release|-r)
            BUILD_TYPE="release"
            shift
            ;;
        clean)
            CLEAN_BUILD=true
            shift
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown argument: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Check dependencies
if ! command -v cmake &> /dev/null; then
    print_error "cmake is not installed or not in PATH"
    exit 1
fi

if ! command -v make &> /dev/null; then
    print_error "make is not installed or not in PATH"
    exit 1
fi

# Start the build process
print_status "Starting build process..."
print_status "Build type: $BUILD_TYPE"
print_status "Clean build: $CLEAN_BUILD"

# Build the application
if build_application "$BUILD_TYPE" "$CLEAN_BUILD"; then
    print_success "Build completed successfully!"
    print_status "You can now run the application using:"
    print_status "  ./scripts/run.sh --$BUILD_TYPE"
    exit 0
else
    print_error "Build failed!"
    exit 1
fi
