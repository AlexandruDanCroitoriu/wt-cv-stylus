#!/bin/bash

# Unified script to run the application
# Usage: ./scripts/run.sh [--debug|-d|--release|-r]

set -e  # Exit on any error

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="$SCRIPT_DIR/output"

# Default configuration
BUILD_TYPE="debug"

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Set log file and clear it for this run
LOG_FILE="$OUTPUT_DIR/run.log"
> "$LOG_FILE"

# Function to cleanup on exit
cleanup() {
    local exit_code=$?
    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}[SUCCESS]${NC} Application stopped gracefully"
    else
        echo -e "${YELLOW}[WARNING]${NC} Application stopped with exit code: $exit_code"
    fi
}

# Set trap for cleanup
trap cleanup EXIT

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
    echo -e "${BLUE}Usage:${NC} $0 [--debug|-d|--release|-r]"
    echo ""
    echo -e "${GREEN}Run the Wt application with default parameters.${NC}"
    echo -e "${GREEN}All configuration is handled by the CMake build system.${NC}"
    echo ""
    echo -e "${YELLOW}Arguments:${NC}"
    echo -e "  ${CYAN}--debug, -d${NC}     Run debug version (default)"
    echo -e "  ${CYAN}--release, -r${NC}   Run release version"
    echo ""
    echo -e "${YELLOW}Application Settings (from CMakeLists.txt):${NC}"
    echo -e "  ${CYAN}--docroot${NC} ../../"
    echo -e "  ${CYAN}-c${NC} ../../wt_config.xml"
    echo -e "  ${CYAN}--http-address${NC} 0.0.0.0"
    echo -e "  ${CYAN}--http-port${NC} 9020"
    echo ""
    echo -e "${YELLOW}Examples:${NC}"
    echo -e "  ${GREEN}$0${NC}              # Run debug version"
    echo -e "  ${GREEN}$0 --debug${NC}      # Run debug version"
    echo -e "  ${GREEN}$0 --release${NC}    # Run release version"
    echo -e "  ${GREEN}$0 -r${NC}           # Run release version"
    echo ""
    echo -e "${YELLOW}Note:${NC} Existing application instances on port 9020 will be automatically terminated."
}

# Function to check if port is available and handle conflicts
check_and_handle_port() {
    local port="9020"
    
    # Check if port is in use
    if command -v ss &> /dev/null; then
        port_info=$(ss -tuln | grep ":$port ")
    elif command -v netstat &> /dev/null; then
        port_info=$(netstat -tuln | grep ":$port ")
    else
        print_warning "Cannot check port availability (ss and netstat not found)"
        return 0
    fi
    
    if [ -n "$port_info" ]; then
        print_warning "Port $port is already in use"
        
        # Try to find the process using the port
        if command -v lsof &> /dev/null; then
            process_info=$(lsof -ti:$port 2>/dev/null)
            if [ -n "$process_info" ]; then
                # Get process details
                pid=$(echo "$process_info" | head -n1)
                if [ -n "$pid" ]; then
                    process_name=$(ps -p "$pid" -o comm= 2>/dev/null || echo "unknown")
                    process_cmd=$(ps -p "$pid" -o cmd= 2>/dev/null || echo "unknown")
                    
                    print_status "Found process using port $port:"
                    print_status "  PID: $pid"
                    print_status "  Name: $process_name"
                    print_status "  Command: $process_cmd"
                    
                    # Check if it's our application
                    if [[ "$process_cmd" == *"app"* ]] && [[ "$process_cmd" == *"9020"* ]]; then
                        print_warning "Detected existing instance of our application"
                        print_status "Automatically killing existing application instance (PID: $pid)..."
                        
                        if kill "$pid" 2>/dev/null; then
                            print_success "Successfully killed process $pid"
                            # Wait a moment for the port to be released
                            sleep 2
                            # Verify port is now free
                            if command -v ss &> /dev/null; then
                                remaining=$(ss -tuln | grep ":$port " || true)
                            else
                                remaining=$(netstat -tuln | grep ":$port " || true)
                            fi
                            
                            if [ -z "$remaining" ]; then
                                print_success "Port $port is now available"
                            else
                                print_warning "Port $port may still be in use"
                            fi
                        else
                            print_error "Failed to kill process $pid"
                            print_status "You may need to manually stop the existing application"
                            return 1
                        fi
                    else
                        print_warning "Port $port is used by another application: $process_name"
                        print_status "Please stop the other application or use a different port"
                        return 1
                    fi
                fi
            fi
        else
            print_warning "Cannot identify process using port $port (lsof not found)"
            print_status "Please manually check what's using port $port"
            return 1
        fi
    else
        print_status "Port $port is available"
    fi
    
    return 0
}

# Function to check if application is built
check_build() {
    local build_type="$1"
    local build_dir="$PROJECT_ROOT/build/$build_type"
    
    if [ ! -f "$build_dir/Makefile" ]; then
        print_warning "Build directory not found or not configured at: $build_dir"
        return 1
    fi
    
    if [ ! -f "$build_dir/app" ]; then
        print_warning "Application binary not found at: $build_dir/app"
        return 1
    fi
    
    return 0
}

# Function to build if needed
ensure_build() {
    local build_type="$1"
    
    if ! check_build "$build_type"; then
        print_status "Application not built for $build_type mode."
        print_status "Building application using build script..."
        echo
        
        # Call the unified build script
        if "$SCRIPT_DIR/build.sh" "--$build_type"; then
            print_success "Build completed successfully!"
        else
            print_error "Build failed! Cannot run application."
            exit 1
        fi
        echo
    else
        print_status "Application already built for $build_type mode."
    fi
}

# Function to run the application
run_application() {
    local build_type="$1"
    local build_dir="$PROJECT_ROOT/build/$build_type"
    
    print_status "Starting $build_type application run..."
    print_status "Log file: $LOG_FILE"
    print_status "Running application in ${build_type^^} mode..."
    print_status "Build directory: $build_dir"
    
    # Check port availability and handle conflicts
    if ! check_and_handle_port; then
        print_error "Cannot start application due to port conflict"
        exit 1
    fi
    
    print_success "Application starting..."
    print_status "Access the application at: http://localhost:9020"
    print_status "Press Ctrl+C to stop the application"
    print_status "Application output will be logged to: $LOG_FILE"
    echo
    
    # Change to build directory and run with make
    cd "$build_dir"
    make run 2>&1 | tee -a "$LOG_FILE"
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

# Start the process
print_status "Starting run process..."
print_status "Build type: $BUILD_TYPE"
echo

# Ensure the application is built
ensure_build "$BUILD_TYPE"

# Run the application
run_application "$BUILD_TYPE"
