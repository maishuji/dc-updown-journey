// Copyright 2025 Quentin Cartier
#include "udjourney/components/ParticleEmitterComponent.hpp"
#include "udjourney/managers/ParticleManager.hpp"
#include "udjourney/interfaces/IActor.hpp"

namespace udjourney {

ParticleEmitterComponent::ParticleEmitterComponent(
    ParticleManager& manager,
    const ParticlePreset& preset,
    Vector2 offset)
    : manager_(manager), preset_(preset), offset_(offset) {
}

ParticleEmitterComponent::~ParticleEmitterComponent() {
    // Emitter is owned by manager, no need to delete
    // Manager will clean it up when it's dead
}

void ParticleEmitterComponent::update(float delta) {
    (void)delta;  // Not used in this component
    
    // Sync emitter position with actor position
    if (owner_ && emitter_) {
        Rectangle actor_rect = owner_->get_rectangle();
        Vector2 actor_center = {
            actor_rect.x + actor_rect.width / 2.0f,
            actor_rect.y + actor_rect.height / 2.0f
        };
        
        Vector2 emitter_pos = {
            actor_center.x + offset_.x,
            actor_center.y + offset_.y
        };
        
        emitter_->set_position(emitter_pos);
    }
}

void ParticleEmitterComponent::on_attach(IActor& actor) {
    owner_ = &actor;
    
    // Create emitter at actor's position
    Rectangle actor_rect = actor.get_rectangle();
    Vector2 initial_pos = {
        actor_rect.x + actor_rect.width / 2.0f + offset_.x,
        actor_rect.y + actor_rect.height / 2.0f + offset_.y
    };
    
    emitter_ = manager_.create_emitter(preset_, initial_pos);
}

void ParticleEmitterComponent::on_detach(IActor& actor) {
    (void)actor;
    
    // Stop emitting and let manager clean up when particles die
    if (emitter_) {
        emitter_->set_active(false);
        emitter_ = nullptr;
    }
    
    owner_ = nullptr;
}

void ParticleEmitterComponent::emit_burst() {
    if (emitter_) {
        emitter_->emit_burst();
    }
}

}  // namespace udjourney
