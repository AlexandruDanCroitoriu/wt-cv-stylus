#!/bin/bash
# Script to install all system dependencies for the Wt CV Stylus project
# Usage: ./scripts/install-dependencies.sh [options]

set -e  # Exit on any error

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="$SCRIPT_DIR/output"
LOG_FILE="$OUTPUT_DIR/install-dependencies.log"

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
    echo "  Installs all system dependencies required for building the Wt CV Stylus project"
    echo "  This includes all packages from the wt_builder Docker image plus additional requirements"
    echo "  Requires sudo privileges for package installation"
    echo ""
    echo -e "${BOLD}${YELLOW}Options:${NC}"
    echo -e "  ${CYAN}-h, --help${NC}      Show this help message"
    echo -e "  ${CYAN}-y, --yes${NC}       Automatically answer yes to all prompts"
    echo -e "  ${CYAN}--skip-update${NC}   Skip apt update step"
    echo ""
    echo -e "${BOLD}${YELLOW}Examples:${NC}"
    echo -e "  ${GREEN}$0${NC}              # Install all dependencies interactively"
    echo -e "  ${GREEN}$0 -y${NC}           # Install all dependencies automatically"
    echo -e "  ${GREEN}$0 --skip-update${NC} # Install without updating package lists"
}

# Variables
AUTO_YES=""
SKIP_UPDATE=false

# Argument parsing (REQUIRED)
while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            show_usage
            exit 0
            ;;
        --yes|-y)
            AUTO_YES="-y"
            shift
            ;;
        --skip-update)
            SKIP_UPDATE=true
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Check if running as root (not recommended)
check_sudo() {
    if [ "$EUID" -eq 0 ]; then
        print_warning "Running as root is not recommended. Consider running as a regular user with sudo."
    fi
    
    # Check if sudo is available
    if ! command -v sudo &> /dev/null; then
        print_error "sudo is not installed. Please install sudo or run as root."
        exit 1
    fi
}

# Detect OS and distribution
detect_os() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS=$NAME
        VERSION=$VERSION_ID
        print_status "Detected OS: $OS $VERSION"
        
        # Check if it's Ubuntu/Debian based
        if [[ "$ID" != "ubuntu" && "$ID_LIKE" != *"debian"* && "$ID" != "debian" ]]; then
            print_warning "This script is designed for Ubuntu/Debian systems. Your system: $ID"
            print_warning "You may need to manually install equivalent packages."
        fi
    else
        print_error "Cannot detect operating system. /etc/os-release not found."
        exit 1
    fi
}

# Update package lists
update_packages() {
    if [ "$SKIP_UPDATE" = false ]; then
        print_status "Updating package lists..."
        if sudo apt update $AUTO_YES 2>&1 | tee -a "$LOG_FILE"; then
            print_success "Package lists updated"
        else
            print_error "Failed to update package lists"
            exit 1
        fi
    else
        print_status "Skipping package list update"
    fi
}

# Install basic development tools
install_basic_tools() {
    print_status "Installing basic development tools..."
    local packages=(
        "git"
        "cmake" 
        "build-essential"
        "curl"
        "wget"
        "software-properties-common"
        "apt-transport-https"
        "ca-certificates"
        "gnupg"
        "lsb-release"
    )
    
    for package in "${packages[@]}"; do
        print_status "Installing $package..."
        if sudo apt install $AUTO_YES "$package" 2>&1 | tee -a "$LOG_FILE"; then
            print_success "$package installed"
        else
            print_error "Failed to install $package"
            exit 1
        fi
    done
}

# Install C++ libraries and development headers
install_cpp_libraries() {
    print_status "Installing C++ libraries and development headers..."
    local packages=(
        "libboost-all-dev"
        "libssl-dev"
        "libcrypto++-dev"
        "libpq-dev"
        "libpsl-dev"
        "libunistring-dev"  # This was the missing library causing build failures
        "zlib1g-dev"
        "libsqlite3-dev"
        "libfcgi-dev"
        "libgraphicsmagick++1-dev"
        "libharu-dev"
        "libpango1.0-dev"
        "libgl1-mesa-dev"
        "libglu1-mesa-dev"
    )
    
    for package in "${packages[@]}"; do
        print_status "Installing $package..."
        if sudo apt install $AUTO_YES "$package" 2>&1 | tee -a "$LOG_FILE"; then
            print_success "$package installed"
        else
            print_warning "Failed to install $package (might not be available on this system)"
        fi
    done
}

