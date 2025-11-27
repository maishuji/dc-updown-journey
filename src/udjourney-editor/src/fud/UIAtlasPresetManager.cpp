// Copyright 2025 Quentin Cartier
#include "udjourney-editor/fud/UIAtlasPresetManager.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

void UIAtlasPresetManager::load_presets() {
    presets_.clear();

    // Try multiple paths for UI atlas presets
    std::vector<std::string> preset_paths = {
        "assets/ui_atlas/",
        "src/udjourney-editor/assets/ui_atlas/",
        "../src/udjourney-editor/assets/ui_atlas/"};

    for (const auto& base_path : preset_paths) {
        if (!fs::exists(base_path) || !fs::is_directory(base_path)) {
            continue;
        }

        std::cout << "Loading UI atlas presets from: " << base_path
                  << std::endl;

        for (const auto& entry : fs::directory_iterator(base_path)) {
            if (entry.is_regular_file() &&
                entry.path().extension() == ".json") {
                load_preset_file_(entry.path().string());
            }
        }
    }

    std::cout << "Loaded " << presets_.size() << " UI sprite presets"
              << std::endl;
}

void UIAtlasPresetManager::load_preset_file_(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open UI atlas preset: " << filepath
                      << std::endl;
            return;
        }

        json preset_json = json::parse(file);

        // Support both single preset and array of presets
        if (preset_json.is_array()) {
            for (const auto& item : preset_json) {
                UISpritePreset preset;
                preset.name = item.value("name", "Unnamed");
                preset.sprite_sheet = item.value("sprite_sheet", "");
                preset.tile_size = item.value("tile_size", 32);
                preset.tile_row = item.value("tile_row", 0);
                preset.tile_col = item.value("tile_col", 0);
                preset.tile_width = item.value("tile_width", 1);
                preset.tile_height = item.value("tile_height", 1);
                preset.category = item.value("category", "General");
                preset.description = item.value("description", "");

                presets_.push_back(preset);
            }
        } else {
            UISpritePreset preset;
            preset.name = preset_json.value("name", "Unnamed");
            preset.sprite_sheet = preset_json.value("sprite_sheet", "");
            preset.tile_size = preset_json.value("tile_size", 32);
            preset.tile_row = preset_json.value("tile_row", 0);
            preset.tile_col = preset_json.value("tile_col", 0);
            preset.tile_width = preset_json.value("tile_width", 1);
            preset.tile_height = preset_json.value("tile_height", 1);
            preset.category = preset_json.value("category", "General");
            preset.description = preset_json.value("description", "");

            presets_.push_back(preset);
        }

        std::cout << "Loaded UI atlas preset from: " << filepath << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error loading UI atlas preset " << filepath << ": "
                  << e.what() << std::endl;
    }
}

std::vector<UISpritePreset> UIAtlasPresetManager::get_presets_by_category(
    const std::string& category) const {
    std::vector<UISpritePreset> filtered;
    for (const auto& preset : presets_) {
        if (preset.category == category) {
            filtered.push_back(preset);
        }
    }
    return filtered;
}

const UISpritePreset* UIAtlasPresetManager::get_preset(size_t index) const {
    if (index < presets_.size()) {
        return &presets_[index];
    }
    return nullptr;
}
