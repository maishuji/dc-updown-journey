// Copyright 2025 Quentin Cartier
#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <memory>
#include <map>

#include "raylib/raylib.h"
#include "udjourney/AnimationConfig.hpp"

namespace udjourney {
namespace editor {

/**
 * @brief Panel for creating and editing animation presets with collision bounds
 */
class AnimationPresetPanel {
 public:
    AnimationPresetPanel();
    ~AnimationPresetPanel();

    /**
     * @brief Draw the animation preset editor panel
     */
    void draw();

    /**
     * @brief Load an animation preset from a JSON file
     * @param filepath Path to the animation preset JSON file
     * @return true if loaded successfully
     */
    bool load_preset(const std::string& filepath);

    /**
     * @brief Save the current preset to a JSON file
     * @param filepath Path to save the animation preset JSON
     * @return true if saved successfully
     */
    bool save_preset(const std::string& filepath);

    /**
     * @brief Create a new empty animation preset
     * @param preset_name Name for the new preset
     */
    void create_new_preset(const std::string& preset_name);

    /**
     * @brief Check if the panel is open
     */
    bool is_open() const { return is_open_; }

    /**
     * @brief Set the panel open state
     */
    void set_open(bool open) { is_open_ = open; }

 private:
    void draw_preset_list();
    void draw_animation_editor();
    void draw_collision_bounds_editor();
    void draw_sprite_config_editor();
    void draw_preview();
    void draw_toolbar();

    // Add a new animation state to the current preset
    void add_animation_state();

    // Remove the selected animation state
    void remove_selected_animation();

    // Duplicate the selected animation state
    void duplicate_selected_animation();

    bool is_open_ = false;
    udjourney::animation::AnimationPresetConfig current_preset_;
    int selected_animation_index_ = -1;
    std::string current_filepath_;
    bool has_unsaved_changes_ = false;

    // UI state
    char preset_name_buffer_[256] = {0};
    char animation_name_buffer_[128] = {0};
    char sprite_filename_buffer_[256] = {0};

    // Preview state
    float preview_scale_ = 2.0f;
    bool show_collision_bounds_ = true;
    bool show_sprite_grid_ = true;
    bool is_playing_ = false;
    int current_frame_index_ = 0;
    float frame_timer_ = 0.0f;

    // Texture cache for sprite previews
    std::map<std::string, Texture2D> texture_cache_;

    // Helper methods
    Texture2D load_texture_cached(const std::string& filepath);
    void cleanup_textures();
};

}  // namespace editor
}  // namespace udjourney
