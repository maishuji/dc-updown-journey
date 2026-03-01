// Copyright 2025 Quentin Cartier
#include "udjourney/particle/ParticleEmitter.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <ctime>

namespace udjourney {

namespace {
// Thread-local random number generator
thread_local std::mt19937 rng(static_cast<unsigned int>(time(nullptr)));

// Helper function for random float in range
float random_float(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}
}  // namespace

ParticleEmitter::ParticleEmitter(const ParticlePreset& preset) :
    preset_(preset) {
    particles_.reserve(100);  // Reserve space for performance
}

void ParticleEmitter::update(float delta) {
    age_ += delta;

    // Continuous emission mode (only if active)
    if (active_ && preset_.burst_count == 0 && preset_.emission_rate > 0.0f) {
        emission_accumulator_ += delta * preset_.emission_rate;

        while (emission_accumulator_ >= 1.0f) {
            spawn_particle_();
            emission_accumulator_ -= 1.0f;
        }
    }

    // Update all particles (even if emitter is inactive)
    for (auto& particle : particles_) {
        particle.update(delta);
    }

    // Clean up dead particles
    cleanup_dead_particles_();
}

void ParticleEmitter::draw(Texture2D texture, Vector2 camera_offset) const {
    for (const auto& particle : particles_) {
        if (!particle.alive) continue;

        Vector2 screen_pos = {
            particle.position.x - camera_offset.x,
            particle.position.y - camera_offset.y,
        };

        Color color = particle.get_current_color();
        float size = particle.get_current_size();

        const bool preset_has_texture = !preset_.texture_file.empty();
        const bool has_texture = preset_has_texture && (texture.id != 0);

        if (has_texture) {
            Rectangle source = {};
            if (preset_.use_atlas) {
                source = particle.texture_rect;
            } else {
                source = Rectangle{0.0f,
                                   0.0f,
                                   static_cast<float>(texture.width),
                                   static_cast<float>(texture.height)};
            }

            Rectangle dest = {screen_pos.x, screen_pos.y, size, size};
            Vector2 origin = {size / 2.0f, size / 2.0f};

            DrawTexturePro(
                texture, source, dest, origin, particle.rotation, color);
        } else {
            // No texture configured (or failed to load): draw a basic form.
            DrawCircleV(screen_pos, size / 2.0f, color);
        }
    }
}

bool ParticleEmitter::is_dead() const {
    // Emitter is dead if it has a lifetime and has expired with no particles
    if (preset_.emitter_lifetime > 0.0f) {
        return age_ >= preset_.emitter_lifetime && particles_.empty();
    }
    return false;
}

void ParticleEmitter::emit_burst() {
    int count = preset_.burst_count > 0 ? preset_.burst_count : 10;
    for (int i = 0; i < count; ++i) {
        spawn_particle_();
    }
}

void ParticleEmitter::spawn_particle_() {
    Particle particle;

    // Initialize position at emitter location
    particle.position = position_;

    // Random velocity within range
    particle.velocity.x =
        random_float(preset_.velocity_min.x, preset_.velocity_max.x);
    particle.velocity.y =
        random_float(preset_.velocity_min.y, preset_.velocity_max.y);

    // Acceleration
    particle.acceleration = preset_.acceleration;

    // Lifetime with variance
    float variance =
        random_float(-preset_.lifetime_variance, preset_.lifetime_variance);
    particle.lifetime = preset_.particle_lifetime + variance;
    particle.age = 0.0f;

    // Visual properties
    particle.start_color = preset_.start_color;
    particle.end_color = preset_.end_color;
    particle.start_size = preset_.start_size;
    particle.end_size = preset_.end_size;

    // Rotation
    particle.rotation = random_float(0.0f, 360.0f);
    particle.rotation_speed = preset_.rotation_speed;

    // Texture
    particle.texture_rect = preset_.source_rect;
    particle.alive = true;

    particles_.push_back(particle);
}

void ParticleEmitter::cleanup_dead_particles_() {
    particles_.erase(
        std::remove_if(particles_.begin(),
                       particles_.end(),
                       [](const Particle& p) { return p.is_dead(); }),
        particles_.end());
}

}  // namespace udjourney
