#!/bin/bash

# Script overview and usage guide
# Usage: ./scripts/README.sh

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

echo -e "${BOLD}${BLUE}=== Wt CV Stylus Build and Run Scripts ===${NC}"
echo ""
echo -e "${BOLD}${GREEN}MAIN SCRIPTS:${NC}"
echo -e "  ${CYAN}./scripts/build.sh${NC} [--debug|-d|--release|-r] [clean]"
echo -e "    ${YELLOW}-${NC} Unified build script for both debug and release"
echo -e "    ${YELLOW}-${NC} Defaults to debug if no type specified"
echo -e "    ${YELLOW}-${NC} Use 'clean' for clean builds"
echo ""
echo -e "  ${CYAN}./scripts/run.sh${NC} [--debug|-d|--release|-r]"
echo -e "    ${YELLOW}-${NC} Unified run script for both debug and release"
echo -e "    ${YELLOW}-${NC} Automatically builds if needed"
echo -e "    ${YELLOW}-${NC} Automatically kills existing instances on port 9020"
echo -e "    ${YELLOW}-${NC} Defaults to debug if no type specified"
echo ""0.


.0
echo -e "${BOLD}${GREEN}OTHER SCRIPTS:${NC}"
echo -e "  ${CYAN}./scripts/clone_libraries.sh${NC}             - Clone external libraries"
echo -e "  ${CYAN}./scripts/memory_analyzer.sh${NC}             - Single memory analysis (used by monitor)"
echo -e "  ${CYAN}./scripts/memory_monitor.sh${NC}              - Continuous monitoring (auto-detects port 9020 app)"
echo ""
echo -e "${BOLD}${YELLOW}EXAMPLES:${NC}"
echo -e "  ${GREEN}./scripts/run.sh${NC}                      # Build (if needed) and run debug"
echo -e "  ${GREEN}./scripts/run.sh --release${NC}            # Build (if needed) and run release"
echo -e "  ${GREEN}./scripts/build.sh --debug clean${NC}      # Clean debug build"
echo -e "  ${GREEN}./scripts/build.sh -r && ./scripts/run.sh -r${NC}  # Build release then run"
echo ""
echo -e "${BOLD}${BLUE}NOTES:${NC}"
echo -e "  ${YELLOW}•${NC} All scripts log output to ${CYAN}./scripts/output/${NC} with descriptive filenames"
echo -e "  ${YELLOW}•${NC} Application runs on ${CYAN}http://localhost:9020${NC} with settings from CMakeLists.txt"
echo -e "  ${YELLOW}•${NC} Use ${CYAN}--help${NC} or ${CYAN}-h${NC} flag on any script for detailed usage"
