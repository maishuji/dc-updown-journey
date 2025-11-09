add_subdirectory(udjourney)

# Editor is only available for non-Dreamcast builds (requires desktop raylib with ImGui)
if (NOT PLATFORM_DREAMCAST)
    add_subdirectory(udjourney-editor)
endif()
