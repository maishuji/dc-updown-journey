// Copyright 2025 Quentin Cartier
#include "udjourney-editor/EditorPanel.hpp"

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <memory>
#include <string>
#include <vector>
#include <cstring>

#include "udjourney-editor/Level.hpp"
#include "udjourney-editor/mode_handlers/TileModeHandler.hpp"
#include "udjourney-editor/mode_handlers/PlatformModeHandler.hpp"
#include "udjourney-editor/mode_handlers/SpawnModeHandler.hpp"
#include "udjourney-editor/mode_handlers/MonsterModeHandler.hpp"
#include "udjourney-editor/mode_handlers/BackgroundModeHandler.hpp"
#include "udjourney-editor/mode_handlers/HUDModeHandler.hpp"

namespace color {
const ImU32 kColorRed = IM_COL32(255, 0, 0, 255);
const ImU32 kColorGreen = IM_COL32(0, 255, 0, 255);
const ImU32 kColorBlue = IM_COL32(0, 0, 255, 255);
const ImU32 kColorOrange = IM_COL32(255, 128, 0, 255);
const ImU32 kColorLightGreen = IM_COL32(0, 255, 128, 255);
const ImU32 kColorPurple = IM_COL32(128, 0, 255, 255);
}  // namespace color

EditorPanel::EditorPanel() {
    // Initialize handlers
    tile_handler_ = std::make_unique<TileModeHandler>();
    platform_handler_ = std::make_unique<PlatformModeHandler>();
    spawn_handler_ = std::make_unique<SpawnModeHandler>();
    monster_handler_ = std::make_unique<MonsterModeHandler>();
    fud_handler_ = std::make_unique<HUDModeHandler>();
    // Background handler will be created when managers are set
}

// Destructor must be defined in .cpp where handler types are complete
EditorPanel::~EditorPanel() = default;

void EditorPanel::set_background_managers(
    BackgroundManager* bg_manager,
    BackgroundObjectPresetManager* preset_manager) {
    background_manager_ = bg_manager;
    background_preset_manager_ = preset_manager;

    // Create background handler now that we have the managers
    if (bg_manager && preset_manager) {
        background_handler_ =
            std::make_unique<BackgroundModeHandler>(bg_manager, preset_manager);
    }
}

struct TileInfp {
    ImU32 color;
    const std::string name;
};

void EditorPanel::set_button(const std::string& iId, ImU32 color) {
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

void EditorPanel::set_selected_platform(EditorPlatform* platform) {
    selected_platform_ = platform;
    if (platform_handler_) {
        platform_handler_->set_selected_platform(platform);
    }
}

void EditorPanel::set_scale(float scale) noexcept {
    this->scale = scale;
    if (tile_handler_) tile_handler_->set_scale(scale);
    if (platform_handler_) platform_handler_->set_scale(scale);
    if (spawn_handler_) spawn_handler_->set_scale(scale);
    if (monster_handler_) monster_handler_->set_scale(scale);
    if (background_handler_) background_handler_->set_scale(scale);
    if (fud_handler_) fud_handler_->set_scale(scale);
}

void EditorPanel::set_selected_monster(EditorMonster* monster) {
    selected_monster_ = monster;
    if (monster_handler_) {
        monster_handler_->set_selected_monster(monster);
    }
}

bool EditorPanel::is_background_placing_mode() const {
    if (background_handler_) return background_handler_->is_placing_mode();
    return background_placing_mode_;
}

int EditorPanel::get_selected_background_preset_idx() const {
    if (background_handler_)
        return background_handler_->get_selected_preset_idx();
    return selected_preset_idx_;
}

float EditorPanel::get_background_object_scale() const {
    if (background_handler_) return background_handler_->get_object_scale();
    return new_bg_object_scale_;
}

void EditorPanel::clear_background_placing_mode() {
    if (background_handler_) {
        background_handler_->clear_placing_mode();
    }
    background_placing_mode_ = false;
    selected_preset_idx_ = -1;
}

EditorMonster* EditorPanel::get_selected_monster() const {
    if (monster_handler_) return monster_handler_->get_selected_monster();
    return selected_monster_;
}

const std::string& EditorPanel::get_selected_monster_preset() const {
    if (monster_handler_)
        return monster_handler_->get_selected_monster_preset();
    return selected_monster_preset;
}

bool EditorPanel::should_delete_selected_monster() const {
    if (monster_handler_)
        return monster_handler_->should_delete_selected_monster();
    return delete_selected_monster_;
}

void EditorPanel::clear_delete_flag() {
    if (monster_handler_) {
        monster_handler_->clear_delete_flag();
    }
    delete_selected_monster_ = false;
    selected_monster_ = nullptr;
}

void EditorPanel::draw() {
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

    // Show scene type indicator
    if (current_level_) {
        ImGui::Separator();
        if (current_level_->scene_type == SceneType::LEVEL) {
            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f),
                               "Scene: LEVEL (Gameplay)");
            ImGui::Text("All modes available");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                               "Scene: UI SCREEN (Menu)");
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                               "Platforms/Monsters/Spawn disabled");
        }
    }
    ImGui::Separator();

    // Mode selection - disable some modes for UI screens
    bool is_ui_screen =
        current_level_ && current_level_->scene_type == SceneType::UI_SCREEN;

    ImGui::Text("Edit Mode:");

    // Tiles mode - only for levels
    ImGui::BeginDisabled(is_ui_screen);
    if (ImGui::RadioButton("Tiles", edit_mode == EditMode::Tiles)) {
        edit_mode = EditMode::Tiles;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Platforms", edit_mode == EditMode::Platforms)) {
        edit_mode = EditMode::Platforms;
    }
    ImGui::EndDisabled();

    // Spawn mode - only for levels
    ImGui::BeginDisabled(is_ui_screen);
    if (ImGui::RadioButton("Spawn", edit_mode == EditMode::PlayerSpawn)) {
        edit_mode = EditMode::PlayerSpawn;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Monsters", edit_mode == EditMode::Monsters)) {
        edit_mode = EditMode::Monsters;
    }
    ImGui::EndDisabled();

    if (ImGui::RadioButton("Background", edit_mode == EditMode::Background)) {
        edit_mode = EditMode::Background;
    }

    ImGui::SameLine();
    if (ImGui::RadioButton("HUD", edit_mode == EditMode::HUD)) {
        edit_mode = EditMode::HUD;
    }

    ImGui::Separator();

    // Draw mode-specific UI
    switch (edit_mode) {
        case EditMode::Tiles:
            if (tile_handler_) {
                tile_handler_->render();
                cur_color = tile_handler_->get_current_color();
            }
            break;
        case EditMode::Platforms:
            if (platform_handler_) {
                platform_handler_->render();
                platform_behavior = platform_handler_->get_platform_behavior();
                platform_size = platform_handler_->get_platform_size();
                feature_spikes =
                    !platform_handler_->get_selected_features().empty();
                selected_platform_ = platform_handler_->get_selected_platform();
            }
            break;
        case EditMode::PlayerSpawn:
            if (spawn_handler_) {
                spawn_handler_->render();
            }
            break;
        case EditMode::Monsters:
            if (monster_handler_) {
                monster_handler_->render();
                // Sync state back to EditorPanel for backward compatibility
                selected_monster_preset =
                    monster_handler_->get_selected_monster_preset();
                selected_monster_ = monster_handler_->get_selected_monster();
                delete_selected_monster_ =
                    monster_handler_->should_delete_selected_monster();
            } else {
                draw_monsters_mode();  // Fallback to legacy code
            }
            break;
        case EditMode::Background:
            if (background_handler_) {
                background_handler_->render();
                background_placing_mode_ =
                    background_handler_->is_placing_mode();
                selected_preset_idx_ =
                    background_handler_->get_selected_preset_idx();
                new_bg_object_scale_ = background_handler_->get_object_scale();
            }
            break;
        case EditMode::HUD:
            if (fud_handler_) {
                fud_handler_->render();
            }
            break;
    }

    ImGui::End();
}

