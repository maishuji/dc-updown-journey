// Copyright 2025 Quentin Cartier
#include "udjourney-editor/hud/HeartHealthHUDRenderer.hpp"

#include <string>

#include "udjourney-editor/Level.hpp"

#include <nlohmann/json.hpp>

// External function from EditorScene
extern Texture2D load_texture_cached(const std::string& filename);

void HeartHealthHUDRenderer::render(const HUDElement& hud,
                                    ImDrawList* draw_list,
                                    const ImVec2& fud_pos,
                                    const ImVec2& fud_end, bool is_selected) {
    // Extract properties
    int max_hearts = 3;
    int heart_spacing = 32;
    std::string heart_sheet = "ui/ui_elements.png";
    int heart_tile_size = 32;
    int full_col = 0, full_row = 3;

    try {
        if (hud.properties.count("max_hearts")) {
            auto& prop = hud.properties.at("max_hearts");
            if (prop.is_number_integer()) {
                max_hearts = prop.get<int>();
            } else if (prop.is_string()) {
                max_hearts = std::stoi(prop.get<std::string>());
            }
        }
        if (hud.properties.count("heart_spacing")) {
            auto& prop = hud.properties.at("heart_spacing");
            if (prop.is_number_integer()) {
                heart_spacing = prop.get<int>();
            } else if (prop.is_string()) {
                heart_spacing = std::stoi(prop.get<std::string>());
            }
        }

        // Try to parse heart sprite config
        if (hud.properties.count("heart_full_sprite")) {
            try {
                auto sprite_obj = hud.properties.at("heart_full_sprite");
                if (sprite_obj.is_object()) {
                    heart_sheet = sprite_obj.value("sheet", heart_sheet);
                    heart_tile_size =
                        sprite_obj.value("tile_size", heart_tile_size);
                    full_col = sprite_obj.value("tile_col", full_col);
                    full_row = sprite_obj.value("tile_row", full_row);
                }
            } catch (...) {
            }
        }
    } catch (...) {
    }

    // Try to render with actual sprites
    Texture2D heart_tex = load_texture_cached(heart_sheet);
    if (heart_tex.id > 0) {
        render_hearts_with_sprites(hud,
                                   draw_list,
                                   fud_pos,
                                   heart_tex,
                                   max_hearts,
                                   heart_spacing,
                                   heart_tile_size,
                                   full_col,
                                   full_row);
    } else {
        render_hearts_fallback(draw_list, fud_pos, max_hearts, heart_spacing);
    }
}

void HeartHealthHUDRenderer::render_hearts_with_sprites(
    const HUDElement& hud, ImDrawList* draw_list, const ImVec2& fud_pos,
    Texture2D heart_tex, int max_hearts, int heart_spacing, int heart_tile_size,
    int full_col, int full_row) {
    for (int h = 0; h < max_hearts; ++h) {
        float hx = fud_pos.x + (h * heart_spacing);
        float hy = fud_pos.y;

        ImVec2 uv0(
            (full_col * heart_tile_size) / static_cast<float>(heart_tex.width),
            (full_row * heart_tile_size) /
                static_cast<float>(heart_tex.height));
        ImVec2 uv1(
            uv0.x + (heart_tile_size / static_cast<float>(heart_tex.width)),
            uv0.y + (heart_tile_size / static_cast<float>(heart_tex.height)));

        draw_list->AddImage(
            static_cast<ImTextureID>(static_cast<intptr_t>(heart_tex.id)),
            ImVec2(hx, hy),
            ImVec2(hx + heart_spacing, hy + heart_spacing),
            uv0,
            uv1,
            IM_COL32(255, 255, 255, 200));
    }
}

void HeartHealthHUDRenderer::render_hearts_fallback(ImDrawList* draw_list,
                                                    const ImVec2& fud_pos,
                                                    int max_hearts,
                                                    int heart_spacing) {
    for (int h = 0; h < max_hearts; ++h) {
        float hx = fud_pos.x + (h * heart_spacing) + 16;
        float hy = fud_pos.y + 16;
        draw_list->AddCircleFilled(
            ImVec2(hx, hy), 10.0f, IM_COL32(255, 50, 50, 150));
    }
}
