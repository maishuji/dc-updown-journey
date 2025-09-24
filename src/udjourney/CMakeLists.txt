cmake_minimum_required( VERSION 3.11.0)
# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()


set(TARGET_NAME "updown-journey")
add_executable(${TARGET_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/Platform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/reuse_strategies/RandomizePositionStrategy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/behavior_strategies/PlatformBehaviorStrategy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/behavior_strategies/HorizontalBehaviorStrategy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/behavior_strategies/OscillatingSizeBehaviorStrategy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/behavior_strategies/EightTurnHorizontalBehaviorStrategy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers/TextureManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers/HUDManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers/BonusManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hud/ScoreHUD.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hud/DialogBoxHUD.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Game.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Player.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Bonus.cpp
)

if (NOT PLATFORM_DREAMCAST)
    message(STATUS "🎭 \t\tCopying assets from ${CMAKE_CURRENT_SOURCE_DIR}/romdisk to binary dir: ${CMAKE_CURRENT_BINARY_DIR}/assets")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/romdisk/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/)
endif()

if (DEFINED PLATFORM_DREAMCAST AND PLATFORM_DREAMCAST)
    target_compile_definitions(${TARGET_NAME} PRIVATE ASSETS_BASE_PATH=\"/rd/\")
else()
    target_compile_definitions(${TARGET_NAME} PRIVATE ASSETS_BASE_PATH=\"assets/\")
endif()

set_target_properties(${TARGET_NAME} 
    PROPERTIES
        CMAKE_CXX_STANDARD 20
        CXX_EXTENSIONS OFF
)

if (NOT PLATFORM_DREAMCAST)
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
endif()
                  

target_include_directories(${TARGET_NAME}
    PUBLIC
        include)



# Note: GL must be linked after raylib
if (DEFINED PLATFORM_DREAMCAST AND PLATFORM_DREAMCAST)
    message(STATUS "Building for Dreamcast")
    target_compile_definitions(${TARGET_NAME}  PRIVATE PLATFORM_DREAMCAST=TRUE)
    target_link_libraries(${TARGET_NAME} 
        PUBLIC 
            dcplib raylib GL
    )
    # ---- ROMDISK generation ----
    set(ROMDISK_DIR "romdisk")
    kos_add_romdisk(${TARGET_NAME} ${ROMDISK_DIR})
    add_generate_cdi_image_for_target(${TARGET_NAME})
else()
    target_link_libraries(${TARGET_NAME} 
        PUBLIC 
            raylib GL
    )
endif()