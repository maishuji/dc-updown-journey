// Copyright 2025 Quentin Cartier
#pragma once
#include <imgui.h>

#include <string>

// Include Level.hpp to get enum definitions
#include "Level.hpp"

namespace color {
extern const ImU32 kColorRed;
extern const ImU32 kColorGreen;
extern const ImU32 kColorBlue;
extern const ImU32 kColorOrange;
extern const ImU32 kColorLightGreen;
extern const ImU32 kColorPurple;
}  // namespace color

enum class EditMode { Tiles, Platforms, PlayerSpawn };

class TilePanel {
 public:
    void draw();
    inline ImU32 get_current_color() const noexcept { return cur_color; }
    inline EditMode get_edit_mode() const noexcept { return edit_mode; }
    inline PlatformBehaviorType get_platform_behavior() const noexcept {
        return platform_behavior;
    }

    // Get selected features for new platforms
    std::vector<PlatformFeatureType> get_selected_features() const;

    ImVec2 get_platform_size() const noexcept;

    void set_button(const std::string& iId, ImU32 color);
    inline void set_scale(float scale) noexcept { this->scale = scale; }

    // Platform editing
    void set_selected_platform(EditorPlatform* platform) {
        selected_platform_ = platform;
    }
    EditorPlatform* get_selected_platform() const { return selected_platform_; }

    // Focus management
    void request_focus() { should_focus_ = true; }

 private:
    float scale = 1.0f;  // Default scale

    ImVec2 platform_size = ImVec2(1.0f, 1.0f);

    ImU32 cur_color = IM_COL32(255, 255, 255,
                               255);  // Default color for the current selection
    EditMode edit_mode = EditMode::Tiles;
    PlatformBehaviorType platform_behavior = PlatformBehaviorType::Static;

    // Feature selection for new platforms
    bool feature_spikes = false;
    bool feature_checkpoint = false;

    // Currently selected platform for editing
    EditorPlatform* selected_platform_ = nullptr;

    // Focus management
    bool should_focus_ = false;

    void draw_tile_mode();
    void draw_platform_mode();
    void draw_spawn_mode();
    void draw_platform_editor();
};
