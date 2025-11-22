// Copyright 2025 Quentin Cartier
#include "udjourney-editor/TilePanel.hpp"

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <string>
#include <vector>
#include <cstring>

#include "udjourney-editor/Level.hpp"

namespace color {
const ImU32 kColorRed = IM_COL32(255, 0, 0, 255);
const ImU32 kColorGreen = IM_COL32(0, 255, 0, 255);
const ImU32 kColorBlue = IM_COL32(0, 0, 255, 255);
const ImU32 kColorOrange = IM_COL32(255, 128, 0, 255);
const ImU32 kColorLightGreen = IM_COL32(0, 255, 128, 255);
const ImU32 kColorPurple = IM_COL32(128, 0, 255, 255);
}  // namespace color

TilePanel::TilePanel() {}

void TilePanel::set_background_managers(
    BackgroundManager* bg_manager,
    BackgroundObjectPresetManager* preset_manager) {
    background_manager_ = bg_manager;
    background_preset_manager_ = preset_manager;
}

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
    // ImGui::SameLine();
    if (ImGui::RadioButton("Spawn", edit_mode == EditMode::PlayerSpawn)) {
        edit_mode = EditMode::PlayerSpawn;
    }

    ImGui::SameLine();
    if (ImGui::RadioButton("Monsters", edit_mode == EditMode::Monsters)) {
        edit_mode = EditMode::Monsters;
    }

    if (ImGui::RadioButton("Background", edit_mode == EditMode::Background)) {
        edit_mode = EditMode::Background;
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
        case EditMode::Background:
            draw_background_mode();
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
    // Ensure we have valid monster presets loaded and selected
    initialize_monster_presets();

    ImGui::Text("Monster Spawns");
    ImGui::Separator();

    // Check if we have any monster presets loaded
    if (!monster_preset_manager_.has_presets()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                           "No monster presets found!");
        ImGui::Text("Make sure assets/monsters/ contains");
        ImGui::Text("valid monster preset JSON files.");

        if (ImGui::Button("Reload Presets")) {
            monster_preset_manager_.load_available_presets();
        }
        return;
    }

    // Dynamic monster preset selection
    ImGui::Text("Select Monster Type:");

    const auto& presets = monster_preset_manager_.get_presets();
    for (const auto& preset : presets) {
        if (ImGui::RadioButton(preset.display_name.c_str(),
                               selected_monster_preset == preset.name)) {
            selected_monster_preset = preset.name;
        }

        // Show preset information on hover
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Name: %s", preset.display_name.c_str());
            ImGui::Text("Health: %d", preset.health);
            ImGui::Text("Speed: %d", preset.speed);
            if (!preset.description.empty()) {
                ImGui::Separator();
                ImGui::TextWrapped("%s", preset.description.c_str());
            }
            ImGui::EndTooltip();
        }
    }

    ImGui::Separator();
    ImGui::Text("Left click: Place %s", selected_monster_preset.c_str());
    ImGui::Text("Right click: Remove monster");
    ImGui::Text("Click existing monster to edit");

    // Show reload button for development
    if (ImGui::Button("Reload Presets")) {
        monster_preset_manager_.load_available_presets();
        // Validate current selection still exists
        if (monster_preset_manager_.get_preset(selected_monster_preset) ==
            nullptr) {
            auto preset_names = monster_preset_manager_.get_preset_names();
            if (!preset_names.empty()) {
                selected_monster_preset = preset_names[0];
            }
        }
    }

    // Monster editor for selected monster
    if (selected_monster_) {
        draw_monster_editor();
    }
}

