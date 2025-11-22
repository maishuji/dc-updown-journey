// Copyright 2025 Quentin Cartier
#include "udjourney-editor/background/BackgroundObjectPresetManager.hpp"

#include <string>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

bool BackgroundObjectPresetManager::load_from_file(
    const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open background preset file: " << filename
                      << std::endl;
            return false;
        }

        nlohmann::json j;
        file >> j;

        presets_.clear();

        if (j.contains("presets") && j["presets"].is_array()) {
            for (const auto& preset_json : j["presets"]) {
                BackgroundObjectPreset preset;
                preset.name = preset_json.value("name", "Unnamed");
                preset.sprite_sheet = preset_json.value("sprite_sheet", "");
                preset.tile_size = preset_json.value("tile_size", 128);
                preset.tile_row = preset_json.value("tile_row", 0);
                preset.tile_col = preset_json.value("tile_col", 0);
                preset.default_scale = preset_json.value("default_scale", 1.0f);

                presets_.push_back(preset);
            }
        }

        std::cout << "Loaded " << presets_.size()
                  << " background object presets" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading background presets: " << e.what()
                  << std::endl;
        return false;
    }
}

const BackgroundObjectPreset* BackgroundObjectPresetManager::get_preset_by_name(
    const std::string& name) const {
    for (const auto& preset : presets_) {
        if (preset.name == name) {
            return &preset;
        }
    }
    return nullptr;
}
