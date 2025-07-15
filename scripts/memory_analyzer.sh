#!/bin/bash

# Memory Analyzer Script
# Usage: ./memory_analyzer.sh <PID>
# Provides detailed memory usage analysis for a given process

# Colors for output formatting
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

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

echo -e "${CYAN}================================================================${NC}"
echo -e "${CYAN}           Memory Usage Analysis for Process $PID${NC}"
echo -e "${CYAN}================================================================${NC}"
echo

# Get basic process information
echo -e "${BLUE}### Process Information ###${NC}"
ps_info=$(ps -p $PID -o pid,ppid,rss,vsz,pmem,comm --no-headers 2>/dev/null)
if [ -n "$ps_info" ]; then
    read pid ppid rss vsz pmem comm <<< "$ps_info"
    cmd_line=$(ps -p $PID -o cmd --no-headers 2>/dev/null)
    
    echo -e "PID: ${GREEN}$pid${NC}"
    echo -e "PPID: $ppid"
    echo -e "Command: ${YELLOW}$comm${NC}"
    echo -e "Full Command: $cmd_line"
    echo -e "RSS (Physical Memory): ${GREEN}$(kb_to_human $rss)${NC} ($(format_number $rss) KB)"
    echo -e "VSZ (Virtual Memory): $(kb_to_human $vsz) ($(format_number $vsz) KB)"
    echo -e "Memory Percentage: ${GREEN}$pmem%${NC}"
else
    echo -e "${RED}Could not retrieve basic process information${NC}"
fi

echo

