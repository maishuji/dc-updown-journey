// Copyright 2025 Quentin Cartier
#include "udjourney-editor/TilePanel.hpp"

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <string>
#include <vector>

#include "udjourney-editor/Level.hpp"

namespace color {
const ImU32 kColorRed = IM_COL32(255, 0, 0, 255);
const ImU32 kColorGreen = IM_COL32(0, 255, 0, 255);
const ImU32 kColorBlue = IM_COL32(0, 0, 255, 255);
const ImU32 kColorOrange = IM_COL32(255, 128, 0, 255);
const ImU32 kColorLightGreen = IM_COL32(0, 255, 128, 255);
const ImU32 kColorPurple = IM_COL32(128, 0, 255, 255);
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

    if (ImGui::Button(iId.c_str(),
                      ImVec2(button_size.x * scale,
                             button_size.y * scale))) { /* select brick */
        cur_color = color;  // Set current color to red
    }
    ImGui::PopStyleColor(3);  // Restore the 3 pushed colors
}

void TilePanel::draw() {
    // Handle focus request
    if (should_focus_) {
        ImGui::SetNextWindowFocus();
        should_focus_ = false;
    }

    // Remove manual positioning - docking handles it automatically
    // Set minimum size constraints - allow unlimited maximum for docking
    // flexibility
    ImGui::SetNextWindowSizeConstraints(ImVec2(250, 300),
                                        ImVec2(FLT_MAX, FLT_MAX));

    bool window_open = true;
    // Enable horizontal scrollbar for text that doesn't fit
    if (!ImGui::Begin("Editor Panel",
                      &window_open,
                      ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::End();
        return;
    }

    // Show help text at the top
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow)) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                           "Panel Focused ✓ (F1: refocus)");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                           "Press F1 to focus this panel");
    }
    ImGui::Separator();

    // Mode selection
    ImGui::Text("Edit Mode:");
    if (ImGui::RadioButton("Tiles", edit_mode == EditMode::Tiles)) {
        edit_mode = EditMode::Tiles;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Platforms", edit_mode == EditMode::Platforms)) {
        edit_mode = EditMode::Platforms;
    }
    //ImGui::SameLine();
    if (ImGui::RadioButton("Spawn",
                           edit_mode == EditMode::PlayerSpawn)) {
        edit_mode = EditMode::PlayerSpawn;
    }

    ImGui::SameLine();
    if (ImGui::RadioButton("Monsters",
                           edit_mode == EditMode::Monsters)) {
        edit_mode = EditMode::Monsters;
    }

    ImGui::Separator();

    // Draw mode-specific UI
    switch (edit_mode) {
        case EditMode::Tiles:
            draw_tile_mode();
            break;
        case EditMode::Platforms:
            draw_platform_mode();
            break;
        case EditMode::PlayerSpawn:
            draw_spawn_mode();
            break;
        case EditMode::Monsters:
            draw_monsters_mode();
            break;
    }

    ImGui::End();
}

ImVec2 TilePanel::get_platform_size() const noexcept { return platform_size; }

void TilePanel::draw_tile_mode() {
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
}

/*
 * Draw the platform creation UI when no platform is selected
 */
