// Copyright 2025 Quentin Cartier
#include "udjourney/ProjectilePresetLoader.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <udj-core/CoreUtils.hpp>

using json = nlohmann::json;

namespace udjourney {

bool ProjectilePresetLoader::load_from_file(const std::string& filepath) {
    std::string full_path = udj::core::filesystem::get_assets_path(filepath);

    if (!udj::core::filesystem::file_exists(full_path)) {
        std::cerr << "Projectile preset file not found: " << full_path
                  << std::endl;
        return false;
    }

    try {
        std::ifstream file(full_path);
        json j = json::parse(file);

        if (!j.contains("projectiles") || !j["projectiles"].is_array()) {
            std::cerr << "Invalid projectile preset file format" << std::endl;
            return false;
        }

        for (const auto& proj_json : j["projectiles"]) {
            ProjectilePreset preset;

            preset.name = proj_json.value("name", "unnamed");
            preset.texture_file = proj_json.value("texture_file", "");

            // Optional atlas/tile config
            preset.tile_width = proj_json.value("tile_width", 0);
            preset.tile_height = proj_json.value("tile_height", 0);
            preset.x_index = proj_json.value("x_index", 0);
            preset.y_index = proj_json.value("y_index", 0);
            preset.x_span = proj_json.value("x_span", 1);
            preset.y_span = proj_json.value("y_span", 1);

            preset.use_atlas = preset.tile_width > 0 && preset.tile_height > 0;
            if (preset.use_atlas) {
                preset.source_rect = Rectangle{
                    static_cast<float>(preset.x_index * preset.tile_width),
                    static_cast<float>(preset.y_index * preset.tile_height),
                    static_cast<float>(preset.x_span * preset.tile_width),
                    static_cast<float>(preset.y_span * preset.tile_height)};
            }
            preset.speed = proj_json.value("speed", 200.0f);
            preset.lifetime = proj_json.value("lifetime", 5.0f);
            preset.damage = proj_json.value("damage", 1);

            // Parse trajectory type
            std::string traj_str = proj_json.value("trajectory", "linear");
            if (traj_str == "linear") {
                preset.trajectory = TrajectoryType::LINEAR;
            } else if (traj_str == "arc") {
                preset.trajectory = TrajectoryType::ARC;
                preset.gravity = proj_json.value("gravity", 500.0f);
            } else if (traj_str == "sine_wave") {
                preset.trajectory = TrajectoryType::SINE_WAVE;
                preset.amplitude = proj_json.value("amplitude", 20.0f);
                preset.frequency = proj_json.value("frequency", 1.0f);
            } else if (traj_str == "homing") {
                preset.trajectory = TrajectoryType::HOMING;
            }

            // Parse collision bounds
            if (proj_json.contains("collision_bounds")) {
                const auto& bounds = proj_json["collision_bounds"];
                preset.collision_bounds.x = bounds.value("x", 0.0f);
                preset.collision_bounds.y = bounds.value("y", 0.0f);
                preset.collision_bounds.width = bounds.value("width", 8.0f);
                preset.collision_bounds.height = bounds.value("height", 8.0f);
            }

            presets_[preset.name] = preset;
            std::cout << "Loaded projectile preset: " << preset.name
                      << std::endl;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading projectile presets: " << e.what()
                  << std::endl;
        return false;
    }
}

const ProjectilePreset* ProjectilePresetLoader::get_preset(
    const std::string& name) const {
    auto it = presets_.find(name);
    if (it != presets_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool ProjectilePresetLoader::has_preset(const std::string& name) const {
    return presets_.find(name) != presets_.end();
}

std::vector<std::string> ProjectilePresetLoader::get_preset_names() const {
    std::vector<std::string> names;
    names.reserve(presets_.size());
    for (const auto& pair : presets_) {
        names.push_back(pair.first);
    }
    return names;
}

}  // namespace udjourney
