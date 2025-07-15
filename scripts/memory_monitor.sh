#!/bin/bash

# Memory Monitor Script using Memory Analyzer
# Usage: ./memory_monitor.sh <PID>
# Monitors memory usage every second and displays in terminal

# Colors for output formatting
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Check if PID argument is provided
if [ $# -eq 0 ]; then
    echo -e "${RED}Error: Please provide a PID as an argument${NC}"
    echo "Usage: $0 <PID>"
    exit 1
fi

PID=$1

# Validate PID is a number
if ! [[ "$PID" =~ ^[0-9]+$ ]]; then
    echo -e "${RED}Error: PID must be a number${NC}"
    exit 1
fi

# Check if process exists
if ! kill -0 "$PID" 2>/dev/null; then
    echo -e "${RED}Error: Process with PID $PID does not exist or you don't have permission to access it${NC}"
    exit 1
fi

# Get script directory and memory analyzer path
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MEMORY_ANALYZER="$SCRIPT_DIR/memory_analyzer.sh"

# Check if memory analyzer exists
if [ ! -f "$MEMORY_ANALYZER" ]; then
    echo -e "${RED}Error: memory_analyzer.sh not found at $MEMORY_ANALYZER${NC}"
    exit 1
fi

# Function to get comprehensive memory info using memory analyzer
get_memory_info() {
    local pid=$1
    local temp_output=$(mktemp)
    
    # Run memory analyzer and capture output
    "$MEMORY_ANALYZER" "$pid" > "$temp_output" 2>/dev/null
    
    if [ $? -ne 0 ]; then
        echo "ERROR"
        rm -f "$temp_output"
        return 1
    fi
    
    # Extract key information from the structured data table
    local rss_mb=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^RSS" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    local vmsize_mb=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^VmSize" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    local vmpeak_mb=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^VmPeak" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    local vmdata_mb=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^VmData" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    local rssanon_mb=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^RssAnon" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    local rssfile_mb=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^RssFile" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    local vmlib_mb=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^VmLib" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    local threads=$(sed -n '/MEMORY_DATA_START/,/MEMORY_DATA_END/p' "$temp_output" | grep "^Threads" | awk -F'|' '{gsub(/ /, "", $3); print $3}')
    
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

# Set up signal handler for Ctrl+C
cleanup() {
    echo
    echo -e "${CYAN}Monitoring stopped.${NC}"
    echo -e "${YELLOW}Total samples: $count${NC}"
    exit 0
}
trap cleanup SIGINT

# Initialize counter
count=0

echo -e "${CYAN}================================================================${NC}"
echo -e "${CYAN}    Comprehensive Memory Monitor for Process $PID${NC}"
echo -e "${CYAN}================================================================${NC}"
echo -e "${GREEN}Monitoring memory usage every second...${NC}"
echo -e "${YELLOW}Press Ctrl+C to stop${NC}"
echo

# Display headers for the comprehensive table
printf "${BLUE}%-8s | %-7s | %-9s | %-8s | %-8s | %-8s | %-7s | %-6s | %-7s | %-12s | %-12s${NC}\n" \
       "Time" "RSS(MB)" "VmSize(MB)" "Peak(MB)" "Data(MB)" "Anon(MB)" "File(MB)" "Lib(MB)" "Threads" "TotalVirt" "Efficiency"
       
printf "${BLUE}%-8s-+-%-7s-+-%-9s-+-%-8s-+-%-8s-+-%-8s-+-%-7s-+-%-6s-+-%-7s-+-%-12s-+-%-12s${NC}\n" \
       "--------" "-------" "---------" "--------" "--------" "--------" "-------" "------" "-------" "------------" "------------"

# Main monitoring loop
while true; do
    # Check if process is still running
    if ! kill -0 "$PID" 2>/dev/null; then
        echo -e "${RED}Process $PID is no longer running. Stopping monitor.${NC}"
        break
    fi
    
    # Get current time
    current_time=$(date '+%H:%M:%S')
    
    # Get memory information using memory analyzer
    memory_info=$(get_memory_info "$PID")
    
    if [ "$memory_info" != "ERROR" ] && [ -n "$memory_info" ]; then
        # Parse the comprehensive memory info
        IFS='|' read -r rss_mb vmsize_mb vmpeak_mb vmdata_mb rssanon_mb rssfile_mb vmlib_mb threads total_virtual total_rss total_dirty process_name efficiency <<< "$memory_info"
        
        # Truncate efficiency for display
        efficiency_short=$(echo "$efficiency" | cut -c1-12)
        total_virtual_short=$(echo "$total_virtual" | cut -c1-12)
        
        # Display the information in a comprehensive table format
        printf "${NC}%-8s | ${GREEN}%-7s${NC} | %-9s | ${YELLOW}%-8s${NC} | %-8s | %-8s | %-7s | %-6s | ${CYAN}%-7s${NC} | %-12s | %-12s\n" \
               "$current_time" "$rss_mb" "$vmsize_mb" "$vmpeak_mb" "$vmdata_mb" "$rssanon_mb" "$rssfile_mb" "$vmlib_mb" "$threads" "$total_virtual_short" "$efficiency_short"
        
        # Every 10 seconds, show detailed summary
        if [ $((count % 10)) -eq 0 ] && [ $count -gt 0 ]; then
            echo
            echo -e "${CYAN}--- Detailed Summary (Sample #$count) ---${NC}"
            echo -e "${BLUE}Process:${NC} $process_name"
            echo -e "${BLUE}Physical Memory (RSS):${NC} ${GREEN}$rss_mb MB${NC} - Actual RAM usage"
            echo -e "${BLUE}Virtual Memory:${NC} $vmsize_mb MB (Peak: $vmpeak_mb MB)"
            echo -e "${BLUE}Memory Breakdown:${NC} Data: $vmdata_mb MB, Anonymous: $rssanon_mb MB, Files: $rssfile_mb MB, Libraries: $vmlib_mb MB"
            echo -e "${BLUE}Thread Count:${NC} ${CYAN}$threads${NC}"
            echo -e "${BLUE}Memory Mapping:${NC} Virtual: $total_virtual, RSS: $total_rss, Dirty: $total_dirty"
            echo -e "${BLUE}Efficiency Assessment:${NC} $efficiency"
            echo -e "${CYAN}--- Continuing monitoring... ---${NC}"
            echo
            
            # Redisplay headers
            printf "${BLUE}%-8s | %-7s | %-9s | %-8s | %-8s | %-8s | %-7s | %-6s | %-7s | %-12s | %-12s${NC}\n" \
                   "Time" "RSS(MB)" "VmSize(MB)" "Peak(MB)" "Data(MB)" "Anon(MB)" "File(MB)" "Lib(MB)" "Threads" "TotalVirt" "Efficiency"
        fi
        
        # Increment counter
        ((count++))
    else
        echo -e "${RED}$current_time - Failed to capture memory data${NC}"
    fi
    
    # Wait for 1 second
    sleep 1
done

# Final cleanup
cleanup
