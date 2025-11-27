// Copyright 2025 Quentin Cartier
#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "udjourney-editor/fud/FUDElement.hpp"

class FUDPresetManager {
 public:
    FUDPresetManager();

    // Load all FUD presets from assets/fuds/ directory
    void load_available_presets();

    // Get all loaded presets
    const std::vector<FUDPreset>& get_presets() const { return presets_; }

    // Get preset by type_id
    const FUDPreset* get_preset(const std::string& type_id) const;

    // Check if any presets are loaded
    bool has_presets() const { return !presets_.empty(); }

    // Get preset type IDs
    std::vector<std::string> get_preset_type_ids() const;

    // Get presets filtered by category
    std::vector<FUDPreset> get_presets_by_category(FUDCategory category) const;

 private:
    std::vector<FUDPreset> presets_;

    // Load a single preset from JSON file
    bool load_preset_from_file(const std::string& filepath);
};
