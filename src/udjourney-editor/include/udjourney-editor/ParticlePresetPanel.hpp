// Copyright 2025 Quentin Cartier
#pragma once

#include <imgui.h>

#include <random>
#include <string>
#include <vector>

#include "udjourney/particle/ParticlePreset.hpp"

namespace udjourney::editor {

class ParticlePresetPanel {
 public:
    ParticlePresetPanel();
    ~ParticlePresetPanel() = default;

    void draw();

    bool is_open() const { return is_open_; }
    void set_open(bool open) { is_open_ = open; }

 private:
    struct PreviewParticle {
        Vector2 position{0.0f, 0.0f};
        Vector2 velocity{0.0f, 0.0f};
        float age = 0.0f;
        float lifetime = 1.0f;
        float rotation = 0.0f;
    };

    void draw_left_panel_();
    void draw_right_panel_();
    void draw_preset_editor_();
    void draw_preview_();

    bool load_from_assets_();
    bool save_to_assets_();

    void select_preset_(int index);
    void create_new_preset_();

    void reset_preview_();
    void update_preview_(float delta_seconds);
    void spawn_preview_particle_();

    static float lerp_(float a, float b, float t);
    static ImU32 lerp_color_(const Color& a, const Color& b, float t);

    bool is_open_ = false;
    std::string particles_json_path_;

    std::vector<udjourney::ParticlePreset> presets_;
    int selected_index_ = -1;
    bool has_unsaved_changes_ = false;

    // Preview state (derived from selected preset)
    udjourney::ParticlePreset preview_preset_;
    std::vector<PreviewParticle> preview_particles_;
    std::mt19937 rng_;
    float preview_emission_accumulator_ = 0.0f;
    float preview_emitter_age_ = 0.0f;
    bool preview_burst_emitted_ = false;

    // UI buffers
    char name_buffer_[128] = {0};
    char texture_file_buffer_[256] = {0};
};

}  // namespace udjourney::editor
