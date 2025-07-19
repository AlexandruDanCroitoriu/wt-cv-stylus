cmake_minimum_required(VERSION 3.13...3.22)
include(FetchContent)
include(ExternalProject)

project(app)

# Set number of parallel build jobs to number of available processors
include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
  set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread")
set(RLIB --docroot ../../ -c ../../wt_config.xml --http-address 0.0.0.0 --http-port 9020)
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/)

include_directories(${SOURCE_DIR})


FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git GIT_TAG da40186618909b1a7363d4e4495aa899c6e0eb75) 
FetchContent_MakeAvailable(cpr)

# Whisper.cpp integration
FetchContent_Declare(
    whisper
    GIT_REPOSITORY https://github.com/ggerganov/whisper.cpp.git
    GIT_TAG v1.5.4
)
FetchContent_MakeAvailable(whisper)

# ./app --docroot ../ –config ./wt_config.xml --http-address 0.0.0.0 --http-port 9020

# Main application sources (without WhisperAi.cpp since we're using external service)
set(MAIN_APP_SOURCES
    ${SOURCE_DIR}/main.cpp
    
    ${SOURCE_DIR}/000-Server/Server.cpp
    
    ${SOURCE_DIR}/001-App/App.cpp
    
    ${SOURCE_DIR}/002-Theme/Theme.cpp
    ${SOURCE_DIR}/002-Theme/ThemeSwitcher.cpp
    ${SOURCE_DIR}/002-Theme/DarkModeToggle.cpp
    
    ${SOURCE_DIR}/003-Components/ComponentsDisplay.cpp
    ${SOURCE_DIR}/003-Components/Button.cpp
    ${SOURCE_DIR}/003-Components/MonacoEditor.cpp
    ${SOURCE_DIR}/003-Components/VoiceRecorder.cpp
    ${SOURCE_DIR}/999-ExternalServices/WhisperCliService.cpp
    ${SOURCE_DIR}/003-Components/BigWorkWidget.cpp
    
    ${SOURCE_DIR}/004-Dbo/Session.cpp
    ${SOURCE_DIR}/004-Dbo/Tables/User.cpp
    ${SOURCE_DIR}/004-Dbo/Tables/Permission.cpp

    ${SOURCE_DIR}/005-Auth/AuthWidget.cpp
    ${SOURCE_DIR}/005-Auth/RegistrationView.cpp
    ${SOURCE_DIR}/005-Auth/UserDetailsModel.cpp

    ${SOURCE_DIR}/006-Navigation/Navigation.cpp
    
    ${SOURCE_DIR}/007-UserSettings/UserSettings.cpp

    ${SOURCE_DIR}/008-AboutMe/AboutMe.cpp

    ${SOURCE_DIR}/010-StarWarsApi/StarWarsApi.cpp
    
    ${SOURCE_DIR}/101-Examples/BroadcastExample.cpp
    ${SOURCE_DIR}/101-Examples/CheckboxBroadcastExample.cpp
)

# Whisper service sources
set(WHISPER_SERVICE_SOURCES
    ${SOURCE_DIR}/999-ExternalServices/whisper_service_main.cpp
)

# Build main application
add_executable(${PROJECT_NAME} ${MAIN_APP_SOURCES})

target_link_libraries(${PROJECT_NAME} 
    wthttp
    wt
    wtdbo
    wtdbosqlite3
    # wtdbopostgres
    boost_regex
    cpr::cpr
    nlohmann_json::nlohmann_json
)

# Build whisper service executable
add_executable(whisper_service ${WHISPER_SERVICE_SOURCES})

target_link_libraries(whisper_service 
    whisper
    nlohmann_json::nlohmann_json
)

# Set runtime library path so the executable can find the shared libraries
set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH "${CMAKE_CURRENT_BINARY_DIR}/_deps/cpr-build/cpr"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

# Set runtime library path for whisper service
set_target_properties(whisper_service PROPERTIES
    INSTALL_RPATH "${CMAKE_CURRENT_BINARY_DIR}/_deps/whisper-build"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

# Add pthread for threading support (if not already present)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_link_libraries(whisper_service Threads::Threads)

# Platform-specific libraries
if(APPLE)
    target_link_libraries(whisper_service "-framework Accelerate")
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(whisper_service m dl)
endif()

add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -E env LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}/_deps/cpr-build/cpr:$ENV{LD_LIBRARY_PATH} $<TARGET_FILE:${PROJECT_NAME}> ${RLIB}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running ${PROJECT_NAME} with proper library paths"
)
