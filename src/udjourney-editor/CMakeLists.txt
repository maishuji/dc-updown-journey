cmake_minimum_required(VERSION 3.16)
message(STATUS "CMake version: ${CMAKE_VERSION}")
project(udjourney_editor)

set(CMAKE_CXX_STANDARD 20)

# Find raylib (make sure raylib is installed)
find_package(raylib REQUIRED)

# ImGui FetchContent
include(FetchContent)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG docking
)

FetchContent_MakeAvailable(imgui)

set(IMGUI_BACKEND_DIR ${imgui_SOURCE_DIR}/backends)
include_directories(${imgui_SOURCE_DIR} ${IMGUI_BACKEND_DIR})

# ImGuiFileDialog FetchContent
FetchContent_Declare(
  ImGuiFileDialog
  GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git
  GIT_TAG master
)
FetchContent_MakeAvailable(ImGuiFileDialog)
include_directories(${ImGuiFileDialog_SOURCE_DIR})



# nlohmann_json FetchContent
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)

# Google Test is already available from root-level CMakeLists.txt
# No need to fetch it again here since it's inherited

# Create library for shared code between main executable and tests
# Create the editor library
add_library(udjourney_editor_lib
  src/Editor.cpp
  src/EditorScene.cpp
  src/TilePanel.cpp
  src/MonsterPresetManager.cpp
  src/DockingHelper.cpp
  src/strategies/level/BlankLevelStrategy.cpp
  src/ui/NewLevelPopup.cpp
  ${ImGuiFileDialog_SOURCE_DIR}/ImGuiFileDialog.cpp

  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp

  # Official OpenGL3 backend for ImGui rendering
  ${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp
)

target_include_directories(udjourney_editor_lib PUBLIC include)
target_include_directories(udjourney_editor_lib PUBLIC ../udjourney/include)
target_link_libraries(udjourney_editor_lib PUBLIC raylib nlohmann_json::nlohmann_json)

# Set EDITOR_TESTING flag when building for tests
option(ENABLE_EDITOR_TESTING "Enable editor testing features" OFF)
if(ENABLE_EDITOR_TESTING)
    target_compile_definitions(udjourney_editor_lib PUBLIC EDITOR_TESTING)
endif()

target_include_directories(udjourney_editor_lib PUBLIC include)
target_link_libraries(udjourney_editor_lib PUBLIC raylib nlohmann_json::nlohmann_json)

# Create test-enabled version of the library
add_library(udjourney_editor_lib_test STATIC
  src/Editor.cpp
  src/EditorScene.cpp
  src/TilePanel.cpp
  src/MonsterPresetManager.cpp
  src/DockingHelper.cpp
  src/strategies/level/BlankLevelStrategy.cpp
  src/ui/NewLevelPopup.cpp
  ${ImGuiFileDialog_SOURCE_DIR}/ImGuiFileDialog.cpp

  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp

  # Official OpenGL3 backend for ImGui rendering
  ${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp
)

target_include_directories(udjourney_editor_lib_test PUBLIC include PRIVATE src tests)
target_include_directories(udjourney_editor_lib_test PUBLIC ../udjourney/include)
target_link_libraries(udjourney_editor_lib_test PUBLIC raylib nlohmann_json::nlohmann_json)
target_compile_definitions(udjourney_editor_lib_test PRIVATE EDITOR_TESTING)

# Ensure assets symlink exists in both source and build directories
add_custom_command(TARGET udjourney_editor_lib PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/src/udjourney/romdisk
        ${CMAKE_SOURCE_DIR}/src/udjourney-editor/assets
    COMMENT "Creating symlink to romdisk for editor assets in source directory"
)

# Main executable
add_executable(udjourney_editor
  src/main.cpp
)

target_link_libraries(udjourney_editor PRIVATE udjourney_editor_lib)

# Also create symlink in build directory for when running from build directory
add_custom_command(TARGET udjourney_editor PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/src/udjourney/romdisk
        ${CMAKE_BINARY_DIR}/src/udjourney-editor/assets
    COMMENT "Creating symlink to romdisk for editor assets in build directory"
)

if(UNIX AND NOT APPLE)
  target_link_libraries(udjourney_editor PRIVATE pthread m dl)
endif()

enable_testing()

add_executable(tilepanel_tests
  tests/test_tilepanel.cpp
  src/TilePanel.cpp
  src/MonsterPresetManager.cpp
  src/strategies/level/BlankLevelStrategy.cpp
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(tilepanel_tests PRIVATE include src ${imgui_SOURCE_DIR})
target_include_directories(tilepanel_tests PRIVATE ../udjourney/include)
target_link_libraries(tilepanel_tests PRIVATE gtest gtest_main nlohmann_json::nlohmann_json)

add_test(NAME TilePanelTests COMMAND tilepanel_tests)

# Add the tests subdirectory
add_subdirectory(tests)
