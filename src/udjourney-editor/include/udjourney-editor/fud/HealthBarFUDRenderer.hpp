// Copyright 2025 Quentin Cartier
#pragma once

#include "udjourney-editor/fud/IFUDRenderer.hpp"

/**
 * @brief Renderer for health bar and mana bar FUD elements
 *
 * Displays a filled rectangle representing 80% health/mana.
 * Health bars are red, mana bars are blue.
 */
class HealthBarFUDRenderer : public IFUDRenderer {
 public:
    void render(const FUDElement& fud, ImDrawList* draw_list,
                const ImVec2& fud_pos, const ImVec2& fud_end,
                bool is_selected) override;
};
