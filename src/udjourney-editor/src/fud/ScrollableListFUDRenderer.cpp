// Copyright 2025 Quentin Cartier
#include "udjourney-editor/fud/ScrollableListFUDRenderer.hpp"
#include "udjourney-editor/Level.hpp"

#include <nlohmann/json.hpp>

void ScrollableListFUDRenderer::render(const FUDElement& fud,
                                       ImDrawList* draw_list,
                                       const ImVec2& fud_pos,
                                       const ImVec2& fud_end,
                                       bool is_selected) {
    // Draw background
    ImU32 bg_color = IM_COL32(0, 0, 0, 128);  // Semi-transparent black
    draw_list->AddRectFilled(fud_pos, fud_end, bg_color);

    // Draw border
    ImU32 border_color =
        is_selected ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 255, 255, 255);
    draw_list->AddRect(fud_pos, fud_end, border_color, 0.0f, 0, 2.0f);

    // Get properties
    std::string data_source = get_data_source(fud);
    int item_height = get_item_height(fud);
    int font_size = get_font_size(fud);
    int visible_items = get_visible_items(fud);

    // Calculate how many items to show
    if (visible_items == 0) {
        visible_items = static_cast<int>((fud_end.y - fud_pos.y) / item_height);
    }

    // Draw sample items
    const char* sample_items[] = {
        "Level 1", "Level 2", "Level 3", "Level 4", "Level 5"};
    if (data_source == "settings") {
        sample_items[0] = "Volume";
        sample_items[1] = "Music";
        sample_items[2] = "Fullscreen";
        sample_items[3] = "Controls";
        sample_items[4] = "Language";
    } else if (data_source == "scores") {
        sample_items[0] = "Player 1 - 12,345";
        sample_items[1] = "Player 2 - 10,200";
        sample_items[2] = "Player 3 - 8,750";
        sample_items[3] = "Player 4 - 6,500";
        sample_items[4] = "Player 5 - 5,000";
    }

    int max_items = std::min(visible_items, 5);
    float padding = 10.0f;

    for (int i = 0; i < max_items; ++i) {
        float item_y = fud_pos.y + (i * item_height);

        // Stop if we exceed the bounds
        if (item_y + item_height > fud_end.y) {
            break;
        }

        ImVec2 item_pos(fud_pos.x, item_y);
        ImVec2 item_end(fud_end.x, item_y + item_height);

        // Highlight first item as "selected"
        if (i == 0) {
            ImU32 selected_color = IM_COL32(255, 255, 0, 76);  // Yellow tint
            draw_list->AddRectFilled(item_pos, item_end, selected_color);
            draw_list->AddRect(
                item_pos, item_end, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
        }

        // Draw item text
        ImVec2 text_pos(item_pos.x + padding, item_pos.y + padding);
        draw_list->AddText(
            text_pos, IM_COL32(255, 255, 255, 255), sample_items[i]);
    }

    // Draw scrollbar indicator if items don't fit
    if (visible_items < 5 ||
        (fud_end.y - fud_pos.y) > (visible_items * item_height)) {
        float scrollbar_x = fud_end.x - 10;
        float scrollbar_width = 5;
        float scrollbar_height = (fud_end.y - fud_pos.y) * 0.6f;
        float scrollbar_y = fud_pos.y + 10;

        draw_list->AddRectFilled(ImVec2(scrollbar_x, scrollbar_y),
                                 ImVec2(scrollbar_x + scrollbar_width,
                                        scrollbar_y + scrollbar_height),
                                 IM_COL32(150, 150, 150, 200));
    }

    // Draw data source label
    std::string label = "List: " + data_source;
    ImVec2 label_pos(fud_pos.x + padding,
                     fud_pos.y + (fud_end.y - fud_pos.y) - 20);
    draw_list->AddText(label_pos, IM_COL32(200, 200, 200, 255), label.c_str());
}

std::string ScrollableListFUDRenderer::get_data_source(
    const FUDElement& fud) const {
    try {
        if (fud.properties.count("data_source")) {
            auto& prop = fud.properties.at("data_source");
            if (prop.is_string()) {
                return prop.get<std::string>();
            }
        }
    } catch (...) {
    }
    return "levels";
}

int ScrollableListFUDRenderer::get_item_height(const FUDElement& fud) const {
    try {
        if (fud.properties.count("item_height")) {
            auto& prop = fud.properties.at("item_height");
            if (prop.is_number_integer()) {
                return prop.get<int>();
            } else if (prop.is_string()) {
                return std::stoi(prop.get<std::string>());
            }
        }
    } catch (...) {
    }
    return 80;
}

int ScrollableListFUDRenderer::get_font_size(const FUDElement& fud) const {
    try {
        if (fud.properties.count("font_size")) {
            auto& prop = fud.properties.at("font_size");
            if (prop.is_number_integer()) {
                return prop.get<int>();
            } else if (prop.is_string()) {
                return std::stoi(prop.get<std::string>());
            }
        }
    } catch (...) {
    }
    return 24;
}

int ScrollableListFUDRenderer::get_visible_items(const FUDElement& fud) const {
    try {
        if (fud.properties.count("visible_items")) {
            auto& prop = fud.properties.at("visible_items");
            if (prop.is_number_integer()) {
                return prop.get<int>();
            } else if (prop.is_string()) {
                return std::stoi(prop.get<std::string>());
            }
        }
    } catch (...) {
    }
    return 5;
}
