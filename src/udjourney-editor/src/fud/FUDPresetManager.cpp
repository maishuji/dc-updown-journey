// Copyright 2025 Quentin Cartier
#include "udjourney-editor/fud/FUDPresetManager.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

FUDPresetManager::FUDPresetManager() { load_available_presets(); }

void FUDPresetManager::load_available_presets() {
    presets_.clear();

    std::vector<std::string> preset_paths = {
        "assets/fuds",
        "src/udjourney-editor/assets/fuds",
        "../assets/fuds",
        "../src/udjourney-editor/assets/fuds"};

    for (const auto& base_path : preset_paths) {
        if (!fs::exists(base_path) || !fs::is_directory(base_path)) {
            continue;
        }

        std::cout << "[FUDPresetManager] Loading FUD presets from: "
                  << base_path << std::endl;

        for (const auto& entry : fs::directory_iterator(base_path)) {
            if (entry.is_regular_file() &&
                entry.path().extension() == ".json") {
                if (load_preset_from_file(entry.path().string())) {
                    std::cout << "[FUDPresetManager] Loaded preset: "
                              << entry.path().filename().string() << std::endl;
                }
            }
        }

        if (!presets_.empty()) {
            break;  // Found presets, stop searching
        }
    }

    if (presets_.empty()) {
        std::cout << "[FUDPresetManager] Warning: No FUD presets loaded"
                  << std::endl;
    } else {
        std::cout << "[FUDPresetManager] Total presets loaded: "
                  << presets_.size() << std::endl;
    }
}

bool FUDPresetManager::load_preset_from_file(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[FUDPresetManager] Failed to open: " << filepath
                      << std::endl;
            return false;
        }

        nlohmann::json j;
        file >> j;

        FUDPreset preset;
        preset.type_id = j.at("type_id").get<std::string>();
        preset.display_name = j.at("display_name").get<std::string>();
        preset.category =
            fud_category_from_string(j.at("category").get<std::string>());
        preset.default_size.x = j.at("default_size").at("x").get<float>();
        preset.default_size.y = j.at("default_size").at("y").get<float>();
        preset.default_anchor =
            fud_anchor_from_string(j.at("default_anchor").get<std::string>());
        preset.properties_schema = j.at("properties_schema");

        if (j.contains("preview_image")) {
            preset.preview_image = j.at("preview_image").get<std::string>();
        }

        if (j.contains("description")) {
            preset.description = j.at("description").get<std::string>();
        }

        presets_.push_back(preset);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[FUDPresetManager] Error loading " << filepath << ": "
                  << e.what() << std::endl;
        return false;
    }
}

const FUDPreset* FUDPresetManager::get_preset(
    const std::string& type_id) const {
    for (const auto& preset : presets_) {
        if (preset.type_id == type_id) {
            return &preset;
        }
    }
    return nullptr;
}

std::vector<std::string> FUDPresetManager::get_preset_type_ids() const {
    std::vector<std::string> type_ids;
    type_ids.reserve(presets_.size());
    for (const auto& preset : presets_) {
        type_ids.push_back(preset.type_id);
    }
    return type_ids;
}

std::vector<FUDPreset> FUDPresetManager::get_presets_by_category(
    FUDCategory category) const {
    std::vector<FUDPreset> filtered;
    for (const auto& preset : presets_) {
        if (preset.category == category) {
            filtered.push_back(preset);
        }
    }
    return filtered;
}
