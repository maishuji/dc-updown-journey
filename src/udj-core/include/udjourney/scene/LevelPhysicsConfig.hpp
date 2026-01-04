// Copyright 2025 Quentin Cartier
#pragma once

namespace udjourney {
namespace scene {

/**
 * @brief Configuration structure for level physics parameters.
 */
struct LevelPhysicsConfig {
    /// Gravity acceleration per frame (default: 0.5)
    float gravity{0.5f};

    /// Maximum falling speed/terminal velocity (default: 5.0)
    float terminal_velocity{5.0f};
};

}  // namespace scene
}  // namespace udjourney
