// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>

#include <map>
#include <memory>
#include <string>

#include "udjourney/core/events/EventDispatcher.hpp"
#include "udjourney/hud/HUDComponent.hpp"

namespace udjourney {
class ProjectilePresetLoader;
struct ProjectilePreset;
}  // namespace udjourney

class WeaponHUD : public HUDComponent {
 public:
    explicit WeaponHUD(
        Vector2 position,
        udjourney::core::events::EventDispatcher& ioEventDispatcher);
    ~WeaponHUD();

    [[nodiscard]] std::string get_type() const override { return "WeaponHUD"; }

    void update(float deltaTime) override {
        // Optional animation/logic
    }

    void draw() const override;

    // Load projectile presets for weapon previews
    void load_projectile_presets(const std::string& config_file);

 private:
    void set_weapon_name(const std::string& weapon_name);
    Texture2D load_texture_cached(const std::string& texture_path);

    std::string m_weapon_name = "";
    Vector2 m_position;

    // Projectile preset system for weapon preview
    std::unique_ptr<udjourney::ProjectilePresetLoader> m_projectile_loader;
    mutable std::map<std::string, Texture2D> m_texture_cache;
};
