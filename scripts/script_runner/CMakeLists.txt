cmake_minimum_required(VERSION 3.16)
project(ScriptRunner VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES REQUIRED ncurses)

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Wpedantic -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Source files
set(SOURCES
    src/main.cpp
    src/Logger.cpp
    src/Exceptions.cpp
    src/ProcessManager.cpp
    src/ScriptRunner.cpp
    src/UIManager.cpp
)

# Create executable
add_executable(script_runner ${SOURCES})

# Link libraries
target_link_libraries(script_runner 
    ${NCURSES_LIBRARIES}
    pthread
)

# Include directories
target_include_directories(script_runner PRIVATE
    include/
    ${NCURSES_INCLUDE_DIRS}
)

# Installation
install(TARGETS script_runner DESTINATION bin)

# Optional: Create custom target for easy execution
add_custom_target(run
    COMMAND $<TARGET_FILE:script_runner>
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running script runner application"
    DEPENDS script_runner
)

# Development build configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(script_runner PRIVATE DEBUG_BUILD)
endif()

# Set output directories
set_target_properties(script_runner PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
