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
                      CXX_EXTENSIONS ON
                      CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH}
                      )

target_include_directories(${TARGET_NAME}
    PUBLIC
        include
        /opt/toolchains/dc/kos-ports/include
    PRIVATE
        /abcde/deb/include
        /opt/toolchains/dc/kos-ports/include
        /opt/toolchains/dc/kos/include
        /opt/toolchains/dc/kos-ports/include
)

target_compile_options(${TARGET_NAME} PUBLIC
    -I/opt/toolchains/dc/sh-elf/sh-elf/include/
    -isystem /opt/toolchains/dc/kos/kernel/arch/dreamcast/include
    -isystem /opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1/sh-elf/m4-single-only/
    -isystem /opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1/
    -isystem /opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1/tr1/
    -isystem /opt/toolchains/dc/sh-elf/sh-elf/include
    -isystem /opt/toolchains/dc/kos/include
    -I/opt/toolchains/dc/kos-ports/include
)

target_include_directories(${TARGET_NAME}
    SYSTEM PRIVATE
        /opt/toolchains/dc/sh-elf/sh-elf/include
       /opt/toolchains/dc/kos/include
       /opt/toolchains/dc/kos-ports/include
)

# GL must be linked after raylib
target_link_libraries(${TARGET_NAME} 
    PUBLIC 
        dcplib raylib GL)

# ---- ROMDISK generation ----
set(ROMDISK_DIR "romdisk")
kos_add_romdisk(${TARGET_NAME} ${ROMDISK_DIR})

add_generate_cdi_image_for_target(${TARGET_NAME})

message(STATUS "(4)CMAKE_SYSTEM_INCLUDE_PATH is set to: ${CMAKE_SYSTEM_INCLUDE_PATH}")
get_property(INCLUDE_DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

# Print out the directories
message(STATUS "Include directories in include_directories: ${INCLUDE_DIRS}")