void TilePanel::draw_monster_editor() {
    if (!selected_monster_) return;

    ImGui::Separator();
    ImGui::Text("Editing Monster at (%d, %d)",
                selected_monster_->tile_x,
                selected_monster_->tile_y);

    // Dynamic preset selection for existing monster
    ImGui::Text("Monster Type:");
    bool preset_changed = false;

    const auto& presets = monster_preset_manager_.get_presets();
    for (const auto& preset : presets) {
        std::string radio_id = preset.display_name + "##edit";
        if (ImGui::RadioButton(radio_id.c_str(),
                               selected_monster_->preset_name == preset.name)) {
            selected_monster_->preset_name = preset.name;
            preset_changed = true;
        }

        // Show preset information on hover
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Name: %s", preset.display_name.c_str());
            ImGui::Text("Health: %d", preset.health);
            ImGui::Text("Speed: %d", preset.speed);
            if (!preset.description.empty()) {
                ImGui::Separator();
                ImGui::TextWrapped("%s", preset.description.c_str());
            }
            ImGui::EndTooltip();
        }
    }

    // Update color based on preset (using a color mapping system)
    if (preset_changed) {
        // Generate a consistent color based on preset name hash
        std::hash<std::string> hasher;
        size_t hash = hasher(selected_monster_->preset_name);

        // Use hash to generate RGB values with good contrast
        ImU8 r = 128 + (hash & 0xFF) / 2;          // 128-255 range
        ImU8 g = 128 + ((hash >> 8) & 0xFF) / 2;   // 128-255 range
        ImU8 b = 128 + ((hash >> 16) & 0xFF) / 2;  // 128-255 range

        selected_monster_->color = IM_COL32(r, g, b, 255);
    }

    // Show current preset info
    const auto* current_preset =
        monster_preset_manager_.get_preset(selected_monster_->preset_name);
    if (current_preset) {
        ImGui::Separator();
        ImGui::Text("Preset Info:");
        ImGui::Text("Health: %d", current_preset->health);
        ImGui::Text("Speed: %d", current_preset->speed);
        if (!current_preset->description.empty()) {
            ImGui::TextWrapped("Description: %s",
                               current_preset->description.c_str());
        }
    }

    // Position info (read-only for now)
    ImGui::Separator();
    ImGui::Text("Position: Tile (%d, %d)",
                selected_monster_->tile_x,
                selected_monster_->tile_y);

    // Option to delete monster
    if (ImGui::Button("Delete Monster")) {
        delete_selected_monster_ = true;  // Flag for deletion
    }
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

void TilePanel::initialize_monster_presets() {
    // Ensure we have a valid selected preset
    if (monster_preset_manager_.has_presets()) {
        auto preset_names = monster_preset_manager_.get_preset_names();
        if (!preset_names.empty()) {
            // Check if current selection is valid
            bool found = false;
            for (const auto& name : preset_names) {
                if (name == selected_monster_preset) {
                    found = true;
                    break;
                }
            }
            // If current selection is invalid, pick the first available preset
            if (!found) {
                selected_monster_preset = preset_names[0];
            }
        }
    }
}

