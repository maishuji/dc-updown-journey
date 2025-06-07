cmake_minimum_required(VERSION 3.16)
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

add_executable(udjourney_editor
  src/main.cpp

  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp

  # Official OpenGL3 backend for ImGui rendering
  ${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp
)

target_link_libraries(udjourney_editor PRIVATE raylib)

if(UNIX AND NOT APPLE)
  target_link_libraries(udjourney_editor PRIVATE pthread m dl)
endif()
