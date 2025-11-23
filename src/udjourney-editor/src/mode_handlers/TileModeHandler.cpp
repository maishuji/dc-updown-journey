// Copyright 2025 Quentin Cartier
#include "udjourney-editor/mode_handlers/TileModeHandler.hpp"

namespace color {
extern const ImU32 kColorRed;
extern const ImU32 kColorGreen;
extern const ImU32 kColorBlue;
extern const ImU32 kColorOrange;
extern const ImU32 kColorLightGreen;
extern const ImU32 kColorPurple;
}  // namespace color

TileModeHandler::TileModeHandler() {}

void TileModeHandler::render() {
    ImGui::Text("Tile Picker");
    ImGui::Separator();

    struct TileInfo {
        ImU32 color;
        const char* name;
    };

    TileInfo tiles[] = {{color::kColorRed, "Brick"},
                        {color::kColorGreen, "Grass"},
                        {color::kColorBlue, "Water"},
                        {color::kColorOrange, "Sand"},
                        {color::kColorLightGreen, "Lava"},
                        {color::kColorPurple, "Stone"}};

    int idx = 1;
    for (const auto& tile : tiles) {
        render_tile_button(tile.name, tile.color);
        if (idx % 3 != 0) {
            ImGui::SameLine();
        }
        ++idx;
    }
}

void TileModeHandler::render_tile_button(const char* name, ImU32 color) {
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

    ImVec2 button_size(32.0f * scale_, 32.0f * scale_);

    if (ImGui::Button(name, button_size)) {
        current_color_ = color;
    }

    ImGui::PopStyleColor(3);
}
