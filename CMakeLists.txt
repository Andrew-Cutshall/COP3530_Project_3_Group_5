cmake_minimum_required(VERSION 3.15)
project(COP3530_Project_3_Group_5 LANGUAGES CXX)
include_directories(include)

# Define Source Files
set(SOURCE_FILES
    "src/window.cpp"
    "src/bfh.cpp"
    "src/dijkstra.cpp"
    "src/graph.cpp"
    "src/dataCollection.cpp"
)

#Set Output Directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include(FetchContent)
if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
endif ()

#For Graphics
find_package(SFML QUIET)
if (NOT SFML_FOUND)
    message(STATUS "SFML not found, fetching from GitHub...")
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
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

#For Database Storage SQLite
set(SQLITECPP_RUN_CPPLINT OFF) #Disable to avoid issues during runtime
FetchContent_Declare(
    sqlite_cpp
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
    GIT_TAG 3.3.1
)
FetchContent_MakeAvailable(sqlite_cpp)

# --- For Data Collection (cURL) ---
find_package(CURL REQUIRED)

#Build Executable
add_executable(main "src/main.cpp" ${SOURCE_FILES}) 
target_compile_definitions(main PRIVATE GIT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(main PRIVATE cxx_std_23)
target_link_libraries(main PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
    SQLiteCpp
    SFML::Graphics
    SFML::Audio
    SFML::Window
    SFML::System
)