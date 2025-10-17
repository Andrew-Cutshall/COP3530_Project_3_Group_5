# CMakeList.txt : CMake project for COP3530_Project_3_Group_5, include source and define
cmake_minimum_required (VERSION 3.15)
project(CMakeSFMLProject LANGUAGES CXX)
include_directories(include)

# Add Sources here when creating new cpp file.
set(SOURCE_FILES 
    src/main.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#Dev mode setting for MSVC, allows for faster work envionment when debugging, to be removed for release builds.
include(FetchContent)
if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
endif ()

# Attempt to find SFML, if not found fetch it.
find_package(SFML QUIET)
if (NOT SFML_FOUND)
    message(STATUS "SFML not found, fetching from GitHub...")
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2 #Latest stable version of SFML
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM
    )
    FetchContent_MakeAvailable(SFML)
endif()

add_executable(main ${SOURCE_FILES})
target_compile_features(main PRIVATE cxx_std_23)
target_link_libraries(main PRIVATE
    SFML::Graphics
    SFML::Audio
    SFML::Window
    SFML::System
)

#Saved for later reference, copies regular files to output directory, can be modified without messing up actually files.
# This copies 'settings.cfg' to the output directory
#configure_file(
#    "${CMAKE_CURRENT_SOURCE_DIR}/resource/settings.cfg"
#    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/settings.cfg"
#    COPYONLY
#)