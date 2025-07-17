#!/bin/bash

# ===========================================================================================
# Memory Monitor Script - Real-time memory monitoring display
# ===========================================================================================
# Description: Provides real-time memory monitoring display for processes using the memory
#              analyzer. Part of the unified wt-cv-stylus-copilot script ecosystem.
# Usage:       ./memory_monitor.sh [OPTIONS] <PID>
# Author:      Script development following wt-cv-stylus-copilot standards
# Created:     $(date +%Y-%m-%d)
# Version:     1.0.0
# ===========================================================================================

# ===========================================================================================
# INITIALIZATION AND SETUP
# ===========================================================================================

# Get script directory and name for standardized logging and operations
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCRIPT_NAME="$(basename "$0" .sh)"
TIMESTAMP="$(date +%Y%m%d_%H%M%S)"

# Initialize LOG_FILE immediately to prevent undefined variable errors
LOG_FILE="$SCRIPT_DIR/output/${SCRIPT_NAME}.log"

# Ensure output directory exists
mkdir -p "$SCRIPT_DIR/output"

# ===========================================================================================
# STANDARD COLOR DEFINITIONS (Following project standards)
# ===========================================================================================
readonly RED='\033[0;31m'
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'
readonly CYAN='\033[0;36m'
readonly PURPLE='\033[0;35m'
readonly WHITE='\033[1;37m'
readonly BOLD='\033[1m'
readonly NC='\033[0m'

# ===========================================================================================
# LOGGING AND OUTPUT FUNCTIONS (Mandatory for all scripts)
# ===========================================================================================

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1" | tee -a "$LOG_FILE"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1" | tee -a "$LOG_FILE"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" | tee -a "$LOG_FILE"
}

log_debug() {
    echo -e "${CYAN}[DEBUG]${NC} $1" | tee -a "$LOG_FILE"
}

# ===========================================================================================
# HELP AND USAGE FUNCTIONS (Mandatory colorized help)
# ===========================================================================================

show_help() {
    echo -e "${BOLD}${BLUE}MEMORY MONITOR${NC} ${CYAN}v1.0.0${NC}"
    echo -e "${BOLD}${WHITE}===============================================${NC}"
    echo
    echo -e "${BOLD}DESCRIPTION:${NC}"
    echo -e "  Real-time memory monitoring display for the application on port 9020."
    echo -e "  Automatically detects and monitors the process with live tabular display."
    echo
    echo -e "${BOLD}USAGE:${NC}"
    echo -e "  ${GREEN}$0${NC} ${YELLOW}[OPTIONS]${NC}"
    echo
    echo -e "${BOLD}OPTIONS:${NC}"
    echo -e "  ${YELLOW}-h, --help${NC}        Show this help message"
    echo -e "  ${YELLOW}-i, --interval${NC}    Set monitoring interval in seconds (default: 1)"
    echo -e "  ${YELLOW}-o, --output${NC}      Output file for monitoring log (default: auto-generated)"
    echo
    echo -e "${BOLD}EXAMPLES:${NC}"
    echo -e "  ${GREEN}$0${NC}                          # Monitor app on port 9020 every second"
    echo -e "  ${GREEN}$0${NC} ${YELLOW}-i 5${NC}                   # Monitor every 5 seconds"
    echo -e "  ${GREEN}$0${NC} ${YELLOW}-o monitor.log${NC}        # Save monitoring log to custom file"
    echo
    echo -e "${BOLD}INTEGRATION:${NC}"
    echo -e "  This script integrates with ${CYAN}memory_analyzer.sh${NC} and follows"
    echo -e "  wt-cv-stylus-copilot development standards. Logs are written to:"
    echo -e "  ${CYAN}$LOG_FILE${NC}"
    echo
    echo -e "${BOLD}CONTROLS:${NC}"
    echo -e "  ${YELLOW}Ctrl+C${NC}            Stop monitoring gracefully"
    echo
}

# ===========================================================================================
# ARGUMENT PARSING AND VALIDATION
# ===========================================================================================

