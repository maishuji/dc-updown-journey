cmake_minimum_required( VERSION 3.11.0)
# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

project(dc-template CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable testing
enable_testing()
include(FetchContent)

# Google Test setup (only for non-Dreamcast builds)
if (NOT PLATFORM_DREAMCAST)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

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

# Add tests (only for non-Dreamcast builds)
if (NOT PLATFORM_DREAMCAST)
    add_subdirectory(tests)
endif()
