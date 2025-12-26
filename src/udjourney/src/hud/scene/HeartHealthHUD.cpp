// Copyright 2025 Quentin Cartier
#include "udjourney/hud/scene/HeartHealthHUD.hpp"
#include "udj-core/Logger.hpp"
#include "udj-core/CoreUtils.hpp"
#include "udjourney/components/HealthComponent.hpp"
#include <nlohmann/json.hpp>

namespace udjourney {
namespace hud {
namespace scene {

std::unordered_map<std::string, Texture2D> HeartHealthHUD::s_texture_cache;

HeartHealthHUD::HeartHealthHUD(const udjourney::scene::HUDData& hud_data,
                               Player* player) :
    m_hud_data(hud_data), m_player(player), m_visible(hud_data.visible) {}

Vector2 HeartHealthHUD::calculate_position() const {
    float anchor_x = 0.0f;
    float anchor_y = 0.0f;

    float screen_width = 640.0f;
    float screen_height = 480.0f;

    switch (m_hud_data.anchor) {
        case udjourney::scene::HUDAnchor::TopLeft:
            anchor_x = 0;
            anchor_y = 0;
            break;
        case udjourney::scene::HUDAnchor::TopCenter:
            anchor_x = screen_width / 2;
            anchor_y = 0;
            break;
        case udjourney::scene::HUDAnchor::TopRight:
            anchor_x = screen_width;
            anchor_y = 0;
            break;
        case udjourney::scene::HUDAnchor::MiddleLeft:
            anchor_x = 0;
            anchor_y = screen_height / 2;
            break;
        case udjourney::scene::HUDAnchor::MiddleCenter:
            anchor_x = screen_width / 2;
            anchor_y = screen_height / 2;
            break;
        case udjourney::scene::HUDAnchor::MiddleRight:
            anchor_x = screen_width;
            anchor_y = screen_height / 2;
            break;
        case udjourney::scene::HUDAnchor::BottomLeft:
            anchor_x = 0;
            anchor_y = screen_height;
            break;
        case udjourney::scene::HUDAnchor::BottomCenter:
            anchor_x = screen_width / 2;
            anchor_y = screen_height;
            break;
        case udjourney::scene::HUDAnchor::BottomRight:
            anchor_x = screen_width;
            anchor_y = screen_height;
            break;
    }

    return Vector2{anchor_x + m_hud_data.offset_x,
                   anchor_y + m_hud_data.offset_y};
}

Texture2D HeartHealthHUD::get_texture(const std::string& path) const {
    if (s_texture_cache.find(path) == s_texture_cache.end()) {
        std::string full_path = udj::core::filesystem::get_assets_path(path);
        Texture2D tex = LoadTexture(full_path.c_str());
        if (tex.id > 0) {
            s_texture_cache[path] = tex;
            udj::core::Logger::info("Loaded heart sprite sheet: %", full_path);
        } else {
            udj::core::Logger::error("Failed to load heart sprite sheet: %",
                                     full_path);
        }
    }
    return s_texture_cache[path];
}

void HeartHealthHUD::draw_heart(Vector2 pos, int heart_index,
                                int current_half_hearts) const {
    // Parse heart sprite configuration from properties
    std::string heart_sheet = "ui/ui_elements.png";
    int heart_tile_size = 32;
    int heart_spacing = 32;
    int full_col = 0, full_row = 3;
    int half_col = 3, half_row = 3;
    int empty_col = 4, empty_row = 3;
    bool show_empty = true;

    try {
        if (m_hud_data.properties.count("heart_spacing")) {
            heart_spacing =
                std::stoi(m_hud_data.properties.at("heart_spacing"));
        }
        if (m_hud_data.properties.count("show_empty_hearts")) {
            show_empty =
                (m_hud_data.properties.at("show_empty_hearts") == "true");
        }

        if (m_hud_data.properties.count("heart_full_sprite")) {
            try {
                auto sprite_json = nlohmann::json::parse(
                    m_hud_data.properties.at("heart_full_sprite"));
                heart_sheet = sprite_json.value("sheet", heart_sheet);
                heart_tile_size =
                    sprite_json.value("tile_size", heart_tile_size);
                full_col = sprite_json.value("tile_col", full_col);
                full_row = sprite_json.value("tile_row", full_row);
            } catch (...) {
            }
        }

        if (m_hud_data.properties.count("heart_half_sprite")) {
            try {
                auto sprite_json = nlohmann::json::parse(
                    m_hud_data.properties.at("heart_half_sprite"));
                half_col = sprite_json.value("tile_col", half_col);
                half_row = sprite_json.value("tile_row", half_row);
            } catch (...) {
            }
        }

        if (m_hud_data.properties.count("heart_empty_sprite")) {
            try {
                auto sprite_json = nlohmann::json::parse(
                    m_hud_data.properties.at("heart_empty_sprite"));
                empty_col = sprite_json.value("tile_col", empty_col);
                empty_row = sprite_json.value("tile_row", empty_row);
            } catch (...) {
            }
        }
    } catch (...) {
        // Use defaults if parsing fails
    }

    // Determine which sprite to draw
    int half_hearts_for_this_position = current_half_hearts - (heart_index * 2);
    int sprite_col, sprite_row;

    if (half_hearts_for_this_position >= 2) {
        sprite_col = full_col;
        sprite_row = full_row;
    } else if (half_hearts_for_this_position == 1) {
        sprite_col = half_col;
        sprite_row = half_row;
    } else {
        if (!show_empty) return;
        sprite_col = empty_col;
        sprite_row = empty_row;
    }

    // Load and draw heart sprite
    Texture2D tex = get_texture(heart_sheet);
    if (tex.id > 0) {
        Rectangle source = {static_cast<float>(sprite_col * heart_tile_size),
                            static_cast<float>(sprite_row * heart_tile_size),
                            static_cast<float>(heart_tile_size),
                            static_cast<float>(heart_tile_size)};

        float heart_x = pos.x + (heart_index * heart_spacing);
        Rectangle dest = {heart_x,
                          pos.y,
                          static_cast<float>(heart_spacing),
                          static_cast<float>(heart_spacing)};

        DrawTexturePro(tex, source, dest, {0, 0}, 0.0f, WHITE);
    } else {
        // Fallback: draw circle
        float heart_x = pos.x + (heart_index * heart_spacing);
        bool is_full = (half_hearts_for_this_position >= 2);
        bool is_half = (half_hearts_for_this_position == 1);

        if (is_full) {
            DrawCircle(static_cast<int>(heart_x + 16),
                       static_cast<int>(pos.y + 16),
                       12,
                       RED);
        } else if (is_half) {
            DrawCircleSector(
                Vector2{heart_x + 16, pos.y + 16}, 12, 90, 270, 16, RED);
        } else if (show_empty) {
            DrawCircle(static_cast<int>(heart_x + 16),
                       static_cast<int>(pos.y + 16),
                       12,
                       ColorAlpha(RED, 0.3f));
            DrawCircleLines(static_cast<int>(heart_x + 16),
                            static_cast<int>(pos.y + 16),
                            12,
                            RED);
        }
    }
}

void HeartHealthHUD::draw() const {
    if (!m_visible || !m_player) {
        return;
    }

    // Get player health
    auto* health = m_player->get_component<HealthComponent>();
    if (!health) {
        return;
    }

    int current_half_hearts = health->get_health();
    int max_hearts = (health->get_max_health() + 1) / 2;

    // Override max_hearts from properties if specified
    try {
        if (m_hud_data.properties.count("max_hearts")) {
            max_hearts = std::stoi(m_hud_data.properties.at("max_hearts"));
        }
    } catch (...) {
    }

    Vector2 pos = calculate_position();

    // Draw all hearts
    for (int i = 0; i < max_hearts; ++i) {
        draw_heart(pos, i, current_half_hearts);
    }
}

}  // namespace scene
}  // namespace hud
}  // namespace udjourney
