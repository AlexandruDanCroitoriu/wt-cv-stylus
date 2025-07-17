#!/bin/bash

# ===========================================================================================
# Memory Analyzer Script - Continuous memory analysis for processes
# ===========================================================================================
# Description: Provides comprehensive memory analysis for a given process with continuous 
#              monitoring capability. Part of the unified wt-cv-stylus-copilot script ecosystem.
# Usage:       ./memory_analyzer.sh [OPTIONS] <PID>
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
    echo -e "${BOLD}${BLUE}MEMORY ANALYZER${NC} ${CYAN}v1.0.0${NC}"
    echo -e "${BOLD}${WHITE}===============================================${NC}"
    echo
    echo -e "${BOLD}DESCRIPTION:${NC}"
    echo -e "  Comprehensive memory analysis tool for the application running on port 9020."
    echo -e "  Automatically detects and analyzes the process, providing detailed memory statistics."
    echo -e "  ${YELLOW}Note: This script performs a single analysis and exits (used by memory_monitor.sh)${NC}"
    echo
    echo -e "${BOLD}USAGE:${NC}"
    echo -e "  ${GREEN}$0${NC} ${YELLOW}[OPTIONS]${NC}"
    echo
    echo -e "${BOLD}OPTIONS:${NC}"
    echo -e "  ${YELLOW}-h, --help${NC}        Show this help message"
    echo -e "  ${YELLOW}-o, --output${NC}      Output file for results (default: stdout)"
    echo
    echo -e "${BOLD}EXAMPLES:${NC}"
    echo -e "  ${GREEN}$0${NC}                          # Analyze app on port 9020"
    echo -e "  ${GREEN}$0${NC} ${YELLOW}-o analysis.log${NC}       # Save analysis to file"
    echo
    echo -e "${BOLD}INTEGRATION:${NC}"
    echo -e "  This script follows wt-cv-stylus-copilot development standards and integrates"
    echo -e "  with the unified script ecosystem. Logs are written to:"
    echo -e "  ${CYAN}$LOG_FILE${NC}"
    echo
}

# ===========================================================================================
# UTILITY FUNCTIONS FOR PID DETECTION
# ===========================================================================================

# Function to find PID of process running on port 9020
find_port_9020_pid() {
    local pid=$(lsof -ti:9020 2>/dev/null | head -1)
    if [ -n "$pid" ]; then
        echo "$pid"
        return 0
    else
        return 1
    fi
}

# ===========================================================================================
# ARGUMENT PARSING AND VALIDATION
# ===========================================================================================

# Default values
OUTPUT_FILE=""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
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
            echo "This script automatically detects the PID from port 9020."
            echo "Use $0 --help for usage information."
            exit 1
            ;;
    esac
done

# Auto-detect PID from port 9020
log_info "Auto-detecting PID for application on port 9020..."

if ! command -v lsof >/dev/null 2>&1; then
    log_error "lsof command not found. Please install lsof package."
    exit 1
fi

PID=$(find_port_9020_pid)
if [ -z "$PID" ]; then
    log_error "No application found running on port 9020"
    echo -e "${YELLOW}Hint: Start the application first with ${CYAN}./scripts/run.sh${NC}"
    exit 1
fi

log_info "Found application with PID $PID running on port 9020"

# Validate detected PID
if ! [[ "$PID" =~ ^[0-9]+$ ]]; then
    log_error "Invalid PID detected: $PID"
    exit 1
fi

# Check if process exists (double-check)
if ! kill -0 "$PID" 2>/dev/null; then
    log_error "Process with PID $PID does not exist or insufficient permissions"
    exit 1
fi

# Set default output file if not specified (empty means stdout)
if [ -z "$OUTPUT_FILE" ]; then
    OUTPUT_TO_STDOUT=true
else
    OUTPUT_TO_STDOUT=false
fi

# ===========================================================================================
# UTILITY FUNCTIONS
# ===========================================================================================

# Function to convert KB to human readable format
kb_to_human() {
    local kb=$1
    if [ -z "$kb" ] || [ "$kb" = "0" ]; then
        echo "0 B"
        return
    fi
    
    if [ $kb -lt 1024 ]; then
        echo "${kb} KB"
    elif [ $kb -lt 1048576 ]; then
        mb=$((kb / 1024))
        echo "${mb} MB"
    else
        gb=$((kb / 1048576))
        echo "${gb} GB"
    fi
}

# Function to format numbers with thousand separators
format_number() {
    printf "%'d" $1 2>/dev/null || echo $1
}

