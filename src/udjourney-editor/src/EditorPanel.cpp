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

void EditorPanel::render_file_dialogs() {
    if (platform_handler_) {
        platform_handler_->render_file_dialogs();
    }
}

const std::string& EditorPanel::get_new_platform_texture_file() const {
    static const std::string kEmpty;
    if (platform_handler_) {
        return platform_handler_->get_new_platform_texture_file();
    }
    return kEmpty;
}

bool EditorPanel::get_new_platform_texture_tiled() const noexcept {
    if (platform_handler_) {
        return platform_handler_->get_new_platform_texture_tiled();
    }
    return false;
}

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
