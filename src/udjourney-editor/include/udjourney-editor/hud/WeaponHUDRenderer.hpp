// Copyright 2025 Quentin Cartier
#pragma once

#include "udjourney-editor/hud/IHUDRenderer.hpp"

/**
 * @brief Renderer for weapon display HUD elements
 *
 * Shows weapon icon preview and name
 */
class WeaponHUDRenderer : public IHUDRenderer {
 public:
    void render(const HUDElement& hud, ImDrawList* draw_list,
                const ImVec2& fud_pos, const ImVec2& fud_end,
                bool is_selected) override;
};