# Default values
MONITOR_INTERVAL=1
OUTPUT_FILE=""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -i|--interval)
            MONITOR_INTERVAL="$2"
            if ! [[ "$MONITOR_INTERVAL" =~ ^[0-9]+$ ]] || [ "$MONITOR_INTERVAL" -lt 1 ]; then
                log_error "Invalid interval value: $MONITOR_INTERVAL (must be a positive integer)"
                exit 1
            fi
            shift 2
            ;;
        -o|--output)
            OUTPUT_FILE="$2"
            shift 2
            ;;
        -*)
            log_error "Unknown option: $1"
            echo "Use $0 --help for usage information."
            exit 1
            ;;
        *)
            log_error "Unexpected argument: $1"
            echo "This script automatically detects the process from port 9020."
            echo "Use $0 --help for usage information."
            exit 1
            ;;
    esac
done

# The memory analyzer will handle PID detection, so we don't need to do it here
# Set default output file if not specified
if [ -z "$OUTPUT_FILE" ]; then
    OUTPUT_FILE="$SCRIPT_DIR/output/${SCRIPT_NAME}.log"
fi

# ===========================================================================================
# INTEGRATION WITH MEMORY ANALYZER
# ===========================================================================================

# Check if memory analyzer exists and is executable
MEMORY_ANALYZER="$SCRIPT_DIR/memory_analyzer.sh"
if [ ! -f "$MEMORY_ANALYZER" ]; then
    log_error "Memory analyzer not found at $MEMORY_ANALYZER"
    exit 1
fi

if [ ! -x "$MEMORY_ANALYZER" ]; then
    log_error "Memory analyzer is not executable at $MEMORY_ANALYZER"
    exit 1
fi

# ===========================================================================================
# UTILITY FUNCTIONS
# ===========================================================================================

# Function to get comprehensive memory info using memory analyzer
get_memory_info() {
    local temp_output=$(mktemp)
    
    # Run memory analyzer and capture output (analyzer auto-detects PID from port 9020)
    if ! "$MEMORY_ANALYZER" > "$temp_output" 2>/dev/null; then
        echo "ERROR"
        rm -f "$temp_output"
        return 1
    fi
    
    # Extract key information from the structured data table
    local rss_mb=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^VmRSS" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    local vmsize_mb=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^VmSize" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    local vmpeak_mb=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^VmPeak" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    local vmdata_mb=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^VmData" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    local rssanon_mb=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^RssAnon" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    local rssfile_mb=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^RssFile" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    local vmlib_mb=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^VmLib" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    local threads=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^Threads" | awk -F'|' '{gsub(/ /, "", $2); print $2}')
    
    # Extract memory mapping info
    local total_virtual=$(grep "Total Virtual:" "$temp_output" | sed 's/.*Total Virtual: \([^[:space:]]*[[:space:]]*[^[:space:]]*\).*/\1/' | tr -d '\n')
    local total_rss=$(grep "Total RSS:" "$temp_output" | sed 's/.*Total RSS: \([^[:space:]]*[[:space:]]*[^[:space:]]*\).*/\1/' | tr -d '\n')
    local total_dirty=$(grep "Total Dirty:" "$temp_output" | sed 's/.*Total Dirty: \([^[:space:]]*[[:space:]]*[^[:space:]]*\).*/\1/' | tr -d '\n')
    
    # Extract process name
    local process_name=$(grep "Command:" "$temp_output" | sed 's/.*Command: \([^[:space:]]*\).*/\1/' | tr -d '\n')
    
    # Extract efficiency assessment
    local efficiency=""
    if grep -q "Low memory usage - very efficient" "$temp_output"; then
        efficiency="Efficient"
    elif grep -q "Moderate memory usage - reasonable" "$temp_output"; then
        efficiency="Moderate"
    elif grep -q "High memory usage - monitor" "$temp_output"; then
        efficiency="High"
    elif grep -q "Very high memory usage - investigate" "$temp_output"; then
        efficiency="VeryHigh"
    else
        efficiency="Unknown"
    fi
    
    # Clean up temp file
    rm -f "$temp_output"
    
    # Validate we got core values
    if [ -z "$rss_mb" ] || [ -z "$vmsize_mb" ] || [ -z "$threads" ]; then
        echo "ERROR"
        return 1
    fi
    
    # Return the extracted data (using | as separator)
    echo "$rss_mb|$vmsize_mb|$vmpeak_mb|$vmdata_mb|$rssanon_mb|$rssfile_mb|$vmlib_mb|$threads|$total_virtual|$total_rss|$total_dirty|$process_name|$efficiency"
}

