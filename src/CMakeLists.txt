# Add the core library first
add_subdirectory(udj-core)

# Add the main game
add_subdirectory(udjourney)

# Editor is only available for non-Dreamcast builds (requires desktop raylib with ImGui)
if (NOT PLATFORM_DREAMCAST)
    add_subdirectory(udjourney-editor)
endif()
