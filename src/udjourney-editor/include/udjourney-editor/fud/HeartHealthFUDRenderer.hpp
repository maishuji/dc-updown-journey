// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>

#include "udjourney-editor/fud/IFUDRenderer.hpp"

/**
 * @brief Renderer for heart-based health display FUD elements
 *
 * Renders multiple heart sprites based on max_hearts property.
 * Falls back to circle rendering if sprites are not available.
 */
class HeartHealthFUDRenderer : public IFUDRenderer {
 public:
    void render(const FUDElement& fud, ImDrawList* draw_list,
                const ImVec2& fud_pos, const ImVec2& fud_end,
                bool is_selected) override;

 private:
    /**
     * @brief Render hearts using sprite textures
     */
    void render_hearts_with_sprites(const FUDElement& fud,
                                    ImDrawList* draw_list,
                                    const ImVec2& fud_pos, Texture2D heart_tex,
                                    int max_hearts, int heart_spacing,
                                    int heart_tile_size, int full_col,
                                    int full_row);

    /**
     * @brief Render hearts as circles (fallback)
     */
    void render_hearts_fallback(ImDrawList* draw_list, const ImVec2& fud_pos,
                                int max_hearts, int heart_spacing);
};
