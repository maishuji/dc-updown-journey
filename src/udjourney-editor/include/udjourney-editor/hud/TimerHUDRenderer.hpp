// Copyright 2025 Quentin Cartier
#pragma once

#include "udjourney-editor/hud/IHUDRenderer.hpp"

/**
 * @brief Renderer for timer display HUD elements
 *
 * Shows example timer text "03:00"
 */
class TimerHUDRenderer : public IHUDRenderer {
 public:
    void render(const HUDElement& hud, ImDrawList* draw_list,
                const ImVec2& fud_pos, const ImVec2& fud_end,
                bool is_selected) override;
};
