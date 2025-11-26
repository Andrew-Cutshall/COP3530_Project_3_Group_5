cmake_minimum_required(VERSION 3.15)
project(COP3530_Project_3_Group_5 LANGUAGES CXX)
include_directories(include)

# Define Source Files
set(SOURCE_FILES
    "src/main.cpp"
    "src/window.h"
    "src/window.cpp"
    "src/bfh.h"
    "src/bfh.cpp"
    "src/dijkstra.h"
    "src/dijkstra.cpp"
    "src/apiKey.h"
    "src/otherFunctions.h"
    "src/dataCollection.h"
    "src/graph.h"
    "src/graph.cpp"
)

#Set Output Directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include(FetchContent)
if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
endif ()

#Find and Fetch Dependencies
find_package(SFML QUIET)
if (NOT SFML_FOUND)
    message(STATUS "SFML not found, fetching from GitHub...")
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM
    )
    FetchContent_MakeAvailable(SFML)
endif()
#For Data Collection JSON Parsing
message(STATUS "Fetching nlohmann/json from GitHub...")
FetchContent_Declare(
    json_library
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json_library)
#For Data Collection
find_package(CURL REQUIRED)

#Build Executable
add_executable(main ${SOURCE_FILES})
target_compile_features(main PRIVATE cxx_std_23)
target_link_libraries(main PRIVATE
    SFML::Graphics
    SFML::Audio
    SFML::Window
    SFML::System
    nlohmann_json::nlohmann_json
    CURL::libcurl
)

# Saved for later reference, copies regular files to output directory, can be modified without messing up actually files.
# This copies 'settings.cfg' to the output directory
# configure_file(
#     "${CMAKE_CURRENT_SOURCE_DIR}/resource/settings.cfg"
#     "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/settings.cfg"
#     COPYONLY
# )