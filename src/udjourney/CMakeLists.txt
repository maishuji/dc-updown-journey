cmake_minimum_required( VERSION 3.11.0)
# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

set(TARGET_NAME "upown-journey")
add_executable(${TARGET_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Player.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Bonus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/BonusManager.cpp
)
set_target_properties(${TARGET_NAME} PROPERTIES
                      CMAKE_CXX_STANDARD 20
                      CXX_EXTENSIONS OFF
                      CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH}
                      )

target_include_directories(${TARGET_NAME}
    PUBLIC
        include)

# GL must be linked after raylib
target_link_libraries(${TARGET_NAME} 
    PUBLIC 
        dcplib raylib GL)

# ---- ROMDISK generation ----
set(ROMDISK_DIR "romdisk")
kos_add_romdisk(${TARGET_NAME} ${ROMDISK_DIR})

add_generate_cdi_image_for_target(${TARGET_NAME})