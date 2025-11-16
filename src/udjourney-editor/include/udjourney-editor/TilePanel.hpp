// Copyright 2025 Quentin Cartier
#pragma once
#include <imgui.h>

#include <string>
#include <vector>

// Include Level.hpp to get enum definitions
#include "udjourney-editor/Level.hpp"
#include "udjourney-editor/MonsterPresetManager.hpp"

namespace color {
extern const ImU32 kColorRed;
extern const ImU32 kColorGreen;
extern const ImU32 kColorBlue;
extern const ImU32 kColorOrange;
extern const ImU32 kColorLightGreen;
extern const ImU32 kColorPurple;
}  // namespace color

enum class EditMode { Tiles, Platforms, PlayerSpawn, Monsters, Npc };

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

    // Monster editing
    void set_selected_monster(EditorMonster* monster) {
        selected_monster_ = monster;
    }
    EditorMonster* get_selected_monster() const { return selected_monster_; }
    const std::string& get_selected_monster_preset() const {
        return selected_monster_preset;
    }
    bool should_delete_selected_monster() const {
        return delete_selected_monster_;
    }
    void clear_delete_flag() {
        delete_selected_monster_ = false;
        selected_monster_ = nullptr;
    }

    // Focus management
    void request_focus() { should_focus_ = true; }

    // Initialize monster preset selection
    void initialize_monster_presets();

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

    // Monster editing
    std::string selected_monster_preset = "goblin";
    EditorMonster* selected_monster_ = nullptr;
    bool delete_selected_monster_ = false;

    // Monster preset management
    udjourney::editor::MonsterPresetManager monster_preset_manager_;

    // Focus management
    bool should_focus_ = false;

    void draw_tile_mode();
    void draw_platform_mode();
    void draw_spawn_mode();
    void draw_platform_editor();
    void draw_monsters_mode();
    void draw_monster_editor();
};
