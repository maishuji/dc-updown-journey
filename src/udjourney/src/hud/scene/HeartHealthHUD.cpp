// Copyright 2025 Quentin Cartier
#include "udjourney/hud/scene/HeartHealthHUD.hpp"
#include "udj-core/Logger.hpp"
#include "udj-core/CoreUtils.hpp"
#include "udjourney/core/events/HealthChangedEvent.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <optional>
#include <cctype>

namespace udjourney {
namespace hud {
namespace scene {

std::unordered_map<std::string, Texture2D> HeartHealthHUD::s_texture_cache;

namespace {

struct HeartSpriteConfig {
    std::string sheet = "ui/ui_elements.png";
    int tile_size = 32;
    int spacing = 32;
    bool show_empty = true;

    int full_col = 0;
    int full_row = 3;
    int half_col = 2;
    int half_row = 3;
    int empty_col = 1;
    int empty_row = 3;
};

std::optional<int> parse_int(const std::string& s) {
    try {
        size_t idx = 0;
        int v = std::stoi(s, &idx);
        if (idx == 0) return std::nullopt;
        return v;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<bool> parse_bool(std::string s) {
    for (auto& ch : s) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    if (s == "true" || s == "1" || s == "yes" || s == "on") return true;
    if (s == "false" || s == "0" || s == "no" || s == "off") return false;
    return std::nullopt;
}

void apply_sprite_override_from_properties(
    const udjourney::scene::HUDData& hud_data, const char* key,
    HeartSpriteConfig& cfg, int& out_col, int& out_row) {
    auto it = hud_data.properties.find(key);
    if (it == hud_data.properties.end()) return;

    try {
        const auto sprite_json = nlohmann::json::parse(it->second);
        if (!sprite_json.is_object()) return;
        cfg.sheet = sprite_json.value("sheet", cfg.sheet);
        cfg.tile_size = sprite_json.value("tile_size", cfg.tile_size);
        out_col = sprite_json.value("tile_col", out_col);
        out_row = sprite_json.value("tile_row", out_row);
    } catch (...) {
        // Ignore bad overrides
    }
}

HeartSpriteConfig apply_level_overrides(
    const udjourney::scene::HUDData& hud_data, HeartSpriteConfig cfg) {
    // Simple scalar overrides
    if (auto it = hud_data.properties.find("heart_spacing");
        it != hud_data.properties.end()) {
        if (auto v = parse_int(it->second)) cfg.spacing = *v;
    }
    if (auto it = hud_data.properties.find("show_empty_hearts");
        it != hud_data.properties.end()) {
        if (auto v = parse_bool(it->second)) cfg.show_empty = *v;
    }

    // Sprite overrides (JSON string objects)
    apply_sprite_override_from_properties(
        hud_data, "heart_full_sprite", cfg, cfg.full_col, cfg.full_row);
    apply_sprite_override_from_properties(
        hud_data, "heart_half_sprite", cfg, cfg.half_col, cfg.half_row);
    apply_sprite_override_from_properties(
        hud_data, "heart_empty_sprite", cfg, cfg.empty_col, cfg.empty_row);

    return cfg;
}

std::optional<HeartSpriteConfig> load_heart_health_defaults_from_assets() {
    const std::string full_path =
        udj::core::filesystem::get_assets_path("huds/heart_health.json");

    std::ifstream in(full_path);
    if (!in.is_open()) {
        udj::core::Logger::info(
            "HeartHealthHUD: could not open defaults at %; using code defaults",
            full_path);
        return std::nullopt;
    }

    try {
        nlohmann::json root;
        in >> root;

        HeartSpriteConfig defaults;

        const auto props_it = root.find("properties_schema");
        if (props_it == root.end() || !props_it->is_object()) {
            return defaults;
        }
        const auto& props = *props_it;

        auto read_int_default = [&](const char* key, int& out_value) {
            const auto it = props.find(key);
            if (it == props.end() || !it->is_object()) return;
            const auto def_it = it->find("default");
            if (def_it != it->end() && def_it->is_number_integer()) {
                out_value = def_it->get<int>();
            }
        };

        auto read_bool_default = [&](const char* key, bool& out_value) {
            const auto it = props.find(key);
            if (it == props.end() || !it->is_object()) return;
            const auto def_it = it->find("default");
            if (def_it != it->end() && def_it->is_boolean()) {
                out_value = def_it->get<bool>();
            }
        };

        auto read_sprite_default = [&](const char* key,
                                       HeartSpriteConfig& cfg,
                                       int& out_col,
                                       int& out_row) {
            const auto it = props.find(key);
            if (it == props.end() || !it->is_object()) return;
            const auto def_it = it->find("default");
            if (def_it == it->end() || !def_it->is_object()) return;

            const auto& def = *def_it;
            cfg.sheet = def.value("sheet", cfg.sheet);
            cfg.tile_size = def.value("tile_size", cfg.tile_size);
            out_col = def.value("tile_col", out_col);
            out_row = def.value("tile_row", out_row);
        };

        read_int_default("heart_spacing", defaults.spacing);
        read_bool_default("show_empty_hearts", defaults.show_empty);

        read_sprite_default("heart_full_sprite",
                            defaults,
                            defaults.full_col,
                            defaults.full_row);
        read_sprite_default("heart_half_sprite",
                            defaults,
                            defaults.half_col,
                            defaults.half_row);
        read_sprite_default("heart_empty_sprite",
                            defaults,
                            defaults.empty_col,
                            defaults.empty_row);

        return defaults;
    } catch (const std::exception& e) {
        udj::core::Logger::info(
            "HeartHealthHUD: failed to parse % (%); using code defaults",
            full_path,
            e.what());
        return std::nullopt;
    }
}

const HeartSpriteConfig& heart_health_defaults() {
    static const HeartSpriteConfig kFallback;
    static std::optional<HeartSpriteConfig> cached =
        load_heart_health_defaults_from_assets();
    return cached ? *cached : kFallback;
}

}  // namespace

HeartHealthHUD::HeartHealthHUD(
    const udjourney::scene::HUDData& hud_data,
    udjourney::core::events::EventDispatcher& event_dispatcher) :
    m_hud_data(hud_data), m_visible(hud_data.visible) {
    // Seed with level-defined values (useful for editor previews and as a
    // fallback before the first runtime health event arrives).
    int max_hearts = 3;
    int current_hearts = 3;

    if (auto it = m_hud_data.properties.find("max_hearts");
        it != m_hud_data.properties.end()) {
        if (auto v = parse_int(it->second)) max_hearts = *v;
    }
    if (auto it = m_hud_data.properties.find("current_hearts");
        it != m_hud_data.properties.end()) {
        if (auto v = parse_int(it->second)) current_hearts = *v;
    }

    m_max_half_hearts = std::max(0, max_hearts * 2);
    m_current_half_hearts =
        std::clamp(current_hearts * 2, 0, m_max_half_hearts);

    // Subscribe to runtime health changes.
    event_dispatcher.register_handler(
        udjourney::core::events::HealthChangedEvent::TYPE,
        [this](const udjourney::core::events::IEvent& evt) {
            const auto& health_ev =
                static_cast<const udjourney::core::events::HealthChangedEvent&>(
                    evt);
            m_max_half_hearts = std::max(0, health_ev.max_health);
            m_current_half_hearts =
                std::clamp(health_ev.current_health, 0, m_max_half_hearts);
        });
}

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
    // Config comes from the HUD preset defaults, overridden by the level's HUD
    // properties (if provided).
    const HeartSpriteConfig cfg =
        apply_level_overrides(m_hud_data, heart_health_defaults());

    // Determine which sprite to draw
    int half_hearts_for_this_position = current_half_hearts - (heart_index * 2);
    int sprite_col, sprite_row;

    if (half_hearts_for_this_position >= 2) {
        sprite_col = cfg.full_col;
        sprite_row = cfg.full_row;
    } else if (half_hearts_for_this_position == 1) {
        sprite_col = cfg.half_col;
        sprite_row = cfg.half_row;
    } else {
        if (!cfg.show_empty) return;
        sprite_col = cfg.empty_col;
        sprite_row = cfg.empty_row;
    }

    // Load and draw heart sprite
    Texture2D tex = get_texture(cfg.sheet);
    if (tex.id > 0) {
        Rectangle source = {static_cast<float>(sprite_col * cfg.tile_size),
                            static_cast<float>(sprite_row * cfg.tile_size),
                            static_cast<float>(cfg.tile_size),
                            static_cast<float>(cfg.tile_size)};

        float heart_x = pos.x + (heart_index * cfg.spacing);
        Rectangle dest = {heart_x,
                          pos.y,
                          static_cast<float>(cfg.spacing),
                          static_cast<float>(cfg.spacing)};

        DrawTexturePro(tex, source, dest, {0, 0}, 0.0f, WHITE);
    } else {
        // Fallback: draw circle
        float heart_x = pos.x + (heart_index * cfg.spacing);
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
        } else if (cfg.show_empty) {
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
    if (!m_visible) {
        return;
    }

    if (m_max_half_hearts < 0 || m_current_half_hearts < 0) {
        return;
    }

    // Base runtime values come from events (HealthComponent units).
    int current_half_hearts = m_current_half_hearts;
    int max_hearts = (m_max_half_hearts + 1) / 2;

    // Level can still override number of heart slots shown.
    if (auto it = m_hud_data.properties.find("max_hearts");
        it != m_hud_data.properties.end()) {
        if (auto v = parse_int(it->second)) max_hearts = *v;
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
