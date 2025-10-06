cmake_minimum_required(VERSION 3.11.0)

# Create test executable
add_executable(updown_journey_tests
    scene/test_scene.cpp
    scene/test_scene_serialization.cpp
    scene/test_coordinate_conversion.cpp
    test_main.cpp
)

# Link libraries
target_link_libraries(updown_journey_tests
    PUBLIC
        gtest
        gtest_main
        nlohmann_json::nlohmann_json
        raylib
        GL
)

# Include directories
target_include_directories(updown_journey_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/udjourney/include
)

# Add source files that tests depend on
target_sources(updown_journey_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/udjourney/src/scene/Scene.cpp
)

# Set C++ standard
set_target_properties(updown_journey_tests
    PROPERTIES
        CMAKE_CXX_STANDARD 20
        CXX_EXTENSIONS OFF
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(updown_journey_tests)

# Create test data directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_data)

# Copy test data files
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test_data/valid_scene.json
    ${CMAKE_CURRENT_BINARY_DIR}/test_data/valid_scene.json
    COPYONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test_data/invalid_scene.json
    ${CMAKE_CURRENT_BINARY_DIR}/test_data/invalid_scene.json
    COPYONLY
)

# Add custom test target for convenience
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS updown_journey_tests
    COMMENT "Running unit tests"
)