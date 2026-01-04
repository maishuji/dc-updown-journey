// Copyright 2025 Quentin Cartier
#include "udjourney-editor/ParticlePresetPanel.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include "udj-core/CoreUtils.hpp"

namespace udjourney::editor {

namespace {
float clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

float uniform_float(std::mt19937& rng, float min_value, float max_value) {
    if (min_value > max_value) std::swap(min_value, max_value);
    std::uniform_real_distribution<float> dist(min_value, max_value);
    return dist(rng);
}

int uniform_int(std::mt19937& rng, int min_value, int max_value) {
    if (min_value > max_value) std::swap(min_value, max_value);
    std::uniform_int_distribution<int> dist(min_value, max_value);
    return dist(rng);
}

}  // namespace

ParticlePresetPanel::ParticlePresetPanel() {
    particles_json_path_ =
        udjourney::coreutils::get_assets_path("particles.json");

    std::random_device rd;
    rng_ = std::mt19937(rd());

    load_from_assets_();
    if (!presets_.empty()) {
        select_preset_(0);
    }
}

void ParticlePresetPanel::draw() {
    if (!is_open_) return;

    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Particles Presets Editor", &is_open_)) {
        // Toolbar
        if (ImGui::Button("New")) {
            create_new_preset_();
        }
        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            if (save_to_assets_()) {
                has_unsaved_changes_ = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload")) {
            load_from_assets_();
            if (!presets_.empty()) {
                if (selected_index_ < 0 ||
                    selected_index_ >= static_cast<int>(presets_.size())) {
                    select_preset_(0);
                } else {
                    select_preset_(selected_index_);
                }
            } else {
                selected_index_ = -1;
            }
        }

        ImGui::SameLine();
        ImGui::TextDisabled("%s", particles_json_path_.c_str());

        if (has_unsaved_changes_) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), "(unsaved)");
        }

        ImGui::Separator();

        ImGui::BeginChild("LeftPanel", ImVec2(300, 0), true);
        draw_left_panel_();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
        draw_right_panel_();
        ImGui::EndChild();
    }
    ImGui::End();
}

void ParticlePresetPanel::draw_left_panel_() {
    ImGui::Text("Presets");
    ImGui::Separator();

    if (presets_.empty()) {
        ImGui::TextDisabled("No presets loaded");
        return;
    }

    for (int i = 0; i < static_cast<int>(presets_.size()); ++i) {
        const auto& preset = presets_[i];
        bool selected = (i == selected_index_);
        if (ImGui::Selectable(preset.name.c_str(), selected)) {
            select_preset_(i);
        }
    }
}

void ParticlePresetPanel::draw_right_panel_() {
    if (selected_index_ < 0 ||
        selected_index_ >= static_cast<int>(presets_.size())) {
        ImGui::TextDisabled("Select a preset to edit");
        return;
    }

    draw_preset_editor_();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Preview");

    float delta = ImGui::GetIO().DeltaTime;
    if (delta <= 0.0f) delta = 1.0f / 60.0f;
    update_preview_(delta);

    draw_preview_();
}

