// Copyright 2025 Quentin Cartier
#pragma once

#include <string>
#include <vector>
#include <memory>

namespace udjourney::editor {

struct MonsterPresetInfo {
    std::string name;          // File name without extension (e.g., "goblin")
    std::string display_name;  // Display name for UI (e.g., "Goblin Warrior")
    std::string description;   // Description for tooltip
    int health = 100;          // Health points
    int speed = 50;            // Movement speed
    std::string sprite_file;   // Main sprite sheet file
    bool is_valid = false;     // Whether the preset loaded successfully
};

class MonsterPresetManager {
 public:
    MonsterPresetManager();
    ~MonsterPresetManager() = default;

    // Load all available monster presets from assets/monsters/
    void load_available_presets();

    // Get list of all loaded presets
    const std::vector<MonsterPresetInfo>& get_presets() const {
        return presets_;
    }

    // Get preset by name (returns nullptr if not found)
    const MonsterPresetInfo* get_preset(const std::string& name) const;

    // Get preset names only (for dropdown/radio buttons)
    std::vector<std::string> get_preset_names() const;

    // Check if any presets are loaded
    bool has_presets() const { return !presets_.empty(); }

    // Get count of loaded presets
    size_t get_preset_count() const { return presets_.size(); }

 private:
    std::vector<MonsterPresetInfo> presets_;

    // Load a single preset file
    MonsterPresetInfo load_single_preset(const std::string& preset_file);

    // Extract sprite file from animation config
    std::string extract_sprite_from_animation_config(
        const std::string& anim_file);
};

}  // namespace udjourney::editor
