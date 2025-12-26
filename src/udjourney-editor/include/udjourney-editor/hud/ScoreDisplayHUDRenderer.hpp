// Copyright 2025 Quentin Cartier
#pragma once

#include "udjourney-editor/hud/IHUDRenderer.hpp"

/**
 * @brief Renderer for score display HUD elements
 *
 * Shows example score text "Score: 1234"
 */
class ScoreDisplayHUDRenderer : public IHUDRenderer {
 public:
    void render(const HUDElement& hud, ImDrawList* draw_list,
                const ImVec2& fud_pos, const ImVec2& fud_end,
                bool is_selected) override;
};
