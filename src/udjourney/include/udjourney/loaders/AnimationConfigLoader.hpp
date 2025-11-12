// Copyright 2025 Quentin Cartier
#pragma once

#include <string>
#include <memory>
#include "udjourney/AnimationConfig.hpp"
#include "udjourney/AnimSpriteController.hpp"

namespace udjourney {
namespace loaders {

/**
 * Loads animation configurations from JSON files and creates
 * AnimSpriteController instances
 */
class AnimationConfigLoader {
 public:
    /**
     * Load an animation preset configuration from a JSON file
     * @param filename Path to the JSON configuration file
     * @return AnimationPresetConfig loaded from file
     * @throws std::runtime_error if file cannot be loaded or parsed
     */
    static animation::AnimationPresetConfig load_preset(
        const std::string& filename);

    /**
     * Create an AnimSpriteController from a preset configuration
     * @param config The animation preset configuration
     * @return Configured AnimSpriteController ready to use
     */
    static AnimSpriteController create_controller(
        const animation::AnimationPresetConfig& config);

    /**
     * Convenience method: Load preset from file and create controller in one
     * call
     * @param filename Path to the JSON configuration file
     * @return Configured AnimSpriteController ready to use
     */
    static AnimSpriteController load_and_create(const std::string& filename);
};

}  // namespace loaders
}  // namespace udjourney