# ===========================================================================================
# SIGNAL HANDLING
# ===========================================================================================

# Set up signal handler for graceful shutdown
cleanup() {
    log_info "Received interrupt signal - stopping memory monitor"
    echo
    echo -e "${CYAN}Memory monitoring stopped.${NC}"
    echo -e "${YELLOW}Total monitoring samples: $sample_count${NC}"
    echo -e "${GREEN}Monitoring log saved to: $OUTPUT_FILE${NC}"
    exit 0
}
trap cleanup SIGINT SIGTERM

# ===========================================================================================
# MAIN EXECUTION LOGIC
# ===========================================================================================

# Initialize sample counter
sample_count=0

log_info "Memory monitor started for port 9020 application, Interval: ${MONITOR_INTERVAL}s"

echo -e "${BOLD}${CYAN}================================================================${NC}"
echo -e "${BOLD}${CYAN}    REAL-TIME MEMORY MONITOR FOR PORT 9020 APPLICATION${NC}"
echo -e "${BOLD}${CYAN}================================================================${NC}"
echo -e "${GREEN}Monitoring memory usage every $MONITOR_INTERVAL second(s)...${NC}"
echo -e "${YELLOW}Press Ctrl+C to stop monitoring${NC}"
echo -e "${CYAN}Output file: $OUTPUT_FILE${NC}"
echo

# Display headers for the comprehensive table
printf "${BOLD}${BLUE}%-8s | %-7s | %-9s | %-8s | %-8s | %-8s | %-7s | %-6s | %-7s | %-12s | %-12s${NC}\n" \
       "Time" "RSS(MB)" "VmSize(MB)" "Peak(MB)" "Data(MB)" "Anon(MB)" "File(MB)" "Lib(MB)" "Threads" "TotalVirt" "Efficiency"
       
printf "${BLUE}%-8s-+-%-7s-+-%-9s-+-%-8s-+-%-8s-+-%-8s-+-%-7s-+-%-6s-+-%-7s-+-%-12s-+-%-12s${NC}\n" \
       "--------" "-------" "---------" "--------" "--------" "--------" "-------" "------" "-------" "------------" "------------"

# Start monitoring log
{
    echo "=== Memory Monitor Log Started - $(date) ==="
    echo "Target: Application on port 9020"
    echo "Interval: ${MONITOR_INTERVAL}s"
    echo ""
    printf "%-8s | %-7s | %-9s | %-8s | %-8s | %-8s | %-7s | %-6s | %-7s | %-12s | %-12s\n" \
           "Time" "RSS(MB)" "VmSize(MB)" "Peak(MB)" "Data(MB)" "Anon(MB)" "File(MB)" "Lib(MB)" "Threads" "TotalVirt" "Efficiency"
    printf "%-8s-+-%-7s-+-%-9s-+-%-8s-+-%-8s-+-%-8s-+-%-7s-+-%-6s-+-%-7s-+-%-12s-+-%-12s\n" \
           "--------" "-------" "---------" "--------" "--------" "--------" "-------" "------" "-------" "------------" "------------"
} > "$OUTPUT_FILE"

