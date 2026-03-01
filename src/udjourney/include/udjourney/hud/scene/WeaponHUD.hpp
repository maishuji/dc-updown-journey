// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>

#include <map>
#include <memory>
#include <string>

#include "udjourney/hud/scene/IHUD.hpp"
#include "udjourney/scene/Scene.hpp"

namespace udjourney {
class ProjectilePresetLoader;
struct ProjectilePreset;

namespace core {
namespace events {
class EventDispatcher;
}
}  // namespace core

namespace hud {
namespace scene {

class WeaponHUD : public IHUD {
 public:
    WeaponHUD(const udjourney::scene::HUDData& hud_data,
              udjourney::core::events::EventDispatcher& event_dispatcher);
    ~WeaponHUD() override;

    void draw() const override;
    bool is_visible() const override { return m_visible; }
    void set_visible(bool visible) override { m_visible = visible; }

    // Load projectile presets for weapon previewsoo
    void load_projectile_presets(const std::string& config_file);

 private:
    void set_weapon_name(const std::string& weapon_name);
    Texture2D load_texture_cached(const std::string& texture_path);
    Vector2 calculate_position() const;

    const udjourney::scene::HUDData& m_hud_data;
    bool m_visible;
    std::string m_weapon_name = "bullet";

    // Projectile preset system for weapon preview
    std::unique_ptr<udjourney::ProjectilePresetLoader> m_projectile_loader;
    mutable std::map<std::string, Texture2D> m_texture_cache;
};

}  // namespace scene
}  // namespace hud
}  // namespace udjourney
