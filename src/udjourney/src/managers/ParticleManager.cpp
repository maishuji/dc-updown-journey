// Copyright 2025 Quentin Cartier
#include "udjourney/managers/ParticleManager.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include <udj-core/Logger.hpp>

#include "udjourney/managers/TextureManager.hpp"

namespace udjourney {

ParticleManager::~ParticleManager() { unload_textures_(); }

void ParticleManager::update(float delta) {
    // Update all emitters
    for (auto& emitter : emitters_) {
        if (emitter) {
            emitter->update(delta);
        }
    }

    // Remove dead emitters
    cleanup_dead_emitters_();
}

void ParticleManager::draw() const { draw(Vector2{0.0f, 0.0f}); }

void ParticleManager::draw(Vector2 camera_offset) const {
    ensure_textures_loaded_();

    // Draw all particles from all emitters (including inactive burst emitters)
    for (const auto& emitter : emitters_) {
        if (emitter) {
            const auto& preset = emitter->get_preset();

            // If no texture is set in the preset, draw basic form.
            Texture2D texture{0};
            if (!preset.texture_file.empty()) {
                texture = TextureManager::get_instance().get_texture(
                    preset.texture_file);
            }
            emitter->draw(texture, camera_offset);
        }
    }
}

ParticleEmitter* ParticleManager::create_emitter(const std::string& preset_name,
                                                 Vector2 position) {
    const ParticlePreset* preset = get_preset(preset_name);
    if (!preset) {
        udj::core::Logger::error("ParticleManager: Preset '" + preset_name +
                                 "' not found");
        return nullptr;
    }

    auto emitter = std::make_unique<ParticleEmitter>(*preset);
    emitter->set_position(position);

    ParticleEmitter* ptr = emitter.get();
    emitters_.push_back(std::move(emitter));

    return ptr;
}

bool ParticleManager::create_burst(const std::string& preset_name,
                                   Vector2 position) {
    const ParticlePreset* preset = get_preset(preset_name);
    if (!preset) {
        udj::core::Logger::error("ParticleManager: Preset '" + preset_name +
                                 "' not found");
        return false;
    }

    auto emitter = std::make_unique<ParticleEmitter>(*preset);
    emitter->set_position(position);
    emitter->emit_burst();
    emitter->set_active(false);  // Disable continuous emission

    emitters_.push_back(std::move(emitter));
    return true;
}

void ParticleManager::clear() {
    emitters_.clear();
    unload_textures_();
    textures_loaded_ = false;
}

size_t ParticleManager::get_total_particle_count() const {
    size_t total = 0;
    for (const auto& emitter : emitters_) {
        if (emitter) {
            total += emitter->get_particle_count();
        }
    }
    return total;
}

void ParticleManager::cleanup_dead_emitters_() {
    emitters_.erase(
        std::remove_if(emitters_.begin(),
                       emitters_.end(),
                       [](const std::unique_ptr<ParticleEmitter>& emitter) {
                           return emitter == nullptr || emitter->is_dead();
                       }),
        emitters_.end());
}

void ParticleManager::ensure_textures_loaded_() const {
    if (textures_loaded_) {
        return;
    }

    // Textures will be loaded on-demand when presets are used
    // For now, mark as loaded
    textures_loaded_ = true;
}

void ParticleManager::unload_textures_() {
    for (auto& [path, texture] : textures_) {
        if (texture.id != 0) {
            UnloadTexture(texture);
        }
    }
    textures_.clear();
    textures_loaded_ = false;
}

bool ParticleManager::load_presets(const std::string& filename) {
    return preset_loader_.load_from_file(filename);
}

const ParticlePreset* ParticleManager::get_preset(
    const std::string& name) const {
    return preset_loader_.get_preset(name);
}

}  // namespace udjourney
