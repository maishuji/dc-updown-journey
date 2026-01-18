// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>

#include <memory>
#include <string>
#include <vector>

namespace udjourney::editor {

struct PlatformPresetInfo {
    std::string name;            // Preset name (e.g., "Platform1")
    std::string sprite_sheet;    // Sprite sheet file path
    int tile_size = 64;          // Size of tile in pixels
    int tile_row = 0;            // Row in the atlas
    int tile_col = 0;            // Column in the atlas
    float default_scale = 1.0f;  // Default scale
    bool is_valid = false;       // Whether the preset loaded successfully

    // Calculate source rectangle in the sprite sheet
    Rectangle get_source_rect() const {
        return Rectangle{static_cast<float>(tile_col * tile_size),
                         static_cast<float>(tile_row * tile_size),
                         static_cast<float>(tile_size),
                         static_cast<float>(tile_size)};
    }
};

class PlatformPresetManager {
 public:
    PlatformPresetManager();
    ~PlatformPresetManager() = default;

    // Load all available platform presets from platform_presets.json
    void load_available_presets();

    // Get list of all loaded presets
    [[nodiscard]] const std::vector<PlatformPresetInfo>& get_presets() const {
        return presets_;
    }

    // Get preset by name (returns nullptr if not found)
    [[nodiscard]] const PlatformPresetInfo* get_preset(
        const std::string& name) const;

    // Get preset names only (for dropdown/radio buttons)
    [[nodiscard]] std::vector<std::string> get_preset_names() const;

    // Check if any presets are loaded
    bool has_presets() const { return !presets_.empty(); }

    // Get count of loaded presets
    [[nodiscard]] size_t get_preset_count() const { return presets_.size(); }

 private:
    std::vector<PlatformPresetInfo> presets_;

    // Load presets from the platform_presets.json file
    void load_from_json(const std::string& json_file);
};

}  // namespace udjourney::editor
