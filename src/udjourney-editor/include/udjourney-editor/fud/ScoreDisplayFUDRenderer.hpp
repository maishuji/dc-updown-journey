// Copyright 2025 Quentin Cartier
#pragma once

#include "udjourney-editor/fud/IFUDRenderer.hpp"

/**
 * @brief Renderer for score display FUD elements
 *
 * Shows example score text "Score: 1234"
 */
class ScoreDisplayFUDRenderer : public IFUDRenderer {
 public:
    void render(const FUDElement& fud, ImDrawList* draw_list,
                const ImVec2& fud_pos, const ImVec2& fud_end,
                bool is_selected) override;
};
