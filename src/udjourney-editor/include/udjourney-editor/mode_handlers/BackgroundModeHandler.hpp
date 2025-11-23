// Copyright 2025 Quentin Cartier
#pragma once

#include <cstddef>

#include "udjourney-editor/mode_handlers/IModeHandler.hpp"
#include "udjourney-editor/background/BackgroundManager.hpp"
#include "udjourney-editor/background/BackgroundObjectPresetManager.hpp"

/**
 * @brief Handler for Background edit mode
 *
 * Manages background layer creation, selection, and object placement.
 */
class BackgroundModeHandler : public IModeHandler {
 public:
    BackgroundModeHandler(BackgroundManager* bg_manager,
                          BackgroundObjectPresetManager* preset_manager);

    void render() override;
    void set_scale(float scale) override { scale_ = scale; }

    // Background-specific API
    bool is_placing_mode() const { return background_placing_mode_; }
    int get_selected_preset_idx() const { return selected_preset_idx_; }
    float get_object_scale() const { return new_bg_object_scale_; }
    void clear_placing_mode() {
        background_placing_mode_ = false;
        selected_preset_idx_ = -1;
    }

 private:
    BackgroundManager* background_manager_;
    BackgroundObjectPresetManager* background_preset_manager_;

    float scale_ = 1.0f;

    // Background management state
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

    // Private rendering methods
    void render_layer_list();
    void render_object_controls();
    void render_delete_confirmation();
};
