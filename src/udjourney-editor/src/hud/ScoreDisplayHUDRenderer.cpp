// Copyright 2025 Quentin Cartier
#include "udjourney-editor/hud/ScoreDisplayHUDRenderer.hpp"
#include "udjourney-editor/Level.hpp"

void ScoreDisplayHUDRenderer::render(const HUDElement& hud,
                                     ImDrawList* draw_list,
                                     const ImVec2& fud_pos,
                                     const ImVec2& fud_end, bool is_selected) {
    ImU32 text_color = hud.get_color_from_property("text_color");

    draw_list->AddText(
        ImVec2(fud_pos.x + 10, fud_pos.y + 10), text_color, "Score: 1234");
}