# Function to get comprehensive memory analysis for a process
analyze_process_memory() {
    local pid=$1
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    
    log_info "Starting memory analysis for PID $pid at $timestamp"
    
    # Check if process still exists
    if ! kill -0 "$pid" 2>/dev/null; then
        log_error "Process $pid no longer exists"
        return 1
    fi
    
    local status_file="/proc/$pid/status"
    local stat_file="/proc/$pid/stat"
    local maps_file="/proc/$pid/maps"
    local smaps_file="/proc/$pid/smaps"
    
    # Check if we can read process files
    if [ ! -r "$status_file" ]; then
        log_error "Cannot read $status_file - insufficient permissions"
        return 1
    fi
    
    echo
    echo -e "${BOLD}${CYAN}================================================================${NC}"
    echo -e "${BOLD}${CYAN}    COMPREHENSIVE MEMORY ANALYSIS - PID $pid${NC}"
    echo -e "${BOLD}${CYAN}    Timestamp: $timestamp${NC}"
    echo -e "${BOLD}${CYAN}================================================================${NC}"
    
    # Extract process information
    local cmd=$(cat /proc/$pid/comm 2>/dev/null || echo "unknown")
    local cmdline=$(cat /proc/$pid/cmdline 2>/dev/null | tr '\0' ' ' || echo "unknown")
    
    echo -e "${BLUE}Process Information:${NC}"
    echo -e "  Command: ${GREEN}$cmd${NC}"
    echo -e "  Command Line: ${GREEN}$cmdline${NC}"
    echo
    
    # Extract memory information from /proc/PID/status
    echo "MEMORY_DATA_START"
    printf "%-12s | %-15s | %-15s\n" "Metric" "Value (KB)" "Value (MB)"
    printf "%-12s-+-%-15s-+-%-15s\n" "------------" "---------------" "---------------"
    
    # Parse status file for memory metrics
    while IFS=: read -r key value; do
        case "$key" in
            VmPeak|VmSize|VmLck|VmPin|VmHWM|VmRSS|VmData|VmStk|VmExe|VmLib|VmPTE|VmSwap|RssAnon|RssFile|RssShmem)
                # Extract numeric value (remove kB suffix and whitespace)
                local kb_value=$(echo "$value" | sed 's/[^0-9]//g')
                if [ -n "$kb_value" ] && [ "$kb_value" -gt 0 ]; then
                    local mb_value=$((kb_value / 1024))
                    printf "%-12s | %'15d | %'15d\n" "$key" "$kb_value" "$mb_value"
                fi
                ;;
            Threads)
                local thread_count=$(echo "$value" | sed 's/[^0-9]//g')
                printf "%-12s | %'15s | %'15s\n" "$key" "$thread_count" "N/A"
                ;;
        esac
    done < "$status_file"
    echo "MEMORY_DATA_END"
    echo
    
    # Memory mapping analysis
    if [ -r "$smaps_file" ]; then
        echo -e "${BLUE}Memory Mapping Analysis:${NC}"
        
        # Calculate totals from smaps
        local total_size=$(awk '/^Size:/ {sum+=$2} END {print sum}' "$smaps_file" 2>/dev/null || echo 0)
        local total_rss=$(awk '/^Rss:/ {sum+=$2} END {print sum}' "$smaps_file" 2>/dev/null || echo 0)
        local total_pss=$(awk '/^Pss:/ {sum+=$2} END {print sum}' "$smaps_file" 2>/dev/null || echo 0)
        local total_dirty=$(awk '/^Private_Dirty:/ {sum+=$2} END {print sum}' "$smaps_file" 2>/dev/null || echo 0)
        
        echo "  Total Virtual: $(kb_to_human $total_size)"
        echo "  Total RSS: $(kb_to_human $total_rss)"
        echo "  Total PSS: $(kb_to_human $total_pss)"
        echo "  Total Dirty: $(kb_to_human $total_dirty)"
        echo
        
        # Memory efficiency assessment
        if [ "$total_rss" -gt 0 ]; then
            local rss_mb=$((total_rss / 1024))
            echo -e "${BLUE}Memory Efficiency Assessment:${NC}"
            if [ $rss_mb -lt 50 ]; then
                echo -e "  ${GREEN}Low memory usage - very efficient${NC} (< 50 MB)"
            elif [ $rss_mb -lt 200 ]; then
                echo -e "  ${YELLOW}Moderate memory usage - reasonable${NC} (50-200 MB)"
            elif [ $rss_mb -lt 500 ]; then
                echo -e "  ${PURPLE}High memory usage - monitor${NC} (200-500 MB)"
            else
                echo -e "  ${RED}Very high memory usage - investigate${NC} (> 500 MB)"
            fi
            echo
        fi
    fi
    
    # Memory regions breakdown
    if [ -r "$maps_file" ]; then
        echo -e "${BLUE}Memory Regions Summary:${NC}"
        local heap_count=$(grep -c "\[heap\]" "$maps_file" 2>/dev/null || echo 0)
        local stack_count=$(grep -c "\[stack" "$maps_file" 2>/dev/null || echo 0)
        local lib_count=$(grep -c "\.so" "$maps_file" 2>/dev/null || echo 0)
        local total_regions=$(wc -l < "$maps_file" 2>/dev/null || echo 0)
        
        echo "  Total Memory Regions: $total_regions"
        echo "  Heap Regions: $heap_count"
        echo "  Stack Regions: $stack_count"
        echo "  Shared Libraries: $lib_count"
        echo
    fi
    
    log_info "Memory analysis completed for PID $pid"
    return 0
}

# ===========================================================================================
# SIGNAL HANDLING
# ===========================================================================================

# Set up signal handler for graceful shutdown
cleanup() {
    log_info "Received interrupt signal - stopping memory analyzer"
    exit 0
}
trap cleanup SIGINT SIGTERM

# ===========================================================================================
# MAIN EXECUTION LOGIC
# ===========================================================================================

log_info "Memory analyzer started for single analysis of PID: $PID"

# Perform single analysis
if [ "$OUTPUT_TO_STDOUT" = true ]; then
    # Output to stdout (for use by memory monitor)
    analyze_process_memory "$PID"
    exit_code=$?
else
    # Output to file
    {
        echo "=== Memory Analysis - $(date) ==="
        analyze_process_memory "$PID"
    } > "$OUTPUT_FILE"
    exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}Analysis complete. Results saved to: $OUTPUT_FILE${NC}"
    fi
fi

if [ $exit_code -eq 0 ]; then
    log_info "Memory analysis completed successfully"
else
    log_error "Memory analysis failed"
fi

exit $exit_code
