#!/bin/bash
# Script to build the wt_cv Docker image
# Usage: ./scripts/docker-b-wt-cv-stylus.sh [options]

set -e  # Exit on any error

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="$SCRIPT_DIR/output"
LOG_FILE="$OUTPUT_DIR/docker-b-wt-cv-stylus.log"

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
    echo "  Builds the wt_cv Docker image for the Wt CV Stylus project"
    echo "  This is a production image that uses wt_builder as base for multi-stage build"
    echo "  Automatically builds wt_builder dependency if it doesn't exist"
    echo "  Automatically removes existing image before building new one"
    echo ""
    echo -e "${BOLD}${YELLOW}Options:${NC}"
    echo -e "  ${CYAN}-h, --help${NC}      Show this help message"
    echo -e "  ${CYAN}-nc, --no-cache${NC} Build without using Docker cache"
    echo ""
    echo -e "${BOLD}${YELLOW}Examples:${NC}"
    echo -e "  ${GREEN}$0${NC}              # Build the wt_cv image"
    echo -e "  ${GREEN}$0 -nc${NC}          # Build without cache"
}

# Variables
NO_CACHE=""
IMAGE_NAME="wt_cv"
DOCKERFILE_PATH="dockerfiles/wt_cv"
DEPENDENCY_IMAGE="wt_builder"

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
    
    if docker info &> /dev/null; then
        print_status "Docker is running and accessible"
        return 0
    fi
    
    print_status "Docker requires sudo privileges. Checking with sudo..."
    if ! sudo docker info &> /dev/null; then
        print_error "Docker is not running. Please start Docker daemon: sudo systemctl start docker"
        exit 1
    fi
    
    print_warning "Docker requires sudo privileges for this user"
    print_status "You may be prompted for your password during the build"
}

# Check if we need sudo for Docker commands
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

# Check if Dockerfile exists
check_dockerfile() {
    if [ ! -f "$PROJECT_ROOT/$DOCKERFILE_PATH" ]; then
        print_error "Dockerfile not found at: $PROJECT_ROOT/$DOCKERFILE_PATH"
        exit 1
    fi
}

# Check if image already exists
image_exists() {
    local img_name="$1"
    if need_sudo_for_docker; then
        sudo docker images --format "table {{.Repository}}" | grep -q "^$img_name$"
    else
        docker images --format "table {{.Repository}}" | grep -q "^$img_name$"
    fi
}

# Check if dependency image exists, build if needed
check_dependency() {
    if ! image_exists "$DEPENDENCY_IMAGE"; then
        print_warning "Dependency image '$DEPENDENCY_IMAGE' not found. Building it first..."
        if "$SCRIPT_DIR/docker-b-wt-builder.sh"; then
            print_success "Dependency image '$DEPENDENCY_IMAGE' built successfully"
        else
            print_error "Failed to build dependency image '$DEPENDENCY_IMAGE'"
            exit 1
        fi
    else
        print_status "Dependency image '$DEPENDENCY_IMAGE' found"
    fi
}

# Set permissions for containers directory
set_permissions() {
    local containers_dir="$PROJECT_ROOT/containers"
    if [ -d "$containers_dir" ]; then
        print_status "Setting permissions for containers directory..."
        if chmod -R 777 "$containers_dir" 2>&1 | tee -a "$LOG_FILE"; then
            print_success "Permissions set for containers directory"
        else
            print_warning "Failed to set permissions for containers directory, continuing anyway..."
        fi
    else
        print_status "Containers directory not found, skipping permission setup"
    fi
}

# Remove existing image before building
remove_existing_image() {
    if image_exists "$IMAGE_NAME"; then
        print_status "Removing existing $IMAGE_NAME image..."
        if need_sudo_for_docker; then
            if sudo docker rmi "$IMAGE_NAME" 2>&1 | tee -a "$LOG_FILE"; then
                print_success "Existing image removed"
            else
                print_warning "Failed to remove existing image, continuing anyway..."
            fi
        else
            if docker rmi "$IMAGE_NAME" 2>&1 | tee -a "$LOG_FILE"; then
                print_success "Existing image removed"
            else
                print_warning "Failed to remove existing image, continuing anyway..."
            fi
        fi
    else
        print_status "No existing $IMAGE_NAME image found"
    fi
    return 0
}

# Build the Docker image
build_image() {
    print_status "Building Docker image: $IMAGE_NAME"
    print_status "Using Dockerfile: $DOCKERFILE_PATH"
    
    # Change to project root for build context
    cd "$PROJECT_ROOT"
    
    # Build command
    if need_sudo_for_docker; then
        local build_cmd="sudo docker build $NO_CACHE -t $IMAGE_NAME:latest -f $DOCKERFILE_PATH ."
        print_status "Running: $build_cmd"
        
        # Execute build with output to both console and log
        if eval "$build_cmd" 2>&1 | tee -a "$LOG_FILE"; then
            print_success "Docker image $IMAGE_NAME built successfully!"
            
            # Show image info
            print_status "Image details:"
            sudo docker images "$IMAGE_NAME" --format "table {{.Repository}}\t{{.Tag}}\t{{.Size}}\t{{.CreatedAt}}" | tee -a "$LOG_FILE"
        else
            print_error "Failed to build Docker image $IMAGE_NAME"
            exit 1
        fi
    else
        local build_cmd="docker build $NO_CACHE -t $IMAGE_NAME:latest -f $DOCKERFILE_PATH ."
        print_status "Running: $build_cmd"
        
        # Execute build with output to both console and log
        if eval "$build_cmd" 2>&1 | tee -a "$LOG_FILE"; then
            print_success "Docker image $IMAGE_NAME built successfully!"
            
            # Show image info
            print_status "Image details:"
            docker images "$IMAGE_NAME" --format "table {{.Repository}}\t{{.Tag}}\t{{.Size}}\t{{.CreatedAt}}" | tee -a "$LOG_FILE"
        else
            print_error "Failed to build Docker image $IMAGE_NAME"
            exit 1
        fi
    fi
}

# Main script logic
print_status "Starting Docker image build for $IMAGE_NAME..."

# Perform checks
check_docker
check_dockerfile

# Check and build dependency if needed
check_dependency

# Set container permissions
set_permissions

# Handle existing image
remove_existing_image

# Build the image
build_image

print_success "Docker image build completed successfully!"
print_status "You can now run the image with:"
print_status "  docker run -p 9020:9020 $IMAGE_NAME:latest"
print_status "Or use the run script: ./scripts/docker-r-wt-cv-stylus.sh"
print_status "The application will be available at http://localhost:9020"
print_status "Build log saved to: $LOG_FILE"