void TilePanel::draw_platform_mode() {
    if (selected_platform_) {
        draw_platform_editor();
    } else {
        ImGui::Text("Platform Creator");
        ImGui::Separator();

        // Platform behavior selection for new platforms
        ImGui::TextWrapped("Behavior for new platforms:");
        if (ImGui::RadioButton(
                "Static", platform_behavior == PlatformBehaviorType::Static)) {
            platform_behavior = PlatformBehaviorType::Static;
        }
        if (ImGui::RadioButton(
                "Horizontal",
                platform_behavior == PlatformBehaviorType::Horizontal)) {
            platform_behavior = PlatformBehaviorType::Horizontal;
        }
        if (ImGui::RadioButton("Eight Turn",
                               platform_behavior ==
                                   PlatformBehaviorType::EightTurnHorizontal)) {
            platform_behavior = PlatformBehaviorType::EightTurnHorizontal;
        }
        if (ImGui::RadioButton(
                "Oscillating Size",
                platform_behavior == PlatformBehaviorType::OscillatingSize)) {
            platform_behavior = PlatformBehaviorType::OscillatingSize;
        }

        if (ImGui::CollapsingHeader("Size", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat(
                "Width", &platform_size.x, 0.5f, 5.0f, "%.1f tiles");
            ImGui::SliderFloat(
                "Height", &platform_size.y, 0.5f, 3.0f, "%.1f tiles");
        }

        ImGui::Separator();
        ImGui::TextWrapped("Features for new platforms:");
        ImGui::Checkbox("Spikes", &feature_spikes);
        ImGui::Checkbox("Checkpoint", &feature_checkpoint);

        ImGui::Separator();
        ImGui::TextWrapped("Controls:");
        ImGui::BulletText("Left click: Create platform");
        ImGui::BulletText("Right click: Delete platform");
        ImGui::BulletText("Ctrl+Left click: Edit platform");
    }
}

/**
 * Draw the platform editor UI when a platform is selected
 */
void TilePanel::draw_platform_editor() {
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Platform Editor");
    ImGui::Separator();

    if (!selected_platform_) return;

    // Show platform position
    ImGui::Text("Position: (%d, %d)",
                selected_platform_->tile_x,
                selected_platform_->tile_y);

    // Edit platform behavior in a collapsible section
    if (ImGui::CollapsingHeader("Behavior", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::RadioButton("Static##edit",
                               selected_platform_->behavior_type ==
                                   PlatformBehaviorType::Static)) {
            selected_platform_->behavior_type = PlatformBehaviorType::Static;
        }
        if (ImGui::RadioButton("Horizontal##edit",
                               selected_platform_->behavior_type ==
                                   PlatformBehaviorType::Horizontal)) {
            selected_platform_->behavior_type =
                PlatformBehaviorType::Horizontal;
        }
        if (ImGui::RadioButton("Eight Turn##edit",
                               selected_platform_->behavior_type ==
                                   PlatformBehaviorType::EightTurnHorizontal)) {
            selected_platform_->behavior_type =
                PlatformBehaviorType::EightTurnHorizontal;
        }
        if (ImGui::RadioButton("Oscillating Size##edit",
                               selected_platform_->behavior_type ==
                                   PlatformBehaviorType::OscillatingSize)) {
            selected_platform_->behavior_type =
                PlatformBehaviorType::OscillatingSize;
        }
    }

    if (ImGui::CollapsingHeader("Size", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Width",
                           &selected_platform_->width_tiles,
                           0.5f,
                           5.0f,
                           "%.1f tiles");
        ImGui::SliderFloat("Height",
                           &selected_platform_->height_tiles,
                           0.5f,
                           3.0f,
                           "%.1f tiles");
    }

    if (ImGui::CollapsingHeader("Features", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Check current features
        bool has_spikes = std::find(selected_platform_->features.begin(),
                                    selected_platform_->features.end(),
                                    PlatformFeatureType::Spikes) !=
                          selected_platform_->features.end();
        bool has_checkpoint = std::find(selected_platform_->features.begin(),
                                        selected_platform_->features.end(),
                                        PlatformFeatureType::Checkpoint) !=
                              selected_platform_->features.end();

        // Feature checkboxes
        if (ImGui::Checkbox("Spikes##edit", &has_spikes)) {
            if (has_spikes) {
                // Add spikes feature
                if (std::find(selected_platform_->features.begin(),
                              selected_platform_->features.end(),
                              PlatformFeatureType::Spikes) ==
                    selected_platform_->features.end()) {
                    selected_platform_->features.push_back(
                        PlatformFeatureType::Spikes);
                }
            } else {
                // Remove spikes feature
                selected_platform_->features.erase(
                    std::remove(selected_platform_->features.begin(),
                                selected_platform_->features.end(),
                                PlatformFeatureType::Spikes),
                    selected_platform_->features.end());
            }
        }

        if (ImGui::Checkbox("Checkpoint##edit", &has_checkpoint)) {
            if (has_checkpoint) {
                // Add checkpoint feature
                if (std::find(selected_platform_->features.begin(),
                              selected_platform_->features.end(),
                              PlatformFeatureType::Checkpoint) ==
                    selected_platform_->features.end()) {
                    selected_platform_->features.push_back(
                        PlatformFeatureType::Checkpoint);
                }
            } else {
                // Remove checkpoint feature
                selected_platform_->features.erase(
                    std::remove(selected_platform_->features.begin(),
                                selected_platform_->features.end(),
                                PlatformFeatureType::Checkpoint),
                    selected_platform_->features.end());
            }
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Done Editing", ImVec2(-1, 0))) {  // Full width button
        selected_platform_ = nullptr;
    }
}

void TilePanel::draw_spawn_mode() {
    ImGui::Text("Player Spawn");
    ImGui::Separator();
    ImGui::Text("Click on the grid to set");
    ImGui::Text("the player spawn position.");
}

void TilePanel::draw_monsters_mode() {
    ImGui::Text("Monster Spawns");
    ImGui::Separator();
    ImGui::Text("Click on the grid to set");
    ImGui::Text("the monster spawn positions.");
}

std::vector<PlatformFeatureType> TilePanel::get_selected_features() const {
    std::vector<PlatformFeatureType> features;
    if (feature_spikes) {
        features.push_back(PlatformFeatureType::Spikes);
    }
    if (feature_checkpoint) {
        features.push_back(PlatformFeatureType::Checkpoint);
    }
    return features;
}
