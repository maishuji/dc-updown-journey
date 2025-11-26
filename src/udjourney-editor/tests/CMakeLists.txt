# Test configuration for udjourney-editor
# Note: Google Test is already available from root-level CMakeLists.txt

# Enable editor testing features
set(ENABLE_EDITOR_TESTING ON PARENT_SCOPE)
set(ENABLE_EDITOR_TESTING ON)

# Create test executable
add_executable(udjourney_editor_tests
    test_json_operations.cpp
    test_tilemap_json_simple.cpp
    test_asset_access.cpp
    test_monster_preset_manager.cpp
    test_background.cpp
)

# Define EDITOR_TESTING for the test executable
target_compile_definitions(udjourney_editor_tests PRIVATE EDITOR_TESTING)

# Link against the test-enabled editor library and GTest
target_link_libraries(udjourney_editor_tests 
    PRIVATE 
    udjourney_editor_lib_test
    gtest
    gtest_main
    nlohmann_json::nlohmann_json
)

# Include directories
target_include_directories(udjourney_editor_tests 
    PRIVATE 
    ${CMAKE_SOURCE_DIR}/src/udjourney-editor/include
)

# Set C++ standard
target_compile_features(udjourney_editor_tests PRIVATE cxx_std_20)

# Ensure assets symlink exists for tests
add_custom_command(TARGET udjourney_editor_tests PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/src/udjourney/romdisk
        ${CMAKE_CURRENT_BINARY_DIR}/assets
    COMMENT "Creating assets symlink for tests"
)

# Add test to CTest using GTest discovery
include(GoogleTest)
gtest_discover_tests(udjourney_editor_tests)

# Custom test target for running tests
add_custom_target(run_editor_tests
    COMMAND udjourney_editor_tests
    DEPENDS udjourney_editor_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running editor unit tests"
)
