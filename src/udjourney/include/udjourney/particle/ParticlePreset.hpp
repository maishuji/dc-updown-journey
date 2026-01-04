// Copyright 2025 Quentin Cartier
#pragma once

#include <string>

#include "raylib/raylib.h"

namespace udjourney {

/**
 * @brief Configuration for a particle effect type
 */
struct ParticlePreset {
	std::string name;
	std::string texture_file;

	// Atlas/tile rendering
	bool use_atlas = false;
	Rectangle source_rect{0, 0, 8, 8};  // Source rect in texture

	// Emission properties
	float emission_rate = 10.0f;  // Particles per second
	int burst_count = 0;          // 0 = continuous, >0 = burst mode

	// Particle lifecycle
	float particle_lifetime = 1.0f;  // How long each particle lives (seconds)
	float lifetime_variance = 0.2f;  // Random variance in lifetime

	// Velocity properties
	Vector2 velocity_min{-50.0f, -100.0f};
	Vector2 velocity_max{50.0f, -50.0f};
	Vector2 acceleration{0.0f, 200.0f};  // Gravity-like acceleration

	// Visual properties
	Color start_color{255, 255, 255, 255};
	Color end_color{255, 255, 255, 0};  // Fade to transparent by default
	float start_size = 4.0f;
	float end_size = 2.0f;

	// Rotation
	float rotation_speed = 0.0f;  // Degrees per second

	// Emitter properties
	float emitter_lifetime = 0.0f;  // 0 = infinite, >0 = auto-destroy after time
};

}  // namespace udjourney
