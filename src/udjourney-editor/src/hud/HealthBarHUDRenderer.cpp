// Copyright 2025 Quentin Cartier
#include "udjourney-editor/hud/HealthBarHUDRenderer.hpp"
#include "udjourney-editor/Level.hpp"

void HealthBarHUDRenderer::render(const HUDElement& hud, ImDrawList* draw_list,
                                  const ImVec2& fud_pos, const ImVec2& fud_end,
                                  bool is_selected) {
    // Determine color based on type
    ImU32 bar_color = (hud.type_id == "healthbar")
                          ? IM_COL32(255, 0, 0, 150)     // Red for health
                          : IM_COL32(0, 100, 255, 150);  // Blue for mana

    // Draw 80% filled bar
    float bar_width = (hud.size.x - 8) * 0.8f;
    ImVec2 bar_start(fud_pos.x + 4, fud_pos.y + 4);
    ImVec2 bar_end_pos(fud_pos.x + 4 + bar_width, fud_end.y - 4);

    draw_list->AddRectFilled(bar_start, bar_end_pos, bar_color);
}
