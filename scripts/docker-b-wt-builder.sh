#!/bin/bash
# Script to build the wt_builder_scripted Docker image
# Usage: ./scripts/docker-b-wt-builder-scripted.sh [options]

set -e  # Exit on any error

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="$SCRIPT_DIR/output"
LOG_FILE="$OUTPUT_DIR/docker-b-wt-builder-scripted.log"

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
    echo "  Builds the wt_builder_scripted Docker image for the Wt CV Stylus project"
    echo "  This image uses the ubuntu-install-dependencies.sh script for dependency installation"
    echo "  Automatically removes existing image before building new one"
    echo ""
    echo -e "${BOLD}${YELLOW}Options:${NC}"
    echo -e "  ${CYAN}-h, --help${NC}      Show this help message"
    echo -e "  ${CYAN}-nc, --no-cache${NC} Build without using Docker cache"
    echo ""
    echo -e "${BOLD}${YELLOW}Examples:${NC}"
    echo -e "  ${GREEN}$0${NC}              # Build the wt_builder_scripted image"
    echo -e "  ${GREEN}$0 -nc${NC}          # Build without cache"
}

# Variables
NO_CACHE=""
IMAGE_NAME="wt_builder_scripted"
DOCKERFILE_PATH="dockerfiles/wt_builder_scripted"

# Argument parsing (REQUIRED)
while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            show_usage
            exit 0
            ;;
        --no-cache|-nc)
            NO_CACHE="--no-cache"
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Check if Docker is installed and running
check_docker() {
    if ! command -v docker &> /dev/null; then
        print_error "Docker is not installed. Please install Docker first."
        exit 1
    fi
    
    # Check if Docker is accessible without sudo first
    if docker info &> /dev/null; then
        print_status "Docker is running and accessible"
        return 0
    fi
    
    # If not accessible without sudo, try with sudo
    print_status "Docker requires sudo privileges. Checking with sudo..."
    if ! sudo docker info &> /dev/null; then
        print_error "Docker is not running or not accessible even with sudo."
        print_error "Please start Docker daemon: sudo systemctl start docker"
        exit 1
    fi
    
    print_warning "Docker requires sudo privileges for this user"
    print_status "You may be prompted for your password during the build"
}

# Check if Dockerfile exists
check_dockerfile() {
    if [ ! -f "$PROJECT_ROOT/$DOCKERFILE_PATH" ]; then
        print_error "Dockerfile not found at: $PROJECT_ROOT/$DOCKERFILE_PATH"
        exit 1
    fi
}

# Check if dependency script exists
check_dependency_script() {
    if [ ! -f "$PROJECT_ROOT/scripts/ubuntu-install-dependencies.sh" ]; then
        print_error "Dependency script not found at: $PROJECT_ROOT/scripts/ubuntu-install-dependencies.sh"
        exit 1
    fi
    
    if [ ! -x "$PROJECT_ROOT/scripts/ubuntu-install-dependencies.sh" ]; then
        print_error "Dependency script is not executable: $PROJECT_ROOT/scripts/ubuntu-install-dependencies.sh"
        print_status "Run: chmod +x scripts/ubuntu-install-dependencies.sh"
        exit 1
    fi
}

# Check if image already exists
image_exists() {
    # Try without sudo first, then with sudo if needed
    if docker images --format "table {{.Repository}}" 2>/dev/null | grep -q "^$IMAGE_NAME$"; then
        return 0
    elif sudo docker images --format "table {{.Repository}}" 2>/dev/null | grep -q "^$IMAGE_NAME$"; then
        return 0
    else
        return 1
    fi
}

# Remove existing image before building
remove_existing_image() {
    if image_exists; then
        print_status "Removing existing $IMAGE_NAME image..."
        
        # Try without sudo first, then with sudo if needed
        if docker rmi "$IMAGE_NAME" 2>&1 | tee -a "$LOG_FILE" 2>/dev/null; then
            print_success "Existing image removed"
        elif sudo docker rmi "$IMAGE_NAME" 2>&1 | tee -a "$LOG_FILE"; then
            print_success "Existing image removed (with sudo)"
        else
            print_warning "Failed to remove existing image, continuing anyway..."
        fi
    else
        print_status "No existing $IMAGE_NAME image found"
    fi
    return 0
}

# Determine if we need sudo for docker commands
need_sudo_for_docker() {
    if docker info &> /dev/null; then
        return 1  # No sudo needed
    else
        return 0  # Sudo needed
    fi
}

# Build the Docker image
build_image() {
    print_status "Building Docker image: $IMAGE_NAME"
    print_status "Using Dockerfile: $DOCKERFILE_PATH"
    print_status "Using dependency script: scripts/ubuntu-install-dependencies.sh"
    
    # Change to project root for build context
    cd "$PROJECT_ROOT"
    
    # Determine if we need sudo for Docker commands
    local docker_cmd="docker"
    if need_sudo_for_docker; then
        docker_cmd="sudo docker"
        print_status "Using sudo for Docker commands"
    fi
    
    # Build command
    local build_cmd="$docker_cmd build $NO_CACHE -t $IMAGE_NAME -f $DOCKERFILE_PATH ."
    print_status "Running: $build_cmd"
    
    # Execute build with output to both console and log
    if eval "$build_cmd" 2>&1 | tee -a "$LOG_FILE"; then
        print_success "Docker image $IMAGE_NAME built successfully!"
        
        # Show image info
        print_status "Image details:"
        $docker_cmd images "$IMAGE_NAME" --format "table {{.Repository}}\t{{.Tag}}\t{{.Size}}\t{{.CreatedAt}}" | tee -a "$LOG_FILE"
    else
        print_error "Failed to build Docker image $IMAGE_NAME"
        exit 1
    fi
}

# Main script logic
print_status "Starting Docker image build for $IMAGE_NAME..."

# Perform checks
check_docker
check_dockerfile
check_dependency_script

# Handle existing image
remove_existing_image

# Build the image
build_image

print_success "Docker image build completed successfully!"
print_status "You can now use the image with: docker run -it $IMAGE_NAME"
print_status "This image was built using the ubuntu-install-dependencies.sh script"
print_status "Build log saved to: $LOG_FILE"
