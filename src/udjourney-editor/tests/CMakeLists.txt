# Test configuration for udjourney-editor
# Note: Catch2 is already fetched in parent CMakeLists.txt

# Enable editor testing features
set(ENABLE_EDITOR_TESTING ON PARENT_SCOPE)
set(ENABLE_EDITOR_TESTING ON)

# Create test executable
add_executable(udjourney_editor_tests
    test_json_operations.cpp
    test_tilemap_json.cpp
)

# Define EDITOR_TESTING for the test executable
target_compile_definitions(udjourney_editor_tests PRIVATE EDITOR_TESTING)

# Link against the test-enabled editor library and Catch2
target_link_libraries(udjourney_editor_tests 
    PRIVATE 
    udjourney_editor_lib_test
    Catch2::Catch2WithMain
    nlohmann_json::nlohmann_json
)

# Include directories
target_include_directories(udjourney_editor_tests 
    PRIVATE 
    ${CMAKE_SOURCE_DIR}/src/udjourney-editor/include
)

# Set C++ standard
target_compile_features(udjourney_editor_tests PRIVATE cxx_std_20)

# Add test to CTest
add_test(NAME JSONOperationsTests COMMAND udjourney_editor_tests)

# Custom test target for running tests
add_custom_target(run_json_tests
    COMMAND udjourney_editor_tests
    DEPENDS udjourney_editor_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running JSON import/export unit tests"
)