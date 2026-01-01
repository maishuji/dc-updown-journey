// Copyright 2025 Quentin Cartier
#pragma once

namespace udjourney {
namespace scene {

/**
 * @brief Configuration structure for level physics parameters.
 *
 * This structure encapsulates all physics-related parameters that can be
 * configured per level, allowing for diverse gameplay experiences across
 * different levels.
 */
struct LevelPhysicsConfig {
    /// Gravity acceleration per frame (default: 0.5)
    float gravity{0.5f};

    /// Maximum falling speed/terminal velocity (default: 5.0)
    float terminal_velocity{5.0f};

    // Future extensibility for additional physics parameters:
    // float air_resistance{0.0f};      // Air friction coefficient
    // float jump_strength_multiplier{1.0f};  // Level-wide jump modifier
    // float water_density{0.0f};       // For underwater physics
    // float wind_force_x{0.0f};        // Horizontal wind force
};

}  // namespace scene
}  // namespace udjourney
