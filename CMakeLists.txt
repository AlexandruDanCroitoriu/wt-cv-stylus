cmake_minimum_required(VERSION 3.13...3.22)
include(FetchContent)

project(app)

# Set number of parallel build jobs to number of available processors
include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
  set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
endif()

set(CMAKE_CXX_STANDARD 17)
set(XMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread")
set(RLIB --docroot ../../ -c ../../wt_config.xml --http-address 0.0.0.0 --http-port 9020)
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/)

include_directories(${SOURCE_DIR})


FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git GIT_TAG da40186618909b1a7363d4e4495aa899c6e0eb75) 
FetchContent_MakeAvailable(cpr)

# ./app --docroot ../ –config ./wt_config.xml --http-address 0.0.0.0 --http-port 9020

set(SOURCES
    ${SOURCE_DIR}/main.cpp
    
    ${SOURCE_DIR}/000-Server/Server.cpp
    
    ${SOURCE_DIR}/001-App/App.cpp
    
    ${SOURCE_DIR}/002-Theme/Theme.cpp
    ${SOURCE_DIR}/002-Theme/ThemeSwitcher.cpp
    ${SOURCE_DIR}/002-Theme/DarkModeToggle.cpp
    
    ${SOURCE_DIR}/003-Components/ComponentsDisplay.cpp
    ${SOURCE_DIR}/003-Components/Button.cpp
    ${SOURCE_DIR}/003-Components/MonacoEditor.cpp
    
    ${SOURCE_DIR}/004-Dbo/Session.cpp
    ${SOURCE_DIR}/004-Dbo/Tables/User.cpp
    ${SOURCE_DIR}/004-Dbo/Tables/Permission.cpp

    ${SOURCE_DIR}/005-Auth/AuthWidget.cpp
    ${SOURCE_DIR}/005-Auth/RegistrationView.cpp
    ${SOURCE_DIR}/005-Auth/UserDetailsModel.cpp

    ${SOURCE_DIR}/006-Navigation/Navigation.cpp
    
    ${SOURCE_DIR}/007-UserSettings/UserSettings.cpp

    ${SOURCE_DIR}/008-AboutMe/AboutMe.cpp

    ${SOURCE_DIR}/101-StarWarsApi/StarWarsApi.cpp

)

add_executable(${PROJECT_NAME} ${SOURCES})

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

add_custom_target(run
    COMMAND ${PROJECT_NAME} ${RLIB}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)
# add_custom_target(run
#     COMMAND ${PROJECT_NAME} ${RLIB} > ./logs.txt 2>&1
#     DEPENDS ${PROJECT_NAME}
#     WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
# )
