// Copyright 2025 Quentin Cartier
#include "udjourney-editor/fud/ButtonFUDRenderer.hpp"
#include "udjourney-editor/Level.hpp"

#include <nlohmann/json.hpp>

void ButtonFUDRenderer::render(const FUDElement& fud, ImDrawList* draw_list,
                               const ImVec2& fud_pos, const ImVec2& fud_end,
                               bool is_selected) {
    // Render background sprite first
    render_background_sprite(fud, draw_list, fud_pos, fud_end);

    // Draw button text
    std::string text = get_button_text(fud);
    ImU32 text_color = get_text_color(fud);

    if (fud.background_sheet.empty()) {
        const ImGuiStyle& style = ImGui::GetStyle();
        ImGuiCol color_id =
            is_selected ? ImGuiCol_ButtonActive : ImGuiCol_Button;
        ImU32 bg_color = ImGui::ColorConvertFloat4ToU32(style.Colors[color_id]);
        draw_list->AddRectFilled(fud_pos, fud_end, bg_color, 5.0f);
    }

    // Calculate centered text position
    ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
    float text_x = fud_pos.x + (fud.size.x - text_size.x) * 0.5f;
    float text_y = fud_pos.y + (fud.size.y - text_size.y) * 0.5f;

    // Draw button boundary
    ImU32 border_color =
        is_selected ? IM_COL32(0, 255, 0, 255) : IM_COL32(200, 0, 0, 255);
    draw_list->AddRect(fud_pos, fud_end, border_color, 5.0f, 0, 2.0f);

    // Draw text
    draw_list->AddText(ImVec2(text_x, text_y), text_color, text.c_str());

    // Render foreground sprite on top
    render_foreground_sprite(fud, draw_list, fud_pos, fud_end);
}

std::string ButtonFUDRenderer::get_button_text(const FUDElement& fud) const {
    try {
        if (fud.properties.count("text")) {
            auto& text_prop = fud.properties.at("text");
            if (text_prop.is_string()) {
                return text_prop.get<std::string>();
            }
        }
    } catch (...) {
    }
    return "Button";
}

int ButtonFUDRenderer::get_font_size(const FUDElement& fud) const {
    try {
        if (fud.properties.count("font_size")) {
            auto& size_prop = fud.properties.at("font_size");
            if (size_prop.is_number_integer()) {
                return size_prop.get<int>();
            } else if (size_prop.is_string()) {
                return std::stoi(size_prop.get<std::string>());
            }
        }
    } catch (...) {
    }
    return 24;  // Default font size
}

ImU32 ButtonFUDRenderer::get_text_color(const FUDElement& fud) const {
    try {
        if (fud.properties.count("normal_color")) {
            auto& color_prop = fud.properties.at("normal_color");
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
