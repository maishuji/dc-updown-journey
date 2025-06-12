
#include "udjourney-editor/TilePanel.hpp"

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <string>
#include <vector>

namespace color {
const auto kColorRed = IM_COL32(255, 0, 0, 255);
const auto kColorGreen = IM_COL32(0, 255, 0, 255);
const auto kColorBlue = IM_COL32(0, 0, 255, 255);
const auto kColorOrange = IM_COL32(255, 128, 0, 255);
const auto kColorLightGreen = IM_COL32(0, 255, 128, 255);
const auto kColorPurple = IM_COL32(128, 0, 255, 255);
}  // namespace color

struct TileInfp {
    ImU32 color;
    const std::string name;
};

void TilePanel::set_button(const std::string& iId, ImU32 color) {
    ImGui::PushStyleColor(ImGuiCol_Button, color);         // Normal
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);  // Hover
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);   // Clicked
    ImVec2 button_size(32, 32);  // Set a fixed size for the buttons

    if (ImGui::Button(iId.c_str(), button_size)) { /* select brick */
        cur_color = color;      // Set current color to red
    }
    ImGui::PopStyleColor(3);  // Restore the 3 pushed colors
}

void TilePanel::draw() {
    ImGui::Begin("Tiles Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Tile Picker");
    ImGui::Separator();

    std::vector<TileInfp> tiles = {{color::kColorRed, "Brick"},
                                   {color::kColorGreen, "Grass"},
                                   {color::kColorBlue, "Water"},
                                   {color::kColorOrange, "Sand"},
                                   {color::kColorLightGreen, "Lava"},
                                   {color::kColorPurple, "Stone"}};
    int idx = 1;
    for (const auto& tile : tiles) {
        set_button(tile.name, tile.color);
        if (idx % 3 != 0) {
            ImGui::SameLine();
        }
        ++idx;
    }

    ImGui::End();
}