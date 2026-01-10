// Copyright 2025 Quentin Cartier
#include "udjourney-editor/PlatformPresetManager.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace udjourney::editor {

PlatformPresetManager::PlatformPresetManager() { load_available_presets(); }

void PlatformPresetManager::load_available_presets() {
    presets_.clear();

    // Try multiple possible locations for platform_presets.json
    std::vector<std::string> possible_paths = {
        "assets/platform_presets.json",
        "../src/udjourney/romdisk/platform_presets.json",
        "src/udjourney/romdisk/platform_presets.json",
    };

    std::string found_path;
    for (const auto& path : possible_paths) {
        if (std::filesystem::exists(path)) {
            found_path = path;
            break;
        }
    }

    if (found_path.empty()) {
        std::cerr << "Platform presets file not found in any expected location"
                  << std::endl;
        return;
    }

    std::cout << "Loading platform presets from: " << found_path << std::endl;

    try {
        load_from_json(found_path);
    } catch (const std::exception& e) {
        std::cerr << "Error loading platform presets: " << e.what()
                  << std::endl;
    }

    std::cout << "Total platform presets loaded: " << presets_.size()
              << std::endl;
}

void PlatformPresetManager::load_from_json(const std::string& json_file) {
    std::ifstream file(json_file);
    if (!file.is_open()) {
        std::cerr << "Failed to open platform presets file: " << json_file
                  << std::endl;
        return;
    }

    nlohmann::json json;
    file >> json;

    if (!json.contains("presets") || !json["presets"].is_array()) {
        std::cerr << "Invalid platform presets file format" << std::endl;
        return;
    }

    for (const auto& preset_json : json["presets"]) {
        PlatformPresetInfo preset;

        try {
            preset.name = preset_json.value("name", "Unnamed");
            preset.sprite_sheet = preset_json.value("sprite_sheet", "");
            preset.tile_size = preset_json.value("tile_size", 64);
            preset.tile_row = preset_json.value("tile_row", 0);
            preset.tile_col = preset_json.value("tile_col", 0);
            preset.default_scale = preset_json.value("default_scale", 1.0f);

            if (!preset.sprite_sheet.empty()) {
                preset.is_valid = true;
                presets_.push_back(preset);
                std::cout << "  Loaded preset: " << preset.name << " from "
                          << preset.sprite_sheet << " (row:" << preset.tile_row
                          << ", col:" << preset.tile_col << ")" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing platform preset: " << e.what()
                      << std::endl;
        }
    }
}

const PlatformPresetInfo* PlatformPresetManager::get_preset(
    const std::string& name) const {
    for (const auto& preset : presets_) {
        if (preset.name == name) {
            return &preset;
        }
    }
    return nullptr;
}

std::vector<std::string> PlatformPresetManager::get_preset_names() const {
    std::vector<std::string> names;
    names.reserve(presets_.size());

    for (const auto& preset : presets_) {
        names.push_back(preset.name);
    }

    return names;
}

}  // namespace udjourney::editor
