// Copyright 2025 Quentin Cartier
#pragma once

#include "udjourney-editor/hud/IHUDRenderer.hpp"

/**
 * @brief Renderer for health bar and mana bar HUD elements
 *
 * Displays a filled rectangle representing 80% health/mana.
 * Health bars are red, mana bars are blue.
 */
class HealthBarHUDRenderer : public IHUDRenderer {
 public:
    void render(const HUDElement& hud, ImDrawList* draw_list,
                const ImVec2& fud_pos, const ImVec2& fud_end,
                bool is_selected) override;
};
