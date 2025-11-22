// Copyright 2025 Quentin Cartier
#pragma once

#include <string>
#include <vector>

struct BackgroundObjectPreset {
    std::string name;
    std::string sprite_sheet;
    int tile_size = 128;
    int tile_row = 0;
    int tile_col = 0;
    float default_scale = 1.0f;
};

class BackgroundObjectPresetManager {
 public:
    BackgroundObjectPresetManager() = default;

    bool load_from_file(const std::string& filename);
    const std::vector<BackgroundObjectPreset>& get_presets() const {
        return presets_;
    }
    const BackgroundObjectPreset* get_preset_by_name(
        const std::string& name) const;
    bool has_presets() const { return !presets_.empty(); }

 private:
    std::vector<BackgroundObjectPreset> presets_;
};
