
message(STATUS "(3.6)CMAKE_SYSTEM_INCLUDE_PATH is set to: ${CMAKE_SYSTEM_INCLUDE_PATH}")

# Set it as a cache variable to preserve it
set(CMAKE_SYSTEM_INCLUDE_PATH_CACHE "${CMAKE_SYSTEM_INCLUDE_PATH}" CACHE STRING "System include paths for the toolchain")

cmake_minimum_required( VERSION 3.11.0)
# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()
message(STATUS "(3.7)CMAKE_SYSTEM_INCLUDE_PATH is set to: ${CMAKE_SYSTEM_INCLUDE_PATH}")

# Retrieve the include directories
get_property(INCLUDE_DIRS GLOBAL PROPERTY INCLUDE_DIRECTORIES)

# Print out the directories
message(STATUS "(1)Include directories in include_directories: ${INCLUDE_DIRS}")

project(dc-template CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)


include("toolchains/dreamcast.cmake")

include_directories(SYSTEM /opt/toolchains/dc/sh-elf/sh-elf/include)
include_directories(SYSTEM /opt/toolchains/dc/kos/include)


message(STATUS "(3.71)CMAKE_SYSTEM_INCLUDE_PATH is set to: ${CMAKE_SYSTEM_INCLUDE_PATH2}")

message(STATUS "(3.8)CMAKE_SYSTEM_INCLUDE_PATH is set to: ${CMAKE_SYSTEM_INCLUDE_PATH}")
set(CMAKE_SYSTEM_INCLUDE_PATH "${CMAKE_SYSTEM_INCLUDE_PATH} ${CMAKE_SYSTEM_INCLUDE_PATH_CACHE}")


# Retrieve the include directories
get_property(INCLUDE_DIRS GLOBAL PROPERTY INCLUDE_DIRECTORIES)

# Print out the directories
message(STATUS "(2)Include directories in include_directories: ${INCLUDE_DIRS}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "(5)CMAKE_SYSTEM_INCLUDE_PATH = ${CMAKE_SYSTEM_INCLUDE_PATH}")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(NoInSourceBuilds)
include(GenerateCdiImage)

# Optional: include the iwyu cmake module, useful for checking includes
find_program(IWYU_PATH NAMES include-what-you-use iwyu)
# ------------------------------------------ #
# Global cmake variables
# ------------------------------------------ #
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True) # Enfore the use of C++20
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------ #
# Subdirectories
# ------------------------------------------ #
add_subdirectory(src)