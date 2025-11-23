// Copyright 2025 Quentin Cartier
#include "udjourney-editor/mode_handlers/MonsterModeHandler.hpp"

#include <imgui.h>
#include <functional>

MonsterModeHandler::MonsterModeHandler() {
    // Load presets on construction
    monster_preset_manager_.load_available_presets();
}

void MonsterModeHandler::render() {
    // Ensure we have valid monster presets loaded and selected
    initialize_presets();

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
                               selected_monster_preset_ == preset.name)) {
            selected_monster_preset_ = preset.name;
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
    ImGui::Text("Left click: Place %s", selected_monster_preset_.c_str());
    ImGui::Text("Right click: Remove monster");
    ImGui::Text("Click existing monster to edit");

    // Show reload button for development
    if (ImGui::Button("Reload Presets")) {
        monster_preset_manager_.load_available_presets();
        // Validate current selection still exists
        if (monster_preset_manager_.get_preset(selected_monster_preset_) ==
            nullptr) {
            auto preset_names = monster_preset_manager_.get_preset_names();
            if (!preset_names.empty()) {
                selected_monster_preset_ = preset_names[0];
            }
        }
    }

    // Monster editor for selected monster
    if (selected_monster_) {
        render_monster_editor();
    }
}

void MonsterModeHandler::initialize_presets() {
    // Ensure we have a valid selected preset
    if (monster_preset_manager_.has_presets()) {
        auto preset_names = monster_preset_manager_.get_preset_names();
        if (!preset_names.empty()) {
            // Check if current selection is valid
            bool found = false;
            for (const auto& name : preset_names) {
                if (name == selected_monster_preset_) {
                    found = true;
                    break;
                }
            }
            // If current selection is invalid, pick the first available preset
            if (!found) {
                selected_monster_preset_ = preset_names[0];
            }
        }
    }
}

void MonsterModeHandler::render_monster_editor() {
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