ImVec2 EditorPanel::get_platform_size() const noexcept { return platform_size; }

void EditorPanel::draw_tile_mode() {
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
void EditorPanel::draw_platform_mode() {
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
void EditorPanel::draw_platform_editor() {
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

void EditorPanel::draw_spawn_mode() {
    ImGui::Text("Player Spawn");
    ImGui::Separator();
    ImGui::Text("Click on the grid to set");
    ImGui::Text("the player spawn position.");
}

void EditorPanel::draw_monsters_mode() {
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

void EditorPanel::draw_monster_editor() {
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

std::vector<PlatformFeatureType> EditorPanel::get_selected_features() const {
    std::vector<PlatformFeatureType> features;
    if (feature_spikes) {
        features.push_back(PlatformFeatureType::Spikes);
    }
    if (feature_checkpoint) {
        features.push_back(PlatformFeatureType::Checkpoint);
    }
    return features;
}

void EditorPanel::initialize_monster_presets() {
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

// HUD editing methods
void EditorPanel::set_selected_fud(HUDElement* hud) {
    if (fud_handler_) {
        fud_handler_->set_selected_fud(hud);
    }
}

HUDElement* EditorPanel::get_selected_fud() const {
    if (fud_handler_) {
        return fud_handler_->get_selected_fud();
    }
    return nullptr;
}

const std::string& EditorPanel::get_selected_fud_preset() const {
    if (fud_handler_) {
        return fud_handler_->get_selected_fud_preset();
    }
    static std::string empty;
    return empty;
}

bool EditorPanel::should_delete_selected_fud() const {
    if (fud_handler_) {
        return fud_handler_->should_delete_selected_fud();
    }
    return false;
}

void EditorPanel::clear_fud_delete_flag() {
    if (fud_handler_) {
        fud_handler_->clear_delete_flag();
    }
}

bool EditorPanel::should_add_fud() const {
    if (fud_handler_) {
        return fud_handler_->should_add_fud();
    }
    return false;
}

void EditorPanel::clear_fud_add_flag() {
    if (fud_handler_) {
        fud_handler_->clear_add_flag();
    }
}

HUDElement EditorPanel::create_fud_from_preset() const {
    if (fud_handler_) {
        return fud_handler_->create_fud_from_preset();
    }
    // Return default HUD if handler not available
    return HUDElement("New FUD",
                      "unknown",
                      HUDAnchor::TopLeft,
                      ImVec2(10, 10),
                      ImVec2(100, 30));
}
