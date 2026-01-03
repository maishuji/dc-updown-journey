// Copyright 2025 Quentin Cartier
#include "udjourney-editor/hud/ButtonHUDRenderer.hpp"

#include <string>

#include "udjourney-editor/Level.hpp"

#include <nlohmann/json.hpp>

void ButtonHUDRenderer::render(const HUDElement& hud, ImDrawList* draw_list,
                               const ImVec2& fud_pos, const ImVec2& fud_end,
                               bool is_selected) {
    // Render background sprite first
    render_background_sprite(hud, draw_list, fud_pos, fud_end);

    // Draw button text
    std::string text = get_button_text(hud);
    ImU32 text_color = get_text_color(hud);
    int font_size = get_font_size(hud);

    // Raylib's default font is slightly larger than ImGui's at the same nominal
    // size Apply a scaling factor to approximate Raylib's text rendering Raylib
    // uses a bitmap font that's approximately 1.2x larger than ImGui's for the
    // same point size
    float raylib_scale_factor = 1.2f;
    float scaled_font_size =
        static_cast<float>(font_size) * raylib_scale_factor;

    if (hud.background_sheet.empty()) {
        const ImGuiStyle& style = ImGui::GetStyle();
        ImGuiCol color_id =
            is_selected ? ImGuiCol_ButtonActive : ImGuiCol_Button;
        ImU32 bg_color = ImGui::ColorConvertFloat4ToU32(style.Colors[color_id]);
        draw_list->AddRectFilled(fud_pos, fud_end, bg_color, 5.0f);
    }

    // Calculate text size with Raylib-approximated scaling
    ImFont* font = ImGui::GetFont();
    ImVec2 text_size =
        font->CalcTextSizeA(scaled_font_size, FLT_MAX, 0.0f, text.c_str());
    float text_x = fud_pos.x + (hud.size.x - text_size.x) * 0.5f;
    float text_y = fud_pos.y + (hud.size.y - text_size.y) * 0.5f;

    // Draw button boundary
    ImU32 border_color =
        is_selected ? IM_COL32(0, 255, 0, 255) : IM_COL32(200, 0, 0, 255);
    draw_list->AddRect(fud_pos, fud_end, border_color, 5.0f, 0, 2.0f);

    // Draw text with Raylib-approximated size
    draw_list->AddText(font,
                       scaled_font_size,
                       ImVec2(text_x, text_y),
                       text_color,
                       text.c_str());

    // Render foreground sprite on top
    render_foreground_sprite(hud, draw_list, fud_pos, fud_end);
}

std::string ButtonHUDRenderer::get_button_text(const HUDElement& hud) const {
    auto opt_text = hud.get_string_from_property("text");
    if (opt_text.has_value()) {
        return opt_text.value();
    }
    return "Button";
}

int ButtonHUDRenderer::get_font_size(const HUDElement& hud) const {
    auto opt_size = hud.get_int_from_property("font_size");
    if (opt_size.has_value()) {
        return opt_size.value();
    }
    return 24;  // Default font size
}

ImU32 ButtonHUDRenderer::get_text_color(const HUDElement& hud) const {
    try {
        if (hud.properties.count("normal_color")) {
            auto& color_prop = hud.properties.at("normal_color");
            if (color_prop.is_array() && color_prop.size() >= 3) {
                int r = color_prop[0].get<int>();
                int g = color_prop[1].get<int>();
                int b = color_prop[2].get<int>();
                return IM_COL32(r, g, b, 200);
            }
        }
    } catch (...) {
    }
    return IM_COL32(255, 255, 255, 200);  // Default white
}
