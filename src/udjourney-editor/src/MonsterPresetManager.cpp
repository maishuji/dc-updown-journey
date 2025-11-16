// Copyright 2025 Quentin Cartier
#include "udjourney-editor/MonsterPresetManager.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace udjourney::editor {

MonsterPresetManager::MonsterPresetManager() { load_available_presets(); }

void MonsterPresetManager::load_available_presets() {
    presets_.clear();

    auto cur_path = std::filesystem::current_path();  // Ensure current path is
                                                      // set correctly
    // std::cout << "Current working directory: " << cur_path.string() <<
    // std::endl;

    const std::string assets_path = "assets/monsters";

    if (!std::filesystem::exists(assets_path)) {
        std::cout << "Monster assets directory not found: " << assets_path
                  << std::endl;
        return;
    }

    std::cout << "Loading monster presets from: " << assets_path << std::endl;

    try {
        for (const auto& entry :
             std::filesystem::directory_iterator(assets_path)) {
            if (entry.is_regular_file() &&
                entry.path().extension() == ".json") {
                MonsterPresetInfo preset =
                    load_single_preset(entry.path().string());
                if (preset.is_valid) {
                    presets_.push_back(preset);
                    std::cout << "  Loaded preset: " << preset.name << " ("
                              << preset.display_name << ")" << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading monster presets: " << e.what() << std::endl;
    }

    std::cout << "Total monster presets loaded: " << presets_.size()
              << std::endl;
}

MonsterPresetInfo MonsterPresetManager::load_single_preset(
    const std::string& preset_file) {
    MonsterPresetInfo preset;
    preset.name = std::filesystem::path(preset_file).stem().string();

    try {
        std::ifstream file(preset_file);
        if (!file.is_open()) {
            std::cerr << "Failed to open monster preset file: " << preset_file
                      << std::endl;
            return preset;
        }

        nlohmann::json json;
        file >> json;

        // Extract basic information
        preset.display_name = json.value("name", preset.name);
        preset.description =
            json.value("description", "No description available");

        // Extract stats
        if (json.contains("stats")) {
            const auto& stats = json["stats"];
            preset.health = stats.value("health", 100);
            preset.speed = stats.value("speed", 50);
        }

        // Extract sprite information from animation config
        if (json.contains("animation_config")) {
            std::string anim_file = json["animation_config"].get<std::string>();
            preset.sprite_file =
                extract_sprite_from_animation_config(anim_file);
        }

        preset.is_valid = true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing monster preset " << preset_file << ": "
                  << e.what() << std::endl;
        preset.is_valid = false;
    }

    return preset;
}

std::string MonsterPresetManager::extract_sprite_from_animation_config(
    const std::string& anim_file) {
    try {
        std::string full_path = "assets/animations/" + anim_file;
        std::ifstream file(full_path);
        if (!file.is_open()) {
            return "";
        }

        nlohmann::json anim_json;
        file >> anim_json;

        return anim_json.value("sprite_sheet", "");
    } catch (const std::exception&) {
        return "";
    }
}

const MonsterPresetInfo* MonsterPresetManager::get_preset(
    const std::string& name) const {
    for (const auto& preset : presets_) {
        if (preset.name == name) {
            return &preset;
        }
    }
    return nullptr;
}

std::vector<std::string> MonsterPresetManager::get_preset_names() const {
    std::vector<std::string> names;
    names.reserve(presets_.size());

    for (const auto& preset : presets_) {
        names.push_back(preset.name);
    }

    return names;
}

}  // namespace udjourney::editor
