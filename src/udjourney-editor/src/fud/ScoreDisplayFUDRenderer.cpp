// Copyright 2025 Quentin Cartier
#include "udjourney-editor/fud/ScoreDisplayFUDRenderer.hpp"
#include "udjourney-editor/Level.hpp"

void ScoreDisplayFUDRenderer::render(const FUDElement& fud,
                                     ImDrawList* draw_list,
                                     const ImVec2& fud_pos,
                                     const ImVec2& fud_end, bool is_selected) {
    draw_list->AddText(ImVec2(fud_pos.x + 10, fud_pos.y + 10),
                       IM_COL32(255, 255, 255, 200),
                       "Score: 1234");
}
