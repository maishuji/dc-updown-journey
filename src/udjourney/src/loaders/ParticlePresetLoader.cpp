// Copyright 2025 Quentin Cartier
#include "udjourney/loaders/ParticlePresetLoader.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <udj-core/CoreUtils.hpp>

using json = nlohmann::json;

namespace udjourney {

namespace {
    // Helper to parse Color from JSON array [r, g, b, a]
    Color parse_color(const json& j, const Color& default_color) {
        if (!j.is_array() || j.size() < 3) {
            return default_color;
        }
        return Color{
            static_cast<unsigned char>(j[0].get<int>()),
            static_cast<unsigned char>(j[1].get<int>()),
            static_cast<unsigned char>(j[2].get<int>()),
            j.size() > 3 ? static_cast<unsigned char>(j[3].get<int>()) : static_cast<unsigned char>(255)
        };
    }
    
    // Helper to parse Vector2 from JSON object {x, y}
    Vector2 parse_vector2(const json& j, const Vector2& default_vec) {
        if (!j.is_object()) {
            return default_vec;
        }
        return Vector2{
            j.value("x", default_vec.x),
            j.value("y", default_vec.y)
        };
    }
}

bool ParticlePresetLoader::load_from_file(const std::string& filepath) {
    std::string full_path = udj::core::filesystem::get_assets_path(filepath);
    
    if (!udj::core::filesystem::file_exists(full_path)) {
        std::cerr << "Particle preset file not found: " << full_path << std::endl;
        return false;
    }
    
    try {
        std::ifstream file(full_path);
        json j = json::parse(file);
        
        if (!j.contains("particles") || !j["particles"].is_array()) {
            std::cerr << "Invalid particle preset file format" << std::endl;
            return false;
        }
        
        for (const auto& particle_json : j["particles"]) {
            ParticlePreset preset;
            
            preset.name = particle_json.value("name", "unnamed");
            preset.texture_file = particle_json.value("texture_file", "");
            
            // Atlas configuration
            preset.use_atlas = particle_json.value("use_atlas", false);
            if (particle_json.contains("source_rect")) {
                const auto& rect = particle_json["source_rect"];
                preset.source_rect = Rectangle{
                    rect.value("x", 0.0f),
                    rect.value("y", 0.0f),
                    rect.value("width", 8.0f),
                    rect.value("height", 8.0f)
                };
            }
            
            // Emission properties
            preset.emission_rate = particle_json.value("emission_rate", 10.0f);
            preset.burst_count = particle_json.value("burst_count", 0);
            
            // Particle lifecycle
            preset.particle_lifetime = particle_json.value("particle_lifetime", 1.0f);
            preset.lifetime_variance = particle_json.value("lifetime_variance", 0.2f);
            
            // Velocity properties
            if (particle_json.contains("velocity_min")) {
                preset.velocity_min = parse_vector2(particle_json["velocity_min"], 
                                                    Vector2{-50.0f, -100.0f});
            }
            if (particle_json.contains("velocity_max")) {
                preset.velocity_max = parse_vector2(particle_json["velocity_max"], 
                                                    Vector2{50.0f, -50.0f});
            }
            if (particle_json.contains("acceleration")) {
                preset.acceleration = parse_vector2(particle_json["acceleration"], 
                                                   Vector2{0.0f, 200.0f});
            }
            
            // Visual properties
            if (particle_json.contains("start_color")) {
                preset.start_color = parse_color(particle_json["start_color"], 
                                                 Color{255, 255, 255, 255});
            }
            if (particle_json.contains("end_color")) {
                preset.end_color = parse_color(particle_json["end_color"], 
                                               Color{255, 255, 255, 0});
            }
            
            preset.start_size = particle_json.value("start_size", 4.0f);
            preset.end_size = particle_json.value("end_size", 2.0f);
            
            // Rotation
            preset.rotation_speed = particle_json.value("rotation_speed", 0.0f);
            
            // Emitter properties
            preset.emitter_lifetime = particle_json.value("emitter_lifetime", 0.0f);
            
            presets_[preset.name] = preset;
            std::cout << "Loaded particle preset: " << preset.name << std::endl;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading particle presets: " << e.what() << std::endl;
        return false;
    }
}

const ParticlePreset* ParticlePresetLoader::get_preset(const std::string& name) const {
    auto it = presets_.find(name);
    if (it != presets_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool ParticlePresetLoader::has_preset(const std::string& name) const {
    return presets_.find(name) != presets_.end();
}

std::vector<std::string> ParticlePresetLoader::get_preset_names() const {
    std::vector<std::string> names;
    names.reserve(presets_.size());
    for (const auto& pair : presets_) {
        names.push_back(pair.first);
    }
    return names;
}

}  // namespace udjourney