# Install Python development tools
install_python_tools() {
    print_status "Installing Python development tools..."
    local packages=(
        "python3"
        "python3-pip"
        "python3-venv"
        "python3-dev"
    )
    
    for package in "${packages[@]}"; do
        print_status "Installing $package..."
        if sudo apt install $AUTO_YES "$package" 2>&1 | tee -a "$LOG_FILE"; then
            print_success "$package installed"
        else
            print_error "Failed to install $package"
            exit 1
        fi
    done
    
    # Install Python build tools
    print_status "Installing Python build tools (meson, ninja)..."
    if python3 -m pip install --user --upgrade pip meson ninja 2>&1 | tee -a "$LOG_FILE"; then
        print_success "Python build tools installed"
    else
        print_error "Failed to install Python build tools"
        exit 1
    fi
}

# Install Node.js and npm
install_nodejs() {
    print_status "Installing Node.js and npm..."
    
    # Add NodeSource repository
    print_status "Adding NodeSource repository..."
    if curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash - 2>&1 | tee -a "$LOG_FILE"; then
        print_success "NodeSource repository added"
    else
        print_error "Failed to add NodeSource repository"
        exit 1
    fi
    
    # Install Node.js and npm
    local packages=("nodejs" "npm")
    for package in "${packages[@]}"; do
        print_status "Installing $package..."
        if sudo apt install $AUTO_YES "$package" 2>&1 | tee -a "$LOG_FILE"; then
            print_success "$package installed"
        else
            print_error "Failed to install $package"
            exit 1
        fi
    done
    
    # Verify installation
    print_status "Node.js version: $(node --version)"
    print_status "npm version: $(npm --version)"
}

# Install Docker (optional)
install_docker() {
    print_status "Checking if Docker should be installed..."
    
    if command -v docker &> /dev/null; then
        print_status "Docker is already installed: $(docker --version)"
        return 0
    fi
    
    if [ "$AUTO_YES" = "-y" ]; then
        install_docker_now=true
    else
        echo -e "${YELLOW}Do you want to install Docker? (y/N):${NC}"
        read -r response
        case "$response" in
            [yY][eE][sS]|[yY]) 
                install_docker_now=true
                ;;
            *)
                install_docker_now=false
                ;;
        esac
    fi
    
    if [ "$install_docker_now" = true ]; then
        print_status "Installing Docker..."
        
        # Add Docker's official GPG key
        sudo mkdir -p /etc/apt/keyrings
        curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
        
        # Add the repository
        echo \
          "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
          $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
        
        # Update and install
        sudo apt update
        sudo apt install $AUTO_YES docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
        
        # Add user to docker group
        sudo usermod -aG docker $USER
        
        print_success "Docker installed successfully!"
        print_warning "You may need to log out and back in for Docker group membership to take effect."
    else
        print_status "Skipping Docker installation"
    fi
}

# Verify installations
verify_installations() {
    print_status "Verifying installations..."
    
    local tools=(
        "git --version"
        "cmake --version"
        "gcc --version"
        "g++ --version"
        "node --version"
        "npm --version"
        "python3 --version"
        "pip3 --version"
    )
    
    for tool in "${tools[@]}"; do
        if $tool &> /dev/null; then
            print_success "✓ $tool"
        else
            print_warning "✗ $tool (not found or not working)"
        fi
    done
}

# Show next steps
show_next_steps() {
    print_success "Dependencies installation completed!"
    echo ""
    echo -e "${BOLD}${GREEN}Next Steps:${NC}"
    echo -e "1. ${CYAN}Clone external libraries:${NC} ./scripts/clone_libraries.sh"
    echo -e "2. ${CYAN}Build the project:${NC} ./scripts/build.sh"
    echo -e "3. ${CYAN}Run the application:${NC} ./scripts/run.sh"
    echo ""
    echo -e "${BOLD}${GREEN}Docker Usage (if installed):${NC}"
    echo -e "1. ${CYAN}Build Docker image:${NC} ./scripts/docker-b-wt-builder.sh"
    echo -e "2. ${CYAN}Build production image:${NC} ./scripts/docker-wt-cv-stylus.sh"
    echo ""
    if command -v docker &> /dev/null && ! docker info &> /dev/null 2>&1; then
        echo -e "${YELLOW}Note: You may need to log out and back in for Docker to work properly.${NC}"
    fi
}

# Main script logic
print_status "Starting dependency installation for Wt CV Stylus project..."

# Perform checks
check_sudo
detect_os

# Install packages
update_packages
install_basic_tools
install_cpp_libraries
install_python_tools
install_nodejs
install_docker

# Verify and show next steps
verify_installations
show_next_steps

print_success "Dependency installation script completed successfully!"
print_status "Installation log saved to: $LOG_FILE"