void TilePanel::draw_background_mode() {
    if (!background_manager_ || !background_preset_manager_) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1),
                           "Background system not initialized");
        return;
    }

    ImGui::Text("Background Layers");
    ImGui::Text("Layers: %zu / %d",
                background_manager_->get_layer_count(),
                BackgroundManager::MAX_LAYERS);
    ImGui::Separator();

    // Add layer controls
    if (background_manager_->can_add_layer()) {
        ImGui::Text("Add New Layer:");
        ImGui::InputText("Name", new_layer_name_, sizeof(new_layer_name_));
        ImGui::SliderFloat("Parallax", &new_layer_parallax_, 0.0f, 1.0f);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                           "(0.0 = static, 1.0 = moves with camera)");
        ImGui::InputInt("Depth", &new_layer_depth_);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                           "(Lower = rendered behind)");

        if (ImGui::Button("Add Layer")) {
            BackgroundLayer layer(
                new_layer_name_, new_layer_parallax_, new_layer_depth_);
            if (background_manager_->add_layer(layer)) {
                std::snprintf(
                    new_layer_name_, sizeof(new_layer_name_), "New Layer");
                new_layer_parallax_ = 0.5f;
                new_layer_depth_ = 0;
            }
        }
        ImGui::Separator();
    } else {
        ImGui::TextColored(ImVec4(1, 0.5f, 0, 1),
                           "Maximum layers reached (5/5)");
        ImGui::Separator();
    }

    // Layer list
    ImGui::Text("Layers:");
    const auto& layers = background_manager_->get_layers();
    auto selected = background_manager_->get_selected_layer();

    for (size_t i = 0; i < layers.size(); ++i) {
        bool is_selected = selected.has_value() && selected.value() == i;

        ImGui::PushID(static_cast<int>(i));

        // Layer name with selection
        if (ImGui::Selectable(layers[i].get_name().c_str(), is_selected)) {
            background_manager_->select_layer(i);
        }

        // Layer info and controls on same line
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                           "(P:%.1f D:%d)",
                           layers[i].get_parallax_factor(),
                           layers[i].get_depth());

        // Move and delete buttons
        ImGui::SameLine();
        if (ImGui::SmallButton("^") && i > 0) {
            background_manager_->move_layer_up(i);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("v") && i < layers.size() - 1) {
            background_manager_->move_layer_down(i);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("X")) {
            background_manager_->remove_layer(i);
            if (selected.has_value() && selected.value() == i) {
                background_manager_->clear_selection();
            }
        }

        ImGui::PopID();
    }

    if (layers.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No layers yet");
    }

    ImGui::Separator();

    // Add object controls - only if layer is selected
    if (!selected.has_value()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                           "Select a layer to add objects");
        background_placing_mode_ = false;
    } else {
        ImGui::Text("Add Objects to: %s",
                    layers[selected.value()].get_name().c_str());

        // Show available presets as clickable list
        if (background_preset_manager_->has_presets()) {
            const auto& presets = background_preset_manager_->get_presets();

            ImGui::Text("Available Objects:");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                               "(Click to select, then click in scene)");

            for (size_t i = 0; i < presets.size(); ++i) {
                ImGui::PushID(static_cast<int>(i + 500));

                bool is_selected =
                    (selected_preset_idx_ == static_cast<int>(i)) &&
                    background_placing_mode_;

                // Highlight selected preset
                if (is_selected) {
                    ImGui::PushStyleColor(ImGuiCol_Button,
                                          ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                }

                if (ImGui::Button(presets[i].name.c_str(), ImVec2(-1, 0))) {
                    selected_preset_idx_ = static_cast<int>(i);
                    new_bg_object_scale_ = presets[i].default_scale;
                    background_placing_mode_ = true;
                }

                if (is_selected) {
                    ImGui::PopStyleColor();
                }

                ImGui::PopID();
            }

            // Scale slider for current selection
            if (background_placing_mode_) {
                ImGui::Separator();
                ImGui::Text("Placing: %s",
                            presets[selected_preset_idx_].name.c_str());
                ImGui::SliderFloat("Scale", &new_bg_object_scale_, 0.1f, 5.0f);
                if (ImGui::Button("Cancel Placement")) {
                    background_placing_mode_ = false;
                }
            }
        } else {
            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "No presets loaded");
            background_placing_mode_ = false;
        }

        ImGui::Separator();

        // Show objects in selected layer
        ImGui::Text("Objects in Layer:");
        const auto& objects = layers[selected.value()].get_objects();
        if (objects.empty()) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                               "No objects yet");
        } else {
            for (size_t i = 0; i < objects.size(); ++i) {
                ImGui::PushID(static_cast<int>(i + 1000));
                ImGui::Text("%zu: %s", i, objects[i].sprite_name.c_str());
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                                   "(%.0f, %.0f) x%.1f",
                                   objects[i].x,
                                   objects[i].y,
                                   objects[i].scale);
                ImGui::SameLine();
                if (ImGui::SmallButton("X")) {
                    background_manager_->remove_object(selected.value(), i);
                }
                ImGui::PopID();
            }
        }
    }
}
