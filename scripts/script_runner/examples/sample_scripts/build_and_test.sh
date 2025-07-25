#!/bin/bash
#
# Example shell script demonstrating good practices for Script Runner.
#
# This script shows how to:
# - Provide clear status messages
# - Handle errors with proper exit codes
# - Report progress for long operations
# - Clean up resources on exit

set -euo pipefail  # Exit on error, undefined vars, pipe failures

# Configuration
SCRIPT_NAME="$(basename "$0")"
LOG_LEVEL="${LOG_LEVEL:-INFO}"
WORK_DIR="/tmp/script_runner_example"

# Logging function
log() {
    local level="$1"
    shift
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo "[$timestamp] $level: $*"
}

# Cleanup function
cleanup() {
    local exit_code=$?
    log "INFO" "Cleaning up temporary files"
    rm -rf "$WORK_DIR" 2>/dev/null || true
    
    if [ $exit_code -eq 0 ]; then
        log "INFO" "Script completed successfully"
    else
        log "ERROR" "Script exited with code $exit_code"
    fi
    
    exit $exit_code
}

# Set up cleanup on exit
trap cleanup EXIT INT TERM

# Progress reporting function
show_progress() {
    local current=$1
    local total=$2
    local task=$3
    local percent=$((current * 100 / total))
    log "INFO" "$task progress: $percent% ($current/$total)"
}

# Simulate file processing
process_files() {
    local count=${1:-10}
    log "INFO" "Processing $count files"
    
    mkdir -p "$WORK_DIR"
    
    for i in $(seq 1 $count); do
        show_progress $i $count "File Processing"
        
        # Simulate file creation and processing
        local filename="$WORK_DIR/file_$i.txt"
        echo "Sample data for file $i" > "$filename"
        
        # Simulate processing time
        sleep 0.5
        
        # Simulate occasional warnings
        if [ $((i % 3)) -eq 0 ]; then
            log "WARNING" "Large file detected: $filename"
        fi
        
        # Verify file was created
        if [ ! -f "$filename" ]; then
            log "ERROR" "Failed to create $filename"
            return 1
        fi
    done
    
    log "INFO" "File processing completed"
}

# Simulate system monitoring
monitor_system() {
    log "INFO" "Starting system monitoring"
    
    for i in {1..5}; do
        show_progress $i 5 "System Monitoring"
        
        # Get system information
        local load_avg=$(uptime | awk -F'load average:' '{print $2}' | awk '{print $1}' | tr -d ',')
        local disk_usage=$(df -h / | awk 'NR==2 {print $5}' | tr -d '%')
        
        log "INFO" "System status - Load: $load_avg, Disk usage: $disk_usage%"
        
        # Simulate monitoring interval
        sleep 1
        
        # Simulate alert condition
        if [ "$disk_usage" -gt 80 ]; then
            log "WARNING" "High disk usage detected: $disk_usage%"
        fi
    done
    
    log "INFO" "System monitoring completed"
}

# Main execution function
main() {
    log "INFO" "$SCRIPT_NAME starting"
    log "INFO" "Working directory: $WORK_DIR"
    log "INFO" "Process ID: $$"
    
    # Check prerequisites
    if ! command -v seq >/dev/null 2>&1; then
        log "ERROR" "Required command 'seq' not found"
        return 1
    fi
    
    # Execute main tasks
    process_files 8
    
    log "INFO" "Waiting between tasks..."
    sleep 2
    
    monitor_system
    
    # Generate summary report
    local file_count=$(find "$WORK_DIR" -name "*.txt" | wc -l)
    log "INFO" "Summary: Created $file_count files, monitored system 5 times"
    
    log "INFO" "All operations completed successfully"
    return 0
}

# Script entry point
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
