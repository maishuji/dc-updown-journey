// Copyright 2025 Quentin Cartier
#include "udjourney-editor/fud/IFUDRenderer.hpp"

#include <raylib/raylib.h>

#include <algorithm>

#include "udjourney-editor/Level.hpp"

FUDRendererFactory& FUDRendererFactory::instance() {
    static FUDRendererFactory factory;
    return factory;
}

void FUDRendererFactory::register_renderer(
    const std::string& type_id, std::unique_ptr<IFUDRenderer> renderer) {
    renderers_[type_id] = std::move(renderer);
}

IFUDRenderer* FUDRendererFactory::get_renderer(const std::string& type_id) {
    auto it = renderers_.find(type_id);
    if (it != renderers_.end()) {
        return it->second.get();
    }
    return nullptr;
}

void IFUDRenderer::render_background_sprite(const FUDElement& fud,
                                            ImDrawList* draw_list,
                                            const ImVec2& fud_pos,
                                            const ImVec2& fud_end) {
    if (fud.background_sheet.empty()) return;

    render_sprite_with_mode(fud,
                            draw_list,
                            fud_pos,
                            fud_end,
                            fud.background_sheet,
                            fud.background_tile_size,
                            fud.background_tile_col,
                            fud.background_tile_row,
                            fud.background_tile_width,
                            fud.background_tile_height,
                            static_cast<int>(fud.background_render_mode),
                            180);
}

void IFUDRenderer::render_foreground_sprite(const FUDElement& fud,
                                            ImDrawList* draw_list,
                                            const ImVec2& fud_pos,
                                            const ImVec2& fud_end) {
    if (fud.foreground_sheet.empty()) return;

    render_sprite_with_mode(fud,
                            draw_list,
                            fud_pos,
                            fud_end,
                            fud.foreground_sheet,
                            fud.foreground_tile_size,
                            fud.foreground_tile_col,
                            fud.foreground_tile_row,
                            fud.foreground_tile_width,
                            fud.foreground_tile_height,
                            static_cast<int>(fud.foreground_render_mode),
                            200);
}

void IFUDRenderer::render_sprite_with_mode(
    const FUDElement& fud, ImDrawList* draw_list, const ImVec2& fud_pos,
    const ImVec2& fud_end, const std::string& sheet, int tile_size,
    int tile_col, int tile_row, int tile_width, int tile_height,
    int render_mode, unsigned char alpha) {
    Texture2D texture = load_texture_cached(sheet);
    if (texture.id == 0) return;

    // Calculate source rect in sprite sheet
    float tile_w = static_cast<float>(tile_size);
    float tile_h = static_cast<float>(tile_size);
    ImVec2 uv0((tile_col * tile_w) / texture.width,
               (tile_row * tile_h) / texture.height);
    ImVec2 uv1(uv0.x + ((tile_width * tile_w) / texture.width),
               uv0.y + ((tile_height * tile_h) / texture.height));

    float sprite_w = tile_width * tile_w;
    float sprite_h = tile_height * tile_h;

    // FUDImageRenderMode: Stretch=0, Tile=1, Center=2
    if (render_mode == 1) {  // Tile
        float fud_w = fud.size.x;
        float fud_h = fud.size.y;

        for (float y = 0; y < fud_h; y += sprite_h) {
            for (float x = 0; x < fud_w; x += sprite_w) {
                float draw_w = std::min(sprite_w, fud_w - x);
                float draw_h = std::min(sprite_h, fud_h - y);

                // Calculate UV for potentially clipped tile
                float uv_w =
                    (draw_w / sprite_w) * (tile_width * tile_w / texture.width);
                float uv_h = (draw_h / sprite_h) *
                             (tile_height * tile_h / texture.height);
                ImVec2 tile_uv1(uv0.x + uv_w, uv0.y + uv_h);

                draw_list->AddImage(
                    static_cast<ImTextureID>(static_cast<intptr_t>(texture.id)),
                    ImVec2(fud_pos.x + x, fud_pos.y + y),
                    ImVec2(fud_pos.x + x + draw_w, fud_pos.y + y + draw_h),
                    uv0,
                    tile_uv1,
                    IM_COL32(255, 255, 255, alpha));
            }
        }
    } else if (render_mode == 2) {  // Center
        float center_x = fud_pos.x + (fud.size.x - sprite_w) * 0.5f;
        float center_y = fud_pos.y + (fud.size.y - sprite_h) * 0.5f;

        draw_list->AddImage(
            static_cast<ImTextureID>(static_cast<intptr_t>(texture.id)),
            ImVec2(center_x, center_y),
            ImVec2(center_x + sprite_w, center_y + sprite_h),
            uv0,
            uv1,
            IM_COL32(255, 255, 255, alpha));
    } else {  // Stretch (default)
        draw_list->AddImage(
            static_cast<ImTextureID>(static_cast<intptr_t>(texture.id)),
            fud_pos,
            fud_end,
            uv0,
            uv1,
            IM_COL32(255, 255, 255, alpha));
    }
}