# Get detailed memory information from /proc/PID/status
echo -e "${BLUE}### Detailed Memory Breakdown ###${NC}"
if [ -r "/proc/$PID/status" ]; then
    # Extract memory values
    vmpeak=$(grep "^VmPeak:" /proc/$PID/status | awk '{print $2}')
    vmsize=$(grep "^VmSize:" /proc/$PID/status | awk '{print $2}')
    vmhwm=$(grep "^VmHWM:" /proc/$PID/status | awk '{print $2}')
    vmrss=$(grep "^VmRSS:" /proc/$PID/status | awk '{print $2}')
    rssanon=$(grep "^RssAnon:" /proc/$PID/status | awk '{print $2}')
    rssfile=$(grep "^RssFile:" /proc/$PID/status | awk '{print $2}')
    rssshmem=$(grep "^RssShmem:" /proc/$PID/status | awk '{print $2}')
    vmdata=$(grep "^VmData:" /proc/$PID/status | awk '{print $2}')
    vmstk=$(grep "^VmStk:" /proc/$PID/status | awk '{print $2}')
    vmexe=$(grep "^VmExe:" /proc/$PID/status | awk '{print $2}')
    vmlib=$(grep "^VmLib:" /proc/$PID/status | awk '{print $2}')
    vmpte=$(grep "^VmPTE:" /proc/$PID/status | awk '{print $2}')
    vmswap=$(grep "^VmSwap:" /proc/$PID/status | awk '{print $2}')
    threads=$(grep "^Threads:" /proc/$PID/status | awk '{print $2}')
    
    echo -e "${GREEN}Current Memory Usage:${NC}"
    echo -e "• RSS (Resident Set Size): ${GREEN}$(kb_to_human $vmrss)${NC} (~$((vmrss/1024)) MB) - ${GREEN}Physical memory currently used${NC}"
    echo -e "• VmSize (Virtual Memory Size): $(kb_to_human $vmsize) (~$((vmsize/1024)) MB) - Total virtual memory"
    echo -e "• VmPeak: $(kb_to_human $vmpeak) (~$((vmpeak/1024)) MB) - Peak virtual memory usage"
    echo
    
    echo -e "${YELLOW}Memory Breakdown:${NC}"
    echo -e "• VmData: $(kb_to_human $vmdata) (~$((vmdata/1024)) MB) - Data/heap memory"
    echo -e "• RssAnon: $(kb_to_human $rssanon) (~$((rssanon/1024)) MB) - Anonymous pages (heap, stack)"
    echo -e "• RssFile: $(kb_to_human $rssfile) (~$((rssfile/1024)) MB) - File-backed pages (libraries, executables)"
    echo -e "• RssShmem: $(kb_to_human $rssshmem) - Shared memory"
    echo -e "• VmLib: $(kb_to_human $vmlib) (~$((vmlib/1024)) MB) - Shared libraries"
    echo -e "• VmExe: $(kb_to_human $vmexe) (~$((vmexe/1024)) MB) - Executable code"
    echo -e "• VmStk: $(kb_to_human $vmstk) - Stack memory"
    echo -e "• VmPTE: $(kb_to_human $vmpte) - Page table entries"
    echo -e "• VmSwap: $(kb_to_human $vmswap) - Swapped memory"
    echo
    
    echo -e "${CYAN}Process Details:${NC}"
    echo -e "• Threads: ${GREEN}$threads${NC} threads running"
    
    # Calculate peak usage
    if [ -n "$vmhwm" ] && [ "$vmhwm" != "0" ]; then
        echo -e "• VmHWM (Peak RSS): $(kb_to_human $vmhwm) (~$((vmhwm/1024)) MB) - Peak physical memory usage"
    fi
    
    echo
    echo -e "${BLUE}### Structured Data Table (for monitoring) ###${NC}"
    echo "MEMORY_DATA_START"
    printf "%-12s | %-8s | %-10s | %-10s | %-11s | %-8s\n" "Metric" "Value_KB" "Value_MB" "Human" "Description" "Type"
    echo "-------------|----------|------------|------------|-------------|----------"
    printf "%-12s | %-8s | %-10s | %-10s | %-11s | %-8s\n" "RSS" "$vmrss" "$((vmrss/1024))" "$(kb_to_human $vmrss)" "Physical_Memory" "CURRENT"
    printf "%-12s | %-8s | %-10s | %-10s | %-11s | %-8s\n" "VmSize" "$vmsize" "$((vmsize/1024))" "$(kb_to_human $vmsize)" "Virtual_Memory" "CURRENT"
    printf "%-12s | %-8s | %-10s | %-10s | %-11s | %-8s\n" "VmPeak" "$vmpeak" "$((vmpeak/1024))" "$(kb_to_human $vmpeak)" "Peak_Virtual" "PEAK"
    printf "%-12s | %-8s | %-10s | %-10s | %-11s | %-8s\n" "VmData" "$vmdata" "$((vmdata/1024))" "$(kb_to_human $vmdata)" "Data_Heap" "CURRENT"
    printf "%-12s | %-8s | %-10s | %-10s | %-11s | %-8s\n" "RssAnon" "$rssanon" "$((rssanon/1024))" "$(kb_to_human $rssanon)" "Anonymous" "CURRENT"
    printf "%-12s | %-8s | %-10s | %-10s | %-11s | %-8s\n" "RssFile" "$rssfile" "$((rssfile/1024))" "$(kb_to_human $rssfile)" "File_Backed" "CURRENT"
    printf "%-12s | %-8s | %-10s | %-10s | %-11s | %-8s\n" "VmLib" "$vmlib" "$((vmlib/1024))" "$(kb_to_human $vmlib)" "Libraries" "CURRENT"
    printf "%-12s | %-8s | %-10s | %-10s | %-11s | %-8s\n" "Threads" "$threads" "$threads" "$threads" "Thread_Count" "COUNT"
    if [ -n "$vmhwm" ] && [ "$vmhwm" != "0" ]; then
        printf "%-12s | %-8s | %-10s | %-10s | %-11s | %-8s\n" "VmHWM" "$vmhwm" "$((vmhwm/1024))" "$(kb_to_human $vmhwm)" "Peak_Physical" "PEAK"
    fi
    echo "MEMORY_DATA_END"
    
else
    echo -e "${RED}Could not read /proc/$PID/status${NC}"
