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

FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(catch2)

# Create library for shared code between main executable and tests
# Create the editor library
add_library(udjourney_editor_lib
  src/Editor.cpp
  src/EditorScene.cpp
  src/TilePanel.cpp
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

target_include_directories(udjourney_editor_lib_test PUBLIC include)
target_link_libraries(udjourney_editor_lib_test PUBLIC raylib nlohmann_json::nlohmann_json)
target_compile_definitions(udjourney_editor_lib_test PRIVATE EDITOR_TESTING)

# Main executable
add_executable(udjourney_editor
  src/main.cpp
)

target_link_libraries(udjourney_editor PRIVATE udjourney_editor_lib)

if(UNIX AND NOT APPLE)
  target_link_libraries(udjourney_editor PRIVATE pthread m dl)
endif()

enable_testing()

add_executable(tilepanel_tests
  tests/test_tilepanel.cpp
  src/TilePanel.cpp
  src/strategies/level/BlankLevelStrategy.cpp
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(tilepanel_tests PRIVATE include src ${imgui_SOURCE_DIR})
target_link_libraries(tilepanel_tests PRIVATE Catch2::Catch2WithMain)

add_test(NAME TilePanelTests COMMAND tilepanel_tests)

# Add the tests subdirectory
add_subdirectory(tests)