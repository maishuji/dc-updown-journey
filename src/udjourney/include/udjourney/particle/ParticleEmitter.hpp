// Copyright 2025 Quentin Cartier
#pragma once

#include <vector>
#include "raylib/raylib.h"
#include "udjourney/particle/Particle.hpp"
#include "udjourney/particle/ParticlePreset.hpp"

namespace udjourney {

/**
 * @brief Emitter that spawns and manages particles based on a preset
 */
class ParticleEmitter {
 public:
    explicit ParticleEmitter(const ParticlePreset& preset);
    ~ParticleEmitter() = default;

   [[nodiscard]] const ParticlePreset& get_preset() const { return preset_; }
    
    /**
     * @brief Update emitter and all its particles
     * @param delta Time step in seconds
     */
    void update(float delta);
    
    /**
     * @brief Emit particles (called by ParticleManager during rendering)
     * @param texture Texture to use for rendering
      * @param camera_offset Camera position in world space (subtracted from particle world positions)
     */
       void draw(Texture2D texture, Vector2 camera_offset) const;
    
    /**
     * @brief Set emitter position
     */
    void set_position(Vector2 position) { position_ = position; }
    
    /**
     * @brief Get emitter position
     */
    [[nodiscard]] Vector2 get_position() const { return position_; }
    
    /**
     * @brief Check if emitter is dead and should be removed
     */
    [[nodiscard]] bool is_dead() const;
    
    /**
     * @brief Emit a burst of particles immediately
     */
    void emit_burst();
    
    /**
     * @brief Get particle count
     */
    [[nodiscard]] size_t get_particle_count() const { return particles_.size(); }
    
    /**
     * @brief Enable/disable emitter
     */
    void set_active(bool active) { active_ = active; }
    
    /**
     * @brief Check if emitter is active
     */
    [[nodiscard]] bool is_active() const { return active_; }

 private:
    void spawn_particle_();
    void cleanup_dead_particles_();
    
    ParticlePreset preset_;
    Vector2 position_{0.0f, 0.0f};
    std::vector<Particle> particles_;
    
    float emission_accumulator_ = 0.0f; // For continuous emission
    float age_ = 0.0f;                  // Emitter age
    bool active_ = true;
};

}  // namespace udjourney
