// Copyright 2025 Quentin Cartier
#pragma once

#include "udjourney-editor/background/BackgroundManager.hpp"
#include "udjourney-editor/background/BackgroundObjectPresetManager.hpp"
#include <imgui.h>

class BackgroundPanel {
 public:
    explicit BackgroundPanel(BackgroundManager& manager,
                             BackgroundObjectPresetManager& preset_manager);

    void render();

 private:
    BackgroundManager& manager_;
    BackgroundObjectPresetManager& preset_manager_;

    // UI state
    char new_layer_name_[128] = "New Layer";
    float new_parallax_factor_ = 0.5f;
    int new_depth_ = 0;

    int selected_preset_idx_ = 0;
    float new_object_x_ = 0.0f;
    float new_object_y_ = 0.0f;
    float new_object_scale_ = 1.0f;

    void render_layer_list();
    void render_layer_properties();
    void render_object_list();
    void render_add_layer_controls();
    void render_add_object_controls();
};