# Main monitoring loop
while true; do
    # Get current time
    current_time=$(date '+%H:%M:%S')
    
    # Get memory information using memory analyzer (analyzer handles PID detection)
    memory_info=$(get_memory_info)
    
    if [ "$memory_info" != "ERROR" ] && [ -n "$memory_info" ]; then
        # Parse the comprehensive memory info
        IFS='|' read -r rss_mb vmsize_mb vmpeak_mb vmdata_mb rssanon_mb rssfile_mb vmlib_mb threads total_virtual total_rss total_dirty process_name efficiency <<< "$memory_info"
        
        # Truncate values for display
        efficiency_short=$(echo "$efficiency" | cut -c1-12)
        total_virtual_short=$(echo "$total_virtual" | cut -c1-12)
        
        # Color code efficiency
        efficiency_color=""
        case "$efficiency" in
            "Efficient") efficiency_color="${GREEN}" ;;
            "Moderate") efficiency_color="${YELLOW}" ;;
            "High") efficiency_color="${PURPLE}" ;;
            "VeryHigh") efficiency_color="${RED}" ;;
            *) efficiency_color="${NC}" ;;
        esac
        
        # Display the information in a comprehensive table format
        printf "${NC}%-8s | ${GREEN}%-7s${NC} | %-9s | ${YELLOW}%-8s${NC} | %-8s | %-8s | %-7s | %-6s | ${CYAN}%-7s${NC} | %-12s | ${efficiency_color}%-12s${NC}\n" \
               "$current_time" "$rss_mb" "$vmsize_mb" "$vmpeak_mb" "$vmdata_mb" "$rssanon_mb" "$rssfile_mb" "$vmlib_mb" "$threads" "$total_virtual_short" "$efficiency_short"
        
        # Log to file in tabular format
        printf "%-8s | %-7s | %-9s | %-8s | %-8s | %-8s | %-7s | %-6s | %-7s | %-12s | %-12s\n" \
               "$current_time" "$rss_mb" "$vmsize_mb" "$vmpeak_mb" "$vmdata_mb" "$rssanon_mb" "$rssfile_mb" "$vmlib_mb" "$threads" "$total_virtual_short" "$efficiency_short" >> "$OUTPUT_FILE"
        
        # Every 20 samples, show detailed summary and refresh headers
        if [ $((sample_count % 20)) -eq 0 ] && [ $sample_count -gt 0 ]; then
            echo
            echo -e "${CYAN}--- Detailed Summary (Sample #$sample_count) ---${NC}"
            echo -e "${BLUE}Process:${NC} $process_name (Port 9020 Application)"
            echo -e "${BLUE}Physical Memory (RSS):${NC} ${GREEN}$rss_mb MB${NC} - Actual RAM usage"
            echo -e "${BLUE}Virtual Memory:${NC} $vmsize_mb MB (Peak: $vmpeak_mb MB)"
            echo -e "${BLUE}Memory Breakdown:${NC} Data: $vmdata_mb MB, Anonymous: $rssanon_mb MB, Files: $rssfile_mb MB, Libraries: $vmlib_mb MB"
            echo -e "${BLUE}Thread Count:${NC} ${CYAN}$threads${NC}"
            echo -e "${BLUE}Memory Mapping:${NC} Virtual: $total_virtual, RSS: $total_rss, Dirty: $total_dirty"
            echo -e "${BLUE}Efficiency Assessment:${NC} ${efficiency_color}$efficiency${NC}"
            echo -e "${CYAN}--- Continuing monitoring... ---${NC}"
            echo
            
            # Redisplay headers
            printf "${BOLD}${BLUE}%-8s | %-7s | %-9s | %-8s | %-8s | %-8s | %-7s | %-6s | %-7s | %-12s | %-12s${NC}\n" \
                   "Time" "RSS(MB)" "VmSize(MB)" "Peak(MB)" "Data(MB)" "Anon(MB)" "File(MB)" "Lib(MB)" "Threads" "TotalVirt" "Efficiency"
        fi
        
        # Increment counter
        ((sample_count++))
    else
        echo -e "${RED}$current_time - Failed to capture memory data${NC}"
        log_warn "Failed to capture memory data for port 9020 application at $current_time"
    fi
    
    # Wait for the specified interval
    sleep "$MONITOR_INTERVAL"
done

# Final cleanup
cleanup
