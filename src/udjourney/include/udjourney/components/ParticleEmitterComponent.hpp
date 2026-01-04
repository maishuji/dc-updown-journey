// Copyright 2025 Quentin Cartier
#pragma once

#include "udjourney/interfaces/IComponent.hpp"
#include "udjourney/particle/ParticlePreset.hpp"

namespace udjourney {

class ParticleManager;
class ParticleEmitter;

/**
 * @brief Component that attaches a particle emitter to an actor
 * 
 * Automatically syncs the emitter position with the owner actor's position.
 * The emitter is registered with ParticleManager on attach and unregistered on detach.
 */
class ParticleEmitterComponent : public IComponent {
 public:
    /**
     * @brief Construct particle emitter component
     * @param manager Reference to the particle manager
     * @param preset Particle preset to use for this emitter
     * @param offset Offset from actor position (default: center of actor)
     */
    explicit ParticleEmitterComponent(
        ParticleManager& manager,
        const ParticlePreset& preset,
        Vector2 offset = {0.0f, 0.0f});
    
    ~ParticleEmitterComponent() override;
    
    void update(float delta) override;
    void on_attach(IActor& actor) override;
    void on_detach(IActor& actor) override;
    
    /**
     * @brief Get the emitter (can be nullptr if not attached)
     */
    [[nodiscard]] ParticleEmitter* get_emitter() const { return emitter_; }
    
    /**
     * @brief Set position offset relative to actor
     */
    void set_offset(Vector2 offset) { offset_ = offset; }
    
    /**
     * @brief Emit a burst of particles
     */
    void emit_burst();

 private:
    ParticleManager& manager_;
    ParticlePreset preset_;
    Vector2 offset_;
    IActor* owner_ = nullptr;
    ParticleEmitter* emitter_ = nullptr;  // Owned by manager
};

}  // namespace udjourney
