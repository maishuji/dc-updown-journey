// Copyright 2025 Quentin Cartier
#pragma once

/**
 * @brief Singleton class for managing editor settings and preferences
 *
 * This class provides a centralized location for all editor settings
 * that can be modified through the UI (View menu, Settings menu, etc.)
 */
class EditorSettings {
 public:
    static EditorSettings& instance() {
        static EditorSettings instance;
        return instance;
    }

    // View settings
    bool show_grid = true;
    bool show_tiles_hints = true;

    bool show_background_placeable_rect = true;
    bool show_background_visible_rect = true;
    bool show_background_to_scene_center_hints = true;

    // HUD snap grid setting (in pixels)
    int hud_snap_grid =
        1;  // 1 = no snapping, 4, 8, 16, 32, 64 for grid snapping

    // Future settings can be added here:
    // bool snap_to_grid = true;
    // float grid_opacity = 1.0f;
    // bool show_collision_boxes = false;
    // bool show_monster_paths = false;

 private:
    EditorSettings() = default;
    EditorSettings(const EditorSettings&) = delete;
    EditorSettings& operator=(const EditorSettings&) = delete;
};
