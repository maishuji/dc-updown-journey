// Copyright 2025 Quentin Cartier
#pragma once

#include <map>
#include <string>
#include <vector>

#include "udjourney/particle/ParticlePreset.hpp"

namespace udjourney {

/**
 * @brief Manages loading and accessing particle presets from JSON
 */
class ParticlePresetLoader {
 public:
    /**
     * @brief Load particle presets from a JSON file
     * @param filepath Path to the JSON file containing particle presets
     * @return true if loading succeeded, false otherwise
     */
    bool load_from_file(const std::string& filepath);

    /**
     * @brief Get a particle preset by name
     * @param name Name of the preset to retrieve
     * @return Pointer to the preset, or nullptr if not found
     */
    const ParticlePreset* get_preset(const std::string& name) const;

    /**
     * @brief Check if a preset exists
     * @param name Name of the preset to check
     * @return true if the preset exists, false otherwise
     */
    bool has_preset(const std::string& name) const;

    /**
     * @brief Get all preset names
     * @return Vector of all preset names
     */
    std::vector<std::string> get_preset_names() const;

 private:
    std::map<std::string, ParticlePreset> presets_;
};

}  // namespace udjourney
