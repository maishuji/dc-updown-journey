cmake_minimum_required(VERSION 3.11.0)
project(udj-core)

# Core library with common utilities
add_library(udj-core
    src/CoreUtils.cpp
    src/Logger.cpp
    src/FileSystemUtils.cpp
    src/MathUtils.cpp
)

target_include_directories(udj-core PUBLIC include)
target_compile_features(udj-core PUBLIC cxx_std_20)

# Platform-specific defines
if(PLATFORM_DREAMCAST)
    target_compile_definitions(udj-core PUBLIC ASSETS_BASE_PATH="romdisk/")
else()
    target_compile_definitions(udj-core PUBLIC ASSETS_BASE_PATH="assets/")
endif()

# For JSON support (optional dependency)
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    target_link_libraries(udj-core PUBLIC nlohmann_json::nlohmann_json)
    target_compile_definitions(udj-core PUBLIC UDJ_CORE_HAS_JSON)
endif()
