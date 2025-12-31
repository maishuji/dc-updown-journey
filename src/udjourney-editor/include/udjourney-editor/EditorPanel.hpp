// Copyright 2025 Quentin Cartier
#pragma once
#include <imgui.h>

#include <string>
#include <vector>
#include <memory>
#include <map>

// Include Level.hpp to get enum definitions
#include "udjourney-editor/Level.hpp"
#include "udjourney-editor/MonsterPresetManager.hpp"
#include "udjourney-editor/background/BackgroundManager.hpp"
#include "udjourney-editor/background/BackgroundObjectPresetManager.hpp"

// Forward declare mode handlers
class IModeHandler;
class TileModeHandler;
class PlatformModeHandler;
class SpawnModeHandler;
class MonsterModeHandler;
class BackgroundModeHandler;
class HUDModeHandler;

namespace color {
extern const ImU32 kColorRed;
extern const ImU32 kColorGreen;
extern const ImU32 kColorBlue;
extern const ImU32 kColorOrange;
extern const ImU32 kColorLightGreen;
extern const ImU32 kColorPurple;
}  // namespace color

enum class EditMode {
    Tiles,
    Platforms,
    PlayerSpawn,
    Monsters,
    Npc,
    Background,
    HUD
};

class EditorPanel {
 public:
    EditorPanel();
    ~EditorPanel();  // Must be defined in .cpp where handler types are complete

    // Delete copy/move to avoid issues with unique_ptr of incomplete types
    EditorPanel(const EditorPanel&) = delete;
    EditorPanel& operator=(const EditorPanel&) = delete;
    EditorPanel(EditorPanel&&) = delete;
    EditorPanel& operator=(EditorPanel&&) = delete;
    void draw();

    // Set the current level for scene type awareness
    void set_current_level(Level* level) { current_level_ = level; }

    // Set background managers (called from Editor)
    void set_background_managers(BackgroundManager* bg_manager,
                                 BackgroundObjectPresetManager* preset_manager);

    inline ImU32 get_current_color() const noexcept { return cur_color; }
    inline EditMode get_edit_mode() const noexcept { return edit_mode; }
    inline PlatformBehaviorType get_platform_behavior() const noexcept {
        return platform_behavior;
    }

    // Get selected features for new platforms
    std::vector<PlatformFeatureType> get_selected_features() const;

    ImVec2 get_platform_size() const noexcept;

    void set_button(const std::string& iId, ImU32 color);
    void set_scale(float scale) noexcept;

    // Platform editing
    void set_selected_platform(EditorPlatform* platform);
    EditorPlatform* get_selected_platform() const { return selected_platform_; }

    // Monster editing
    void set_selected_monster(EditorMonster* monster);
    EditorMonster* get_selected_monster() const;
    const std::string& get_selected_monster_preset() const;
    bool should_delete_selected_monster() const;
    void clear_delete_flag();
    udjourney::editor::MonsterPresetManager* get_monster_preset_manager() {
        return &monster_preset_manager_;
    }

    // Focus management
    void request_focus() { should_focus_ = true; }

    // Initialize monster preset selection
    void initialize_monster_presets();

    // Background getters
    bool is_background_placing_mode() const;
    int get_selected_background_preset_idx() const;
    float get_background_object_scale() const;

    // Background control
    void clear_background_placing_mode();

    // HUD editing
    void set_selected_fud(HUDElement* hud);
    HUDElement* get_selected_fud() const;
    const std::string& get_selected_fud_preset() const;
    bool should_delete_selected_fud() const;
    void clear_fud_delete_flag();
    bool should_add_fud() const;
    void clear_fud_add_flag();
    HUDElement create_fud_from_preset() const;

 private:
    float scale = 1.0f;  // Default scale
    EditMode edit_mode = EditMode::Tiles;
    Level* current_level_ = nullptr;  // For scene type awareness

    // Mode handlers (Strategy pattern)
    std::unique_ptr<TileModeHandler> tile_handler_;
    std::unique_ptr<PlatformModeHandler> platform_handler_;
    std::unique_ptr<SpawnModeHandler> spawn_handler_;
    std::unique_ptr<MonsterModeHandler> monster_handler_;
    std::unique_ptr<BackgroundModeHandler> background_handler_;
    std::unique_ptr<HUDModeHandler> fud_handler_;

    // Legacy members kept for compatibility (TODO: fully migrate to handlers)
    ImU32 cur_color = IM_COL32(255, 255, 255, 255);
    ImVec2 platform_size = ImVec2(1.0f, 1.0f);
    PlatformBehaviorType platform_behavior = PlatformBehaviorType::Static;
    bool feature_spikes = false;
    bool feature_checkpoint = false;
    EditorPlatform* selected_platform_ = nullptr;

    // Monster editing (TODO: create MonsterModeHandler)
    std::string selected_monster_preset = "goblin";
    EditorMonster* selected_monster_ = nullptr;
    bool delete_selected_monster_ = false;
    udjourney::editor::MonsterPresetManager monster_preset_manager_;

    // Background management (delegated to BackgroundModeHandler)
    BackgroundManager* background_manager_ = nullptr;
    BackgroundObjectPresetManager* background_preset_manager_ = nullptr;
    int selected_preset_idx_ = 0;
    float new_bg_object_scale_ = 1.0f;
    char new_layer_name_[128] = "New Layer";
    float new_layer_parallax_ = 0.5f;
    int new_layer_depth_ = 0;
    bool background_placing_mode_ = false;

    // Layer deletion confirmation
    bool show_delete_layer_confirmation_ = false;
    size_t layer_to_delete_ = 0;
    size_t layer_object_count_ = 0;

    // Focus management
    bool should_focus_ = false;
};