void ParticlePresetPanel::draw_preset_editor_() {
    auto& p = presets_[selected_index_];

    auto tooltip = [](const char* text) {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
            ImGui::SetTooltip("%s", text);
        }
    };

    // Name + texture
    std::strncpy(name_buffer_, p.name.c_str(), sizeof(name_buffer_) - 1);
    std::strncpy(texture_file_buffer_,
                 p.texture_file.c_str(),
                 sizeof(texture_file_buffer_) - 1);

    if (ImGui::InputText("Name", name_buffer_, sizeof(name_buffer_))) {
        p.name = name_buffer_;
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip(
        "Unique preset identifier used to reference this effect in code/JSON.");

    if (ImGui::InputText("Texture File",
                         texture_file_buffer_,
                         sizeof(texture_file_buffer_))) {
        p.texture_file = texture_file_buffer_;
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip(
        "Optional texture file for particles (relative to assets).\nIf empty, "
        "the preview uses simple circles.");

    if (ImGui::Checkbox("Use Atlas", &p.use_atlas)) {
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip(
        "When enabled, particles are rendered using a sub-rectangle (Source "
        "Rect) of the texture.");

    // Source rect
    ImGui::Text("Source Rect");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
        ImGui::SetTooltip(
            "Texture sub-rectangle used when Use Atlas is enabled.\n"
            "X/Y are the top-left pixel in the texture; W/H are the size in "
            "pixels.");
    }
    int rect_x = static_cast<int>(p.source_rect.x);
    int rect_y = static_cast<int>(p.source_rect.y);
    int rect_w = static_cast<int>(p.source_rect.width);
    int rect_h = static_cast<int>(p.source_rect.height);
    bool rect_changed = false;
    rect_changed |= ImGui::InputInt("X", &rect_x);
    tooltip("Source rect X (pixels) in the particle texture.");
    rect_changed |= ImGui::InputInt("Y", &rect_y);
    tooltip("Source rect Y (pixels) in the particle texture.");
    rect_changed |= ImGui::InputInt("W", &rect_w);
    tooltip("Source rect width (pixels). Only used if Use Atlas is enabled.");
    rect_changed |= ImGui::InputInt("H", &rect_h);
    tooltip("Source rect height (pixels). Only used if Use Atlas is enabled.");
    if (rect_changed) {
        p.source_rect.x = static_cast<float>(rect_x);
        p.source_rect.y = static_cast<float>(rect_y);
        p.source_rect.width = static_cast<float>(std::max(0, rect_w));
        p.source_rect.height = static_cast<float>(std::max(0, rect_h));
        has_unsaved_changes_ = true;
        reset_preview_();
    }

    ImGui::Separator();

    // Emission
    if (ImGui::InputFloat(
            "Emission Rate", &p.emission_rate, 1.0f, 10.0f, "%.2f")) {
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip(
        "Continuous emission rate in particles/second.\nSet to 0 for "
        "burst-only effects.");
    if (ImGui::InputInt("Burst Count", &p.burst_count)) {
        p.burst_count = std::max(0, p.burst_count);
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip(
        "If > 0, spawns this many particles at once (burst).\nIf 0, the effect "
        "relies on Emission Rate.");

    // Lifetimes
    if (ImGui::InputFloat(
            "Particle Lifetime", &p.particle_lifetime, 0.05f, 0.2f, "%.2f")) {
        p.particle_lifetime = std::max(0.01f, p.particle_lifetime);
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip("Base lifetime (seconds) for each particle before it disappears.");
    if (ImGui::InputFloat(
            "Lifetime Variance", &p.lifetime_variance, 0.01f, 0.1f, "%.2f")) {
        p.lifetime_variance = std::max(0.0f, p.lifetime_variance);
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip(
        "Random +/- variance added to Particle Lifetime for each particle "
        "(seconds).");

    // Velocity/acceleration
    if (ImGui::SliderFloat2(
            "Velocity Min", &p.velocity_min.x, -500.0f, 500.0f, "%.1f")) {
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip(
        "Minimum initial velocity (x,y).\nEach particle picks a random "
        "velocity between Min and Max.");
    if (ImGui::SliderFloat2(
            "Velocity Max", &p.velocity_max.x, -500.0f, 500.0f, "%.1f")) {
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip(
        "Maximum initial velocity (x,y).\nEach particle picks a random "
        "velocity between Min and Max.");
    if (ImGui::SliderFloat2(
            "Acceleration", &p.acceleration.x, -1000.0f, 1000.0f, "%.1f")) {
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip(
        "Constant acceleration applied every frame (x,y).\nUse this like "
        "gravity or wind.");

    // Colors (0..255 stored)
    float start_rgba[4] = {p.start_color.r / 255.0f,
                           p.start_color.g / 255.0f,
                           p.start_color.b / 255.0f,
                           p.start_color.a / 255.0f};
    float end_rgba[4] = {p.end_color.r / 255.0f,
                         p.end_color.g / 255.0f,
                         p.end_color.b / 255.0f,
                         p.end_color.a / 255.0f};

    if (ImGui::ColorEdit4("Start Color", start_rgba)) {
        p.start_color.r =
            static_cast<unsigned char>(clamp01(start_rgba[0]) * 255.0f);
        p.start_color.g =
            static_cast<unsigned char>(clamp01(start_rgba[1]) * 255.0f);
        p.start_color.b =
            static_cast<unsigned char>(clamp01(start_rgba[2]) * 255.0f);
        p.start_color.a =
            static_cast<unsigned char>(clamp01(start_rgba[3]) * 255.0f);
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip(
        "Particle color at spawn (RGBA).\nParticles interpolate from Start "
        "Color to End Color over lifetime.");

    if (ImGui::ColorEdit4("End Color", end_rgba)) {
        p.end_color.r =
            static_cast<unsigned char>(clamp01(end_rgba[0]) * 255.0f);
        p.end_color.g =
            static_cast<unsigned char>(clamp01(end_rgba[1]) * 255.0f);
        p.end_color.b =
            static_cast<unsigned char>(clamp01(end_rgba[2]) * 255.0f);
        p.end_color.a =
            static_cast<unsigned char>(clamp01(end_rgba[3]) * 255.0f);
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip(
        "Particle color at the end of its life (RGBA).\nCommonly fades alpha "
        "to 0.");

    // Size / rotation / emitter lifetime
    if (ImGui::InputFloat("Start Size", &p.start_size, 0.5f, 2.0f, "%.2f")) {
        p.start_size = std::max(0.0f, p.start_size);
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip("Particle size at spawn (pixels). Interpolates to End Size.");
    if (ImGui::InputFloat("End Size", &p.end_size, 0.5f, 2.0f, "%.2f")) {
        p.end_size = std::max(0.0f, p.end_size);
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip("Particle size at the end of its life (pixels).");
    if (ImGui::InputFloat(
            "Rotation Speed", &p.rotation_speed, 10.0f, 50.0f, "%.1f")) {
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip(
        "Rotation speed in degrees/second.\nOnly visible when using textured "
        "rendering.");
    if (ImGui::InputFloat(
            "Emitter Lifetime", &p.emitter_lifetime, 0.1f, 0.5f, "%.2f")) {
        p.emitter_lifetime = std::max(0.0f, p.emitter_lifetime);
        has_unsaved_changes_ = true;
        reset_preview_();
    }
    tooltip(
        "How long the emitter keeps spawning particles (seconds).\n0 means "
        "infinite (never auto-stops).");
}

void ParticlePresetPanel::draw_preview_() {
    ImGui::BeginChild("ParticlePreview", ImVec2(0, 250), true);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImVec2 center(origin.x + avail.x * 0.5f, origin.y + avail.y * 0.5f);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRect(origin,
                       ImVec2(origin.x + avail.x, origin.y + avail.y),
                       IM_COL32(80, 80, 80, 255));

    for (const auto& particle : preview_particles_) {
        float t =
            particle.lifetime > 0.0f ? particle.age / particle.lifetime : 1.0f;
        t = clamp01(t);

        float size =
            lerp_(preview_preset_.start_size, preview_preset_.end_size, t);
        ImU32 col = lerp_color_(
            preview_preset_.start_color, preview_preset_.end_color, t);

        ImVec2 p(center.x + particle.position.x,
                 center.y + particle.position.y);
        float radius = std::max(0.5f, size * 0.5f);
        draw_list->AddCircleFilled(p, radius, col, 12);
    }

    ImGui::EndChild();
}

bool ParticlePresetPanel::load_from_assets_() {
    presets_.clear();

    std::ifstream file(particles_json_path_);
    if (!file.is_open()) {
        std::cerr << "Failed to open particle presets: " << particles_json_path_
                  << std::endl;
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;

        if (!j.contains("particles") || !j["particles"].is_array()) {
            std::cerr
                << "Invalid particles.json format: missing particles array"
                << std::endl;
            return false;
        }

        for (const auto& pjson : j["particles"]) {
            udjourney::ParticlePreset p;
            p.name = pjson.value("name", "");
            p.texture_file = pjson.value("texture_file", "");
            p.use_atlas = pjson.value("use_atlas", false);

            if (pjson.contains("source_rect")) {
                const auto& r = pjson["source_rect"];
                p.source_rect.x = r.value("x", 0);
                p.source_rect.y = r.value("y", 0);
                p.source_rect.width = r.value("width", 8);
                p.source_rect.height = r.value("height", 8);
            }

            p.emission_rate = pjson.value("emission_rate", 0.0f);
            p.burst_count = pjson.value("burst_count", 0);

            p.particle_lifetime = pjson.value("particle_lifetime", 1.0f);
            p.lifetime_variance = pjson.value("lifetime_variance", 0.0f);

            if (pjson.contains("velocity_min")) {
                p.velocity_min.x = pjson["velocity_min"].value("x", 0.0f);
                p.velocity_min.y = pjson["velocity_min"].value("y", 0.0f);
            }
            if (pjson.contains("velocity_max")) {
                p.velocity_max.x = pjson["velocity_max"].value("x", 0.0f);
                p.velocity_max.y = pjson["velocity_max"].value("y", 0.0f);
            }
            if (pjson.contains("acceleration")) {
                p.acceleration.x = pjson["acceleration"].value("x", 0.0f);
                p.acceleration.y = pjson["acceleration"].value("y", 0.0f);
            }

            if (pjson.contains("start_color") &&
                pjson["start_color"].is_array() &&
                pjson["start_color"].size() == 4) {
                p.start_color.r = pjson["start_color"][0].get<int>();
                p.start_color.g = pjson["start_color"][1].get<int>();
                p.start_color.b = pjson["start_color"][2].get<int>();
                p.start_color.a = pjson["start_color"][3].get<int>();
            }
            if (pjson.contains("end_color") && pjson["end_color"].is_array() &&
                pjson["end_color"].size() == 4) {
                p.end_color.r = pjson["end_color"][0].get<int>();
                p.end_color.g = pjson["end_color"][1].get<int>();
                p.end_color.b = pjson["end_color"][2].get<int>();
                p.end_color.a = pjson["end_color"][3].get<int>();
            }

            p.start_size = pjson.value("start_size", 4.0f);
            p.end_size = pjson.value("end_size", 2.0f);
            p.rotation_speed = pjson.value("rotation_speed", 0.0f);
            p.emitter_lifetime = pjson.value("emitter_lifetime", 0.0f);

            if (!p.name.empty()) {
                presets_.push_back(p);
            }
        }

        has_unsaved_changes_ = false;
        reset_preview_();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing particles.json: " << e.what() << std::endl;
        return false;
    }
}

bool ParticlePresetPanel::save_to_assets_() {
    try {
        nlohmann::json j;
        j["particles"] = nlohmann::json::array();

        for (const auto& p : presets_) {
            nlohmann::json pj;
            pj["name"] = p.name;
            pj["texture_file"] = p.texture_file;
            pj["use_atlas"] = p.use_atlas;
            pj["source_rect"] = {
                {"x", static_cast<int>(p.source_rect.x)},
                {"y", static_cast<int>(p.source_rect.y)},
                {"width", static_cast<int>(p.source_rect.width)},
                {"height", static_cast<int>(p.source_rect.height)}};

            pj["emission_rate"] = p.emission_rate;
            pj["burst_count"] = p.burst_count;
            pj["particle_lifetime"] = p.particle_lifetime;
            pj["lifetime_variance"] = p.lifetime_variance;
            pj["velocity_min"] = {{"x", p.velocity_min.x},
                                  {"y", p.velocity_min.y}};
            pj["velocity_max"] = {{"x", p.velocity_max.x},
                                  {"y", p.velocity_max.y}};
            pj["acceleration"] = {{"x", p.acceleration.x},
                                  {"y", p.acceleration.y}};

            pj["start_color"] = {p.start_color.r,
                                 p.start_color.g,
                                 p.start_color.b,
                                 p.start_color.a};
            pj["end_color"] = {
                p.end_color.r, p.end_color.g, p.end_color.b, p.end_color.a};

            pj["start_size"] = p.start_size;
            pj["end_size"] = p.end_size;
            pj["rotation_speed"] = p.rotation_speed;
            pj["emitter_lifetime"] = p.emitter_lifetime;

            j["particles"].push_back(pj);
        }

        std::ofstream out(particles_json_path_);
        if (!out.is_open()) {
            std::cerr << "Failed to write particles.json: "
                      << particles_json_path_ << std::endl;
            return false;
        }
        out << j.dump(2);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving particles.json: " << e.what() << std::endl;
        return false;
    }
}

void ParticlePresetPanel::select_preset_(int index) {
    if (index < 0 || index >= static_cast<int>(presets_.size())) return;
    selected_index_ = index;

    std::strncpy(name_buffer_,
                 presets_[selected_index_].name.c_str(),
                 sizeof(name_buffer_) - 1);
    std::strncpy(texture_file_buffer_,
                 presets_[selected_index_].texture_file.c_str(),
                 sizeof(texture_file_buffer_) - 1);

    reset_preview_();
}

void ParticlePresetPanel::create_new_preset_() {
    udjourney::ParticlePreset p;

    int suffix = static_cast<int>(presets_.size()) + 1;
    p.name = "new_particle_" + std::to_string(suffix);
    p.texture_file = "";
    p.use_atlas = false;
    p.source_rect = {0, 0, 8, 8};

    p.emission_rate = 20.0f;
    p.burst_count = 0;

    p.particle_lifetime = 1.0f;
    p.lifetime_variance = 0.2f;

    p.velocity_min = {-50.0f, -80.0f};
    p.velocity_max = {50.0f, -30.0f};
    p.acceleration = {0.0f, 120.0f};

    p.start_color = {255, 255, 255, 200};
    p.end_color = {255, 255, 255, 0};

    p.start_size = 6.0f;
    p.end_size = 1.0f;

    p.rotation_speed = 0.0f;
    p.emitter_lifetime = 0.0f;

    presets_.push_back(p);
    has_unsaved_changes_ = true;
    select_preset_(static_cast<int>(presets_.size()) - 1);
}

void ParticlePresetPanel::reset_preview_() {
    preview_particles_.clear();
    preview_emission_accumulator_ = 0.0f;
    preview_emitter_age_ = 0.0f;
    preview_burst_emitted_ = false;

    if (selected_index_ >= 0 &&
        selected_index_ < static_cast<int>(presets_.size())) {
        preview_preset_ = presets_[selected_index_];

        // Burst presets need an immediate emission to be visible.
        if (preview_preset_.burst_count > 0) {
            for (int i = 0; i < preview_preset_.burst_count; ++i) {
                spawn_preview_particle_();
            }
            preview_burst_emitted_ = true;
        }
    }
}

void ParticlePresetPanel::update_preview_(float delta_seconds) {
    if (selected_index_ < 0 ||
        selected_index_ >= static_cast<int>(presets_.size())) {
        return;
    }

    // Keep preview preset in sync with current edit state
    preview_preset_ = presets_[selected_index_];

    preview_emitter_age_ += delta_seconds;

    bool emitter_alive = true;
    if (preview_preset_.emitter_lifetime > 0.0f &&
        preview_emitter_age_ > preview_preset_.emitter_lifetime) {
        emitter_alive = false;
    }

    if (emitter_alive) {
        if (!preview_burst_emitted_ && preview_preset_.burst_count > 0) {
            for (int i = 0; i < preview_preset_.burst_count; ++i) {
                spawn_preview_particle_();
            }
            preview_burst_emitted_ = true;
        }

        if (preview_preset_.emission_rate > 0.0f) {
            preview_emission_accumulator_ +=
                preview_preset_.emission_rate * delta_seconds;
            while (preview_emission_accumulator_ >= 1.0f) {
                spawn_preview_particle_();
                preview_emission_accumulator_ -= 1.0f;
            }
        }
    }

    // Update particles
    for (auto& particle : preview_particles_) {
        particle.age += delta_seconds;
        particle.velocity.x += preview_preset_.acceleration.x * delta_seconds;
        particle.velocity.y += preview_preset_.acceleration.y * delta_seconds;
        particle.position.x += particle.velocity.x * delta_seconds;
        particle.position.y += particle.velocity.y * delta_seconds;
        particle.rotation += preview_preset_.rotation_speed * delta_seconds;
    }

    // Cleanup dead
    preview_particles_.erase(std::remove_if(preview_particles_.begin(),
                                            preview_particles_.end(),
                                            [](const PreviewParticle& p) {
                                                return p.age >= p.lifetime;
                                            }),
                             preview_particles_.end());

    // If everything died (burst presets), restart automatically so the preview
    // keeps showing.
    if (preview_particles_.empty() && preview_preset_.burst_count > 0) {
        preview_emitter_age_ = 0.0f;
        preview_burst_emitted_ = false;
    }
}

void ParticlePresetPanel::spawn_preview_particle_() {
    PreviewParticle p;
    p.position = {0.0f, 0.0f};

    p.velocity.x = uniform_float(
        rng_, preview_preset_.velocity_min.x, preview_preset_.velocity_max.x);
    p.velocity.y = uniform_float(
        rng_, preview_preset_.velocity_min.y, preview_preset_.velocity_max.y);

    float variance = std::max(0.0f, preview_preset_.lifetime_variance);
    float lifetime_min =
        std::max(0.01f, preview_preset_.particle_lifetime - variance);
    float lifetime_max =
        std::max(lifetime_min, preview_preset_.particle_lifetime + variance);
    p.lifetime = uniform_float(rng_, lifetime_min, lifetime_max);

    // Small random initial offset helps dense bursts read better
    p.position.x += static_cast<float>(uniform_int(rng_, -2, 2));
    p.position.y += static_cast<float>(uniform_int(rng_, -2, 2));

    preview_particles_.push_back(p);
}

float ParticlePresetPanel::lerp_(float a, float b, float t) {
    return a + (b - a) * t;
}

ImU32 ParticlePresetPanel::lerp_color_(const Color& a, const Color& b,
                                       float t) {
    t = clamp01(t);

    auto lerp_u8 = [t](unsigned char av, unsigned char bv) {
        float v = static_cast<float>(av) +
                  (static_cast<float>(bv) - static_cast<float>(av)) * t;
        v = std::round(v);
        if (v < 0.0f) v = 0.0f;
        if (v > 255.0f) v = 255.0f;
        return static_cast<unsigned char>(v);
    };

    unsigned char r = lerp_u8(a.r, b.r);
    unsigned char g = lerp_u8(a.g, b.g);
    unsigned char bl = lerp_u8(a.b, b.b);
    unsigned char al = lerp_u8(a.a, b.a);

    return IM_COL32(r, g, bl, al);
}

}  // namespace udjourney::editor
