// Copyright 2025 Quentin Cartier
#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

#include "raylib/raylib.h"
#include "udjourney/particle/ParticleEmitter.hpp"
#include "udjourney/particle/ParticlePreset.hpp"

namespace udjourney {

/**
 * @brief Manages all particle emitters in the game
 * 
 * Handles creation, updating, and rendering of all particle effects.
 * Emitters can be added/removed dynamically and are automatically cleaned up
 * when they're finished emitting.
 */
class ParticleManager {
 public:
    ParticleManager() = default;
    ~ParticleManager();
    
    ParticleManager(const ParticleManager&) = delete;
    ParticleManager& operator=(const ParticleManager&) = delete;
    
    /**
     * @brief Update all active emitters
     * @param delta Time step in seconds
     */
    void update(float delta);
    
    /**
     * @brief Draw all particles from all emitters
     */
   void draw() const;

   /**
    * @brief Draw all particles from all emitters with a camera offset
    * @param camera_offset Camera position in world space (subtracted from particle world positions)
    */
   void draw(Vector2 camera_offset) const;
    
    /**
     * @brief Create a new emitter from a preset
     * @param preset The particle preset to use
     * @param position Initial position of the emitter
     * @return Pointer to the created emitter (owned by manager)
     */
    ParticleEmitter* create_emitter(const ParticlePreset& preset, Vector2 position);
    
    /**
     * @brief Create a one-shot burst effect
     * @param preset The particle preset to use
     * @param position Position to spawn the burst
     */
    void create_burst(const ParticlePreset& preset, Vector2 position);
    
    /**
     * @brief Remove all emitters
     */
    void clear();
    
    /**
     * @brief Get total number of active particles across all emitters
     */
    [[nodiscard]] size_t get_total_particle_count() const;
    
    /**
     * @brief Get number of active emitters
     */
    [[nodiscard]] size_t get_emitter_count() const { return emitters_.size(); }

 private:
    void cleanup_dead_emitters_();
    void ensure_textures_loaded_() const;
    void unload_textures_();
    
    std::vector<std::unique_ptr<ParticleEmitter>> emitters_;
    
    // Texture cache (similar to BackgroundManager pattern)
    mutable bool textures_loaded_ = false;
    mutable std::unordered_map<std::string, Texture2D> textures_;
};

}  // namespace udjourney
