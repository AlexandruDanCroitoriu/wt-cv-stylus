#!/bin/bash
# Script to run the wt_cv Docker image
# Usage: ./scripts/docker-r-wt-cv-stylus.sh [options]

set -e  # Exit on any error

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="$SCRIPT_DIR/output"
LOG_FILE="$OUTPUT_DIR/docker-r-wt-cv-stylus.log"

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
    echo "  Runs the wt_cv Docker image for the Wt CV Stylus project"
    echo "  Automatically builds the image if it doesn't exist"
    echo "  Kills any existing containers on the same port"
    echo "  Maps port 9020 from container to host"
    echo ""
    echo -e "${BOLD}${YELLOW}Options:${NC}"
    echo -e "  ${CYAN}-h, --help${NC}      Show this help message"
    echo -e "  ${CYAN}-d, --detached${NC}  Run container in detached mode (background)"
    echo -e "  ${CYAN}-p, --port PORT${NC} Use custom port (default: 9020)"
    echo -e "  ${CYAN}--rebuild${NC}       Force rebuild of image before running"
    echo ""
    echo -e "${BOLD}${YELLOW}Examples:${NC}"
    echo -e "  ${GREEN}$0${NC}              # Run the container interactively"
    echo -e "  ${GREEN}$0 -d${NC}           # Run in background (detached)"
    echo -e "  ${GREEN}$0 -p 8080${NC}      # Run on port 8080"
    echo -e "  ${GREEN}$0 --rebuild${NC}    # Rebuild image before running"
}

# Variables
DETACHED_MODE=false
PORT="9020"
REBUILD=false
IMAGE_NAME="wt_cv"
CONTAINER_NAME="wt-cv-stylus"

# Argument parsing (REQUIRED)
while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            show_usage
            exit 0
            ;;
        --detached|-d)
            DETACHED_MODE=true
            shift
            ;;
        --port|-p)
            PORT="$2"
            shift 2
            ;;
        --rebuild)
            REBUILD=true
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
    print_status "You may be prompted for your password during operations"
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

# Check if image exists
image_exists() {
    local img_name="$1"
    if need_sudo_for_docker; then
        sudo docker images --format "table {{.Repository}}" | grep -q "^$img_name$"
    else
        docker images --format "table {{.Repository}}" | grep -q "^$img_name$"
    fi
}

# Build image if needed
ensure_image_exists() {
    if [ "$REBUILD" = true ] || ! image_exists "$IMAGE_NAME"; then
        if [ "$REBUILD" = true ]; then
            print_status "Rebuilding $IMAGE_NAME image as requested..."
        else
            print_warning "Image '$IMAGE_NAME' not found. Building it first..."
        fi
        
        if "$SCRIPT_DIR/docker-b-wt-cv-stylus.sh"; then
            print_success "Image '$IMAGE_NAME' built successfully"
        else
            print_error "Failed to build image '$IMAGE_NAME'"
            exit 1
        fi
    else
        print_status "Image '$IMAGE_NAME' found"
    fi
}

# Kill existing containers on the same port
kill_existing_containers() {
    print_status "Checking for existing containers on port $PORT..."
    
    # Find processes using the port
    if lsof -ti:$PORT >/dev/null 2>&1; then
        print_warning "Port $PORT is in use. Killing existing processes..."
        lsof -ti:$PORT | xargs kill -9 2>/dev/null || true
        sleep 2
    fi
    
    # Stop and remove existing container with same name
    if need_sudo_for_docker; then
        if sudo docker ps -a --format "{{.Names}}" | grep -q "^$CONTAINER_NAME$"; then
            print_status "Stopping and removing existing container: $CONTAINER_NAME"
            sudo docker stop "$CONTAINER_NAME" >/dev/null 2>&1 || true
            sudo docker rm "$CONTAINER_NAME" >/dev/null 2>&1 || true
        fi
    else
        if docker ps -a --format "{{.Names}}" | grep -q "^$CONTAINER_NAME$"; then
            print_status "Stopping and removing existing container: $CONTAINER_NAME"
            docker stop "$CONTAINER_NAME" >/dev/null 2>&1 || true
            docker rm "$CONTAINER_NAME" >/dev/null 2>&1 || true
        fi
    fi
}

# Run the Docker container
run_container() {
    print_status "Running Docker container: $CONTAINER_NAME"
    print_status "Port mapping: $PORT:9020"
    print_status "Image: $IMAGE_NAME:latest"
    
        # Prepare run command
    local run_args="--name $CONTAINER_NAME -p $PORT:9020 -v ./static/:/apps/cv/static/"
    
    if [ "$DETACHED_MODE" = true ]; then
        run_args="$run_args -d"
        print_status "Running in detached mode (background)"
    else
        run_args="$run_args -it --rm"
        print_status "Running in interactive mode (foreground)"
    fi
    
    # Execute run command
    if need_sudo_for_docker; then
        local run_cmd="sudo docker run $run_args $IMAGE_NAME:latest"
        print_status "Running: $run_cmd"
        
        if [ "$DETACHED_MODE" = true ]; then
            if eval "$run_cmd" 2>&1 | tee -a "$LOG_FILE"; then
                print_success "Container started successfully in background"
                print_status "Container ID: $(sudo docker ps --filter "name=$CONTAINER_NAME" --format "{{.ID}}")"
                print_status "View logs with: sudo docker logs -f $CONTAINER_NAME"
                print_status "Stop with: sudo docker stop $CONTAINER_NAME"
            else
                print_error "Failed to start container"
                exit 1
            fi
        else
            print_status "Starting interactive container. Press Ctrl+C to stop."
            eval "$run_cmd"
        fi
    else
        local run_cmd="docker run $run_args $IMAGE_NAME:latest"
        print_status "Running: $run_cmd"
        
        if [ "$DETACHED_MODE" = true ]; then
            if eval "$run_cmd" 2>&1 | tee -a "$LOG_FILE"; then
                print_success "Container started successfully in background"
                print_status "Container ID: $(docker ps --filter "name=$CONTAINER_NAME" --format "{{.ID}}")"
                print_status "View logs with: docker logs -f $CONTAINER_NAME"
                print_status "Stop with: docker stop $CONTAINER_NAME"
            else
                print_error "Failed to start container"
                exit 1
            fi
        else
            print_status "Starting interactive container. Press Ctrl+C to stop."
            eval "$run_cmd"
        fi
    fi
}

# Cleanup function for graceful shutdown
cleanup() {
    local exit_code=$?
    if [ $exit_code -eq 0 ]; then
        print_success "Container stopped gracefully"
    else
        print_warning "Container stopped with exit code: $exit_code"
    fi
}

# Set trap for cleanup
trap cleanup EXIT

# Main script logic
print_status "Starting Docker container for $IMAGE_NAME..."

# Perform checks
check_docker

# Ensure image exists
ensure_image_exists

# Kill existing containers
kill_existing_containers

# Run the container
run_container

print_success "Docker container operation completed!"
print_status "The application should be available at http://localhost:$PORT"
print_status "Run log saved to: $LOG_FILE"
