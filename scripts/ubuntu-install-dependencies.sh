#!/bin/bash
# Script to install Ubuntu dependencies for the Wt CV Stylus project
# Usage: ./scripts/ubuntu-install-dependencies.sh [options]

set -e  # Exit on any error

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="$SCRIPT_DIR/output"
LOG_FILE="$OUTPUT_DIR/ubuntu-install-dependencies.log"

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
    echo "  Installs all required Ubuntu dependencies for the Wt CV Stylus project"
    echo "  This includes build tools, libraries, Node.js, and development headers"
    echo "  Automatically installs missing dependencies without confirmation prompts"
    echo "  Requires sudo privileges for package installation"
    echo ""
    echo -e "${BOLD}${YELLOW}Options:${NC}"
    echo -e "  ${CYAN}-h, --help${NC}      Show this help message"
    echo -e "  ${CYAN}--dry-run${NC}       Show what would be installed without installing"
    echo ""
    echo -e "${BOLD}${YELLOW}Examples:${NC}"
    echo -e "  ${GREEN}$0${NC}              # Install all missing dependencies"
    echo -e "  ${GREEN}$0 --dry-run${NC}    # Show what would be installed"
}

# Variables
AUTO_YES="-y"  # Default to auto-confirm installations
DRY_RUN=false

# Argument parsing (REQUIRED)
while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            show_usage
            exit 0
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Check if running on Ubuntu
check_ubuntu() {
    if ! grep -q "ubuntu\|Ubuntu" /etc/os-release 2>/dev/null; then
        print_error "This script is designed for Ubuntu systems only."
        print_error "Detected OS: $(lsb_release -d 2>/dev/null | cut -f2 || echo 'Unknown')"
        exit 1
    fi
    
    local ubuntu_version=$(lsb_release -rs 2>/dev/null || echo "Unknown")
    print_status "Detected Ubuntu version: $ubuntu_version"
}

# Check if running in Docker environment
is_docker_environment() {
    # Check for .dockerenv file (standard Docker indicator)
    if [ -f /.dockerenv ]; then
        return 0  # In Docker
    fi
    
    # Check if running as root and no sudo available (typical Docker scenario)
    if [ "$EUID" -eq 0 ] && ! command -v sudo &> /dev/null; then
        return 0  # Likely in Docker
    fi
    
    return 1  # Not in Docker
}

