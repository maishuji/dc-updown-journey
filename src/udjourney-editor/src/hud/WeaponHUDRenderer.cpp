// Copyright 2025 Quentin Cartier
#include "udjourney-editor/hud/WeaponHUDRenderer.hpp"

#include <imgui.h>

void WeaponHUDRenderer::render(const HUDElement& hud, ImDrawList* draw_list,
                               const ImVec2& fud_pos, const ImVec2& fud_end,
                               bool is_selected) {
    // Draw background rectangle
    ImU32 bg_color = IM_COL32(0, 0, 0, 128);
    draw_list->AddRectFilled(fud_pos, fud_end, bg_color);

    // Draw border
    ImU32 border_color =
        is_selected ? IM_COL32(255, 255, 0, 255) : IM_COL32(200, 200, 200, 200);
    draw_list->AddRect(fud_pos, fud_end, border_color, 0.0f, 0, 2.0f);

    // Draw example weapon display
    ImVec2 text_pos = ImVec2(fud_pos.x + 5, fud_pos.y + 5);

    // Draw weapon icon placeholder (small square)
    ImVec2 icon_pos = text_pos;
    ImVec2 icon_end = ImVec2(icon_pos.x + 20, icon_pos.y + 20);
    draw_list->AddRectFilled(icon_pos, icon_end, IM_COL32(100, 150, 255, 200));
    draw_list->AddRect(icon_pos, icon_end, IM_COL32(150, 200, 255, 255));

    // Draw weapon name text
    ImVec2 name_pos = ImVec2(icon_end.x + 5, text_pos.y);
    draw_list->AddText(name_pos, IM_COL32(255, 255, 0, 255), "Laser");

    // Draw type label
    ImVec2 label_pos = ImVec2(fud_pos.x + 5, fud_end.y - 15);
    draw_list->AddText(
        label_pos, IM_COL32(150, 150, 150, 255), "weapon_display");
}
