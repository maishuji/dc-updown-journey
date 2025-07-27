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

add_executable(udjourney_editor
  src/main.cpp
  src/Editor.cpp
  src/TilePanel.cpp

  ${ImGuiFileDialog_SOURCE_DIR}/ImGuiFileDialog.cpp

  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp

  # Official OpenGL3 backend for ImGui rendering
  ${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp
)

target_include_directories(udjourney_editor PUBLIC include)

target_link_libraries(udjourney_editor PRIVATE raylib nlohmann_json::nlohmann_json)

if(UNIX AND NOT APPLE)
  target_link_libraries(udjourney_editor PRIVATE pthread m dl)
endif()