# Check sudo privileges
check_sudo() {
    if [ "$DRY_RUN" = true ]; then
        return 0
    fi
    
    # If in Docker environment, check if we're root
    if is_docker_environment; then
        if [ "$EUID" -eq 0 ]; then
            print_status "Running in Docker container as root - no sudo needed"
            return 0
        else
            print_error "Running in Docker container but not as root. This is unexpected."
            exit 1
        fi
    fi
    
    # Host system - check sudo
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

# Define package lists
declare -a BASIC_PACKAGES=(
    "git"
    "cmake"
    "build-essential"
    "curl"
    "gnupg"
    "software-properties-common"
)

declare -a DEVELOPMENT_PACKAGES=(
    "libboost-all-dev"
    "libssl-dev"
    "libcrypto++-dev"
    "libpq-dev"
    "libunistring-dev"
    "pkg-config"
    "python3"
    "python3-pip"
    "python3-venv"
    "libpsl-dev"
)

declare -a OPTIONAL_PACKAGES=(
    "htop"
    "tree"
    "vim"
    "nano"
)

# Install packages function
install_packages() {
    local package_array=("$@")
    local package_list="${package_array[*]}"
    
    if [ "$DRY_RUN" = true ]; then
        print_status "[DRY RUN] Would install: $package_list"
        return 0
    fi
    
    print_status "Installing packages: $package_list"
    
    # Use appropriate command based on environment
    if is_docker_environment || [ "$EUID" -eq 0 ]; then
        if apt install $AUTO_YES $package_list 2>&1 | tee -a "$LOG_FILE"; then
            print_success "Successfully installed: $package_list"
        else
            print_error "Failed to install some packages: $package_list"
            return 1
        fi
    else
        if sudo apt install $AUTO_YES $package_list 2>&1 | tee -a "$LOG_FILE"; then
            print_success "Successfully installed: $package_list"
        else
            print_error "Failed to install some packages: $package_list"
            return 1
        fi
    fi
}

# Update package lists
update_package_lists() {
    if [ "$DRY_RUN" = true ]; then
        print_status "[DRY RUN] Would update package lists"
        return 0
    fi
    
    print_status "Updating package lists..."
    
    # Use appropriate command based on environment
    if is_docker_environment || [ "$EUID" -eq 0 ]; then
        if apt update 2>&1 | tee -a "$LOG_FILE"; then
            print_success "Package lists updated successfully"
        else
            print_error "Failed to update package lists"
            exit 1
        fi
    else
        if sudo apt update 2>&1 | tee -a "$LOG_FILE"; then
            print_success "Package lists updated successfully"
        else
            print_error "Failed to update package lists"
            exit 1
        fi
    fi
}

# Install Node.js
install_nodejs() {
    if command -v node &> /dev/null; then
        local node_version=$(node --version)
        print_status "Node.js already installed: $node_version"
        
        # Check if it's version 20.x
        if [[ $node_version == v20.* ]]; then
            print_success "Node.js 20.x is already installed"
            return 0
        else
            print_warning "Node.js version is not 20.x, will install Node.js 20.x"
        fi
    fi
    
    if [ "$DRY_RUN" = true ]; then
        print_status "[DRY RUN] Would install Node.js 20.x and npm"
        return 0
    fi
    
    print_status "Installing Node.js 20.x..."
    
    # Add NodeSource repository
    if is_docker_environment || [ "$EUID" -eq 0 ]; then
        if curl -fsSL https://deb.nodesource.com/setup_20.x | bash - 2>&1 | tee -a "$LOG_FILE"; then
            print_success "NodeSource repository added successfully"
        else
            print_error "Failed to add NodeSource repository"
            return 1
        fi
    else
        if curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash - 2>&1 | tee -a "$LOG_FILE"; then
            print_success "NodeSource repository added successfully"
        else
            print_error "Failed to add NodeSource repository"
            return 1
        fi
    fi
    
    # Install Node.js and npm
    if is_docker_environment || [ "$EUID" -eq 0 ]; then
        if apt install $AUTO_YES nodejs 2>&1 | tee -a "$LOG_FILE"; then
            local installed_version=$(node --version)
            print_success "Node.js installed successfully: $installed_version"
            
            local npm_version=$(npm --version)
            print_success "npm version: $npm_version"
        else
            print_error "Failed to install Node.js"
            return 1
        fi
    else
        if sudo apt install $AUTO_YES nodejs 2>&1 | tee -a "$LOG_FILE"; then
            local installed_version=$(node --version)
            print_success "Node.js installed successfully: $installed_version"
            
            local npm_version=$(npm --version)
            print_success "npm version: $npm_version"
        else
            print_error "Failed to install Node.js"
            return 1
        fi
    fi
}

# Install Python Meson and Ninja
install_python_tools() {
    if [ "$DRY_RUN" = true ]; then
        print_status "[DRY RUN] Would install Python tools (meson, ninja)"
        return 0
    fi
    
    print_status "Installing Python build tools..."
    
    # Create virtual environment if it doesn't exist
    local venv_path="/opt/venv"
    if [ ! -d "$venv_path" ]; then
        print_status "Creating Python virtual environment at $venv_path..."
        if is_docker_environment || [ "$EUID" -eq 0 ]; then
            if python3 -m venv "$venv_path" 2>&1 | tee -a "$LOG_FILE"; then
                print_success "Virtual environment created"
            else
                print_error "Failed to create virtual environment"
                return 1
            fi
        else
            if sudo python3 -m venv "$venv_path" 2>&1 | tee -a "$LOG_FILE"; then
                print_success "Virtual environment created"
            else
                print_error "Failed to create virtual environment"
                return 1
            fi
        fi
    fi
    
    # Install meson and ninja in the virtual environment
    print_status "Installing meson and ninja..."
    if is_docker_environment || [ "$EUID" -eq 0 ]; then
        if "$venv_path/bin/pip" install --upgrade pip 2>&1 | tee -a "$LOG_FILE" && \
           "$venv_path/bin/pip" install 'meson==1.3.0' ninja 2>&1 | tee -a "$LOG_FILE"; then
            print_success "Python build tools installed successfully"
        else
            print_error "Failed to install Python build tools"
            return 1
        fi
    else
        if sudo "$venv_path/bin/pip" install --upgrade pip 2>&1 | tee -a "$LOG_FILE" && \
           sudo "$venv_path/bin/pip" install 'meson==1.3.0' ninja 2>&1 | tee -a "$LOG_FILE"; then
            print_success "Python build tools installed successfully"
        else
            print_error "Failed to install Python build tools"
            return 1
        fi
    fi
}

# Show summary
show_summary() {
    echo ""
    print_success "=== Installation Summary ==="
    print_status "Basic packages: ${BASIC_PACKAGES[*]}"
    print_status "Development packages: ${DEVELOPMENT_PACKAGES[*]}"
    print_status "Node.js 20.x and npm"
    print_status "Python tools: meson, ninja (in /opt/venv)"
    
    if [ "$DRY_RUN" != true ]; then
        echo ""
        print_success "All dependencies installed successfully!"
        print_status "You can now build the project with: ./scripts/build.sh"
        print_status "Or use Docker with: ./scripts/docker-b-wt-builder.sh"
    fi
}

# Main script logic
print_status "Starting Ubuntu dependency installation for Wt CV Stylus project..."

if [ "$DRY_RUN" = true ]; then
    print_warning "DRY RUN MODE - No packages will be installed"
fi

# Perform checks
check_ubuntu
check_sudo

# Update package lists
update_package_lists

# Install basic packages
print_status "Installing basic build tools and utilities..."
install_packages "${BASIC_PACKAGES[@]}"

# Install development libraries
print_status "Installing development libraries and headers..."
install_packages "${DEVELOPMENT_PACKAGES[@]}"

# Install Node.js
install_nodejs

# Install Python tools
install_python_tools

# Show completion summary
show_summary

print_success "Ubuntu dependency installation completed successfully!"
print_status "Installation log saved to: $LOG_FILE"
