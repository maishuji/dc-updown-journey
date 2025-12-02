// Copyright 2025 Quentin Cartier
#pragma once

#include "udjourney-editor/fud/IFUDRenderer.hpp"

/**
 * @brief Renderer for timer display FUD elements
 *
 * Shows example timer text "03:00"
 */
class TimerFUDRenderer : public IFUDRenderer {
 public:
    void render(const FUDElement& fud, ImDrawList* draw_list,
                const ImVec2& fud_pos, const ImVec2& fud_end,
                bool is_selected) override;
};
