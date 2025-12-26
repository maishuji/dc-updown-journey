// Copyright 2025 Quentin Cartier
#include "udjourney/hud/WeaponHUD.hpp"

#include <raylib/raylib.h>
#include <udj-core/CoreUtils.hpp>

#include <string>
#include <utility>

#include "udjourney/core/events/IEvent.hpp"
#include "udjourney/core/events/WeaponSelectedEvent.hpp"
#include "udjourney/ProjectilePresetLoader.hpp"
#include "udjourney/Projectile.hpp"

WeaponHUD::WeaponHUD(
    Vector2 iPosition,
    udjourney::core::events::EventDispatcher& ioEventDispatcher) :
    m_position(iPosition) {
    ioEventDispatcher.register_handler(
        udjourney::core::events::WeaponSelectedEvent::TYPE,
        [this](const udjourney::core::events::IEvent& evt) {
            const auto& weapon_ev = static_cast<
                const udjourney::core::events::WeaponSelectedEvent&>(evt);
            set_weapon_name(weapon_ev.weapon_name);
        });

    // Initialize projectile preset loader
    m_projectile_loader = std::make_unique<udjourney::ProjectilePresetLoader>();
}

WeaponHUD::~WeaponHUD() {
    // Unload cached textures
    for (auto& [path, texture] : m_texture_cache) {
        if (texture.id > 0) {
            UnloadTexture(texture);
        }
    }
    m_texture_cache.clear();
}

void WeaponHUD::load_projectile_presets(const std::string& config_file) {
    if (m_projectile_loader) {
        m_projectile_loader->load_from_file(config_file);
    }
}

Texture2D WeaponHUD::load_texture_cached(const std::string& texture_path) {
    // Check cache first
    auto it = m_texture_cache.find(texture_path);
    if (it != m_texture_cache.end()) {
        return it->second;
    }

    // Load texture
    std::string full_path =
        udj::core::filesystem::get_assets_path(texture_path);
    Texture2D texture = {0};

    if (udj::core::filesystem::file_exists(full_path)) {
        texture = LoadTexture(full_path.c_str());
        if (texture.id > 0) {
            m_texture_cache[texture_path] = texture;
        }
    }

    return texture;
}

void WeaponHUD::draw() const {
    const char* weapon_cstr =
        m_weapon_name.empty() ? "(none)" : m_weapon_name.c_str();

    // Calculate text position
    int text_x = static_cast<int>(m_position.x);
    int text_y = static_cast<int>(m_position.y);

    int preview_x = text_x;

    // Try to render weapon preview sprite
    if (m_projectile_loader && !m_weapon_name.empty() &&
        m_weapon_name != "(none)") {
        const auto* preset = m_projectile_loader->get_preset(m_weapon_name);

        if (preset && !preset->texture_file.empty()) {
            Texture2D texture =
                const_cast<WeaponHUD*>(this)->load_texture_cached(
                    preset->texture_file);

            if (texture.id > 0) {
                // Define preview size (small icon next to text)
                const int preview_size = 32;

                if (preset->use_atlas) {
                    // Draw from atlas using source rectangle
                    Rectangle dest = {
                        static_cast<float>(preview_x),
                        static_cast<float>(text_y -
                                           6),  // Center vertically with text
                        static_cast<float>(preview_size),
                        static_cast<float>(preview_size)};

                    DrawTexturePro(texture,
                                   preset->source_rect,
                                   dest,
                                   Vector2{0, 0},
                                   0.0f,
                                   WHITE);
                } else {
                    // Draw full texture scaled down
                    DrawTextureEx(
                        texture,
                        Vector2{static_cast<float>(preview_x),
                                static_cast<float>(text_y - 6)},
                        0.0f,
                        static_cast<float>(preview_size) / texture.width,
                        WHITE);
                }

                // Update position for weapon name text
                preview_x += preview_size + 8;  // Add spacing
            }
        }
    }

    // Draw weapon name after preview
    DrawText(weapon_cstr, preview_x, text_y, 20, YELLOW);
}

void WeaponHUD::set_weapon_name(const std::string& weapon_name) {
    m_weapon_name = weapon_name;
}
