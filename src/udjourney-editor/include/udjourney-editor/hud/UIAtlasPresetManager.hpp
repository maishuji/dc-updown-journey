// Copyright 2025 Quentin Cartier
#pragma once

#include <string>
#include <vector>

// UI sprite preset from atlas
struct UISpritePreset {
    std::string name;          // Display name (e.g., "Health Bar Background")
    std::string sprite_sheet;  // Path to sprite sheet
    int tile_size;             // Size of each tile (e.g., 32)
    int tile_row;              // Row in sprite sheet (0-indexed)
    int tile_col;              // Column in sprite sheet (0-indexed)
    int tile_width;            // Width in tiles (default: 1)
    int tile_height;           // Height in tiles (default: 1)
    std::string category;      // e.g., "Health", "Score", "Icons"
    std::string description;   // Optional description

    UISpritePreset() :
        tile_size(32),
        tile_row(0),
        tile_col(0),
        tile_width(1),
        tile_height(1) {}
};

class UIAtlasPresetManager {
 public:
    UIAtlasPresetManager() = default;

    // Load UI sprite presets from JSON files
    void load_presets();

    // Get all loaded presets
    const std::vector<UISpritePreset>& get_presets() const { return presets_; }

    // Get presets filtered by category
    std::vector<UISpritePreset> get_presets_by_category(
        const std::string& category) const;

    // Check if any presets are loaded
    bool has_presets() const { return !presets_.empty(); }

    // Get preset by index
    const UISpritePreset* get_preset(size_t index) const;

 private:
    std::vector<UISpritePreset> presets_;
    void load_preset_file_(const std::string& filepath);
};