fi

echo

# Get memory mapping summary
echo -e "${BLUE}### Memory Mapping Summary ###${NC}"
if command -v pmap >/dev/null 2>&1; then
    pmap_output=$(pmap -x $PID 2>/dev/null | tail -1)
    if [ -n "$pmap_output" ]; then
        # Check if this is the total line (contains "total")
        if echo "$pmap_output" | grep -q "total"; then
            # Extract numeric values from the total line
            total_virtual=$(echo "$pmap_output" | awk '{print $3}')
            total_rss=$(echo "$pmap_output" | awk '{print $4}')
            total_dirty=$(echo "$pmap_output" | awk '{print $5}')
            
            # Validate that we got numeric values
            if [[ "$total_virtual" =~ ^[0-9]+$ ]] && [[ "$total_rss" =~ ^[0-9]+$ ]] && [[ "$total_dirty" =~ ^[0-9]+$ ]]; then
                echo -e "Total Virtual: $(kb_to_human $total_virtual)"
                echo -e "Total RSS: ${GREEN}$(kb_to_human $total_rss)${NC}"
                echo -e "Total Dirty: $(kb_to_human $total_dirty)"
            else
                echo -e "${YELLOW}Could not parse memory mapping values${NC}"
                echo -e "Raw pmap output: $pmap_output"
            fi
        else
            echo -e "${YELLOW}Unexpected pmap output format${NC}"
            echo -e "Raw pmap output: $pmap_output"
        fi
    else
        echo -e "${YELLOW}Could not retrieve memory mapping information${NC}"
    fi
else
    echo -e "${YELLOW}pmap command not available${NC}"
fi

echo

# Get process name and additional info
echo -e "${BLUE}### Key Observations ###${NC}"
process_name=$(ps -p $PID -o comm= 2>/dev/null)
if [ -n "$vmrss" ] && [ -n "$vmsize" ]; then
    physical_mb=$((vmrss / 1024))
    virtual_gb=$((vmsize / 1048576))
    
    echo -e "1. ${GREEN}Physical Memory Usage${NC}: ~${GREEN}${physical_mb} MB${NC} - This is the actual RAM being used"
    echo -e "2. ${YELLOW}Virtual Memory${NC}: ~${virtual_gb} GB - This includes memory-mapped files and allocated but not physically used memory"
    
    if [ -n "$threads" ]; then
        echo -e "3. ${CYAN}Threads${NC}: $threads threads running"
    fi
    
    if [ -n "$vmpeak" ] && [ "$vmpeak" != "$vmsize" ]; then
        peak_gb=$((vmpeak / 1048576))
        echo -e "4. ${BLUE}Peak Usage${NC}: The process peaked at ~${peak_gb} GB virtual memory"
    fi
    
    echo
    echo -e "${GREEN}### Memory Efficiency Notes ###${NC}"
    if [ -n "$process_name" ]; then
        echo -e "For a ${YELLOW}$process_name${NC} process:"
    fi
    echo -e "The ${GREEN}${physical_mb} MB physical memory usage${NC} represents the actual RAM consumption."
    echo -e "Virtual memory includes memory-mapped files, shared libraries, and allocated but unused memory."
    
    # Memory usage assessment
    if [ $physical_mb -lt 100 ]; then
        echo -e "${GREEN}✓ Low memory usage - very efficient${NC}"
    elif [ $physical_mb -lt 500 ]; then
        echo -e "${YELLOW}✓ Moderate memory usage - reasonable for most applications${NC}"
    elif [ $physical_mb -lt 1000 ]; then
        echo -e "${YELLOW}⚠ High memory usage - monitor for memory leaks${NC}"
    else
        echo -e "${RED}⚠ Very high memory usage - investigate for optimization opportunities${NC}"
    fi
fi

echo
echo -e "${CYAN}================================================================${NC}"
echo -e "${CYAN}                    Analysis Complete${NC}"
echo -e "${CYAN}================================================================${NC}"
