// Copyright 2025 Quentin Cartier
#include "udjourney/managers/BackgroundManager.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

#include <udj-core/Logger.hpp>

#include "udjourney/scene/Scene.hpp"

namespace udjourney {

BackgroundManager::~BackgroundManager() { unload_textures_(); }

void BackgroundManager::clear() {
    m_scene = nullptr;
    m_sorted_layers.clear();
    unload_textures_();
    m_ui_scroll_y = 0.0f;
    m_textures_loaded = false;
}

void BackgroundManager::set_scene(const udjourney::scene::Scene& scene) {
    m_scene = &scene;
    m_ui_scroll_y = 0.0f;
    m_textures_loaded = false;

    rebuild_sorted_layers_();
}

void BackgroundManager::rebuild_sorted_layers_() {
    m_sorted_layers.clear();
    if (!m_scene) {
        return;
    }

    const auto& layers = m_scene->get_background_layers();
    if (layers.empty()) {
        return;
    }

    m_sorted_layers.reserve(layers.size());
    for (const auto& layer : layers) {
        m_sorted_layers.push_back(&layer);
    }

    std::sort(m_sorted_layers.begin(),
              m_sorted_layers.end(),
              [](const auto* a, const auto* b) { return a->depth < b->depth; });
}

void BackgroundManager::ensure_textures_loaded_() const {
    if (m_textures_loaded) {
        return;
    }
    if (!m_scene) {
        return;
    }

    // Critical: raylib textures require an initialized window/context.
    // Game loads the title scene in its constructor (before InitWindow).
    if (!IsWindowReady()) {
        return;
    }

    std::unordered_set<std::string> needed_sheets;

    const auto& layers = m_scene->get_background_layers();
    for (const auto& layer : layers) {
        for (const auto& obj : layer.objects) {
            if (!obj.sprite_sheet.empty()) {
                needed_sheets.insert(obj.sprite_sheet);
            }
        }
    }

    for (const auto& sheet : needed_sheets) {
        if (m_textures.find(sheet) != m_textures.end()) {
            continue;
        }

        const std::string texture_path = std::string(ASSETS_BASE_PATH) + sheet;
        Texture2D tex = LoadTexture(texture_path.c_str());
        if (tex.id > 0) {
            m_textures.emplace(sheet, tex);
            udj::core::Logger::info("Loaded background sprite sheet: %",
                                    texture_path);
        } else {
            // Cache failure as an invalid texture to avoid repeated attempts
            m_textures.emplace(sheet, tex);
            udj::core::Logger::error(
                "Failed to load background sprite sheet: %", texture_path);
        }
    }

    m_textures_loaded = true;
}

void BackgroundManager::unload_textures_() {
    for (auto& [_, tex] : m_textures) {
        if (tex.id > 0) {
            UnloadTexture(tex);
        }
    }
    m_textures.clear();
}

void BackgroundManager::update_ui_scroll(float dt, float viewport_height) {
    if (!m_scene) {
        return;
    }

    // Safe to call before InitWindow; this will no-op until window is ready.
    ensure_textures_loaded_();

    if (m_scene->get_type() != udjourney::scene::SceneType::UiScreen) {
        return;
    }

    const auto& layers = m_scene->get_background_layers();
    float default_scroll_speed = 0.0f;
    float max_scroll_limit = 0.0f;
    bool should_clamp = false;

    for (const auto& layer : layers) {
        if (!layer.auto_scroll_enabled) {
            continue;
        }

        // Use first non-zero scroll speed found
        if (default_scroll_speed == 0.0f && layer.scroll_speed_y != 0.0f) {
            default_scroll_speed = layer.scroll_speed_y;
        }

        // Clamp scrolling for layers without repeat
        if (!layer.repeat && !layer.objects.empty()) {
            should_clamp = true;

            float max_y = 0.0f;
            for (const auto& obj : layer.objects) {
                float obj_bottom = obj.y + (obj.tile_size * obj.scale);
                max_y = std::max(max_y, obj_bottom);
            }

            if (max_y > viewport_height) {
                float scroll_limit = max_y - viewport_height;
                max_scroll_limit = std::max(max_scroll_limit, scroll_limit);
            }
        }
    }

    // Legacy fallback: if any layer auto-scrolls, default speed to 30px/s
    if (default_scroll_speed == 0.0f) {
        for (const auto& layer : layers) {
            if (layer.auto_scroll_enabled) {
                default_scroll_speed = 30.0f;
                break;
            }
        }
    }

    m_ui_scroll_y += default_scroll_speed * dt;

    if (should_clamp && max_scroll_limit > 0.0f) {
        m_ui_scroll_y = std::min(m_ui_scroll_y, max_scroll_limit);
    }
}

void BackgroundManager::draw(float gameplay_camera_y, bool use_ui_scroll,
                             float /*viewport_width*/,
                             float /*viewport_height*/) const {
    if (!m_scene) {
        return;
    }

    ensure_textures_loaded_();

    const auto& layers = m_scene->get_background_layers();
    if (layers.empty()) {
        return;
    }

    const float scroll_y = use_ui_scroll ? m_ui_scroll_y : gameplay_camera_y;
    const Vector2 camera_pos = {0.0f, scroll_y};

    // Preserve existing behavior (base coordinate system: 640x480)
    constexpr float BG_CENTER_OFFSET = 320.0f;

    for (const auto* layer : m_sorted_layers) {
        if (!layer) {
            continue;
        }

        // When repeating, derive the wrap height from the layer contents.
        // This avoids a "wait until the whole background ends" gap caused by
        // using an arbitrary wrap height.
        float layer_min_y = std::numeric_limits<float>::infinity();
        float layer_max_y = -std::numeric_limits<float>::infinity();
        if (layer->repeat && !layer->objects.empty()) {
            for (const auto& obj : layer->objects) {
                const float scaled_size = obj.tile_size * obj.scale;
                layer_min_y = std::min(layer_min_y, obj.y);
                layer_max_y = std::max(layer_max_y, obj.y + scaled_size);
            }
        }
        const bool has_layer_bounds = std::isfinite(layer_min_y) &&
                                      std::isfinite(layer_max_y) &&
                                      (layer_max_y > layer_min_y);
        const float layer_wrap_height =
            has_layer_bounds ? (layer_max_y - layer_min_y) : 0.0f;

        float parallax_offset_x =
            camera_pos.x * (1.0f - layer->parallax_factor);
        float parallax_offset_y =
            camera_pos.y * (1.0f - layer->parallax_factor);

        for (const auto& obj : layer->objects) {
            if (obj.sprite_sheet.empty()) {
                continue;
            }

            const auto it = m_textures.find(obj.sprite_sheet);
            if (it == m_textures.end() || it->second.id <= 0) {
                continue;
            }
            const Texture2D& tex = it->second;

            Rectangle source = {
                static_cast<float>(obj.tile_col * obj.tile_size),
                static_cast<float>(obj.tile_row * obj.tile_size),
                static_cast<float>(obj.tile_size),
                static_cast<float>(obj.tile_size)};

            float screen_x = obj.x - parallax_offset_x - BG_CENTER_OFFSET;

            float screen_y;
            if (m_scene->get_type() == udjourney::scene::SceneType::UiScreen &&
                layer->auto_scroll_enabled) {
                float base_y = obj.y - m_ui_scroll_y;
                if (layer->repeat) {
                    const float wrap_height = (layer_wrap_height > 0.0f)
                                                  ? layer_wrap_height
                                                  : 1280.0f;

                    // Normalize into (-wrap_height, 0] so scrolling upward is
                    // continuous, then draw a second copy directly below.
                    // This makes the bottom connect to the top smoothly.
                    float local_base = base_y;
                    if (has_layer_bounds && layer_wrap_height > 0.0f) {
                        local_base = (obj.y - layer_min_y) - m_ui_scroll_y;
                    }

                    float wrapped = std::fmod(local_base, wrap_height);
                    if (wrapped > 0.0f) {
                        wrapped -= wrap_height;
                    }

                    screen_y = wrapped;
                    if (has_layer_bounds && layer_wrap_height > 0.0f) {
                        screen_y += layer_min_y;
                    }

                    const float scaled_size = obj.tile_size * obj.scale;
                    Rectangle dest_wrap = {screen_x,
                                           screen_y + wrap_height,
                                           scaled_size,
                                           scaled_size};
                    DrawTexturePro(
                        tex, source, dest_wrap, {0, 0}, obj.rotation, WHITE);
                } else {
                    screen_y = base_y;
                }
            } else if (m_scene->get_type() ==
                       udjourney::scene::SceneType::UiScreen) {
                screen_y = obj.y;
            } else {
                screen_y = obj.y - parallax_offset_y;
            }

            float scaled_size = obj.tile_size * obj.scale;
            Rectangle dest = {screen_x, screen_y, scaled_size, scaled_size};
            DrawTexturePro(tex, source, dest, {0, 0}, obj.rotation, WHITE);
        }
    }
}

}  // namespace udjourney
