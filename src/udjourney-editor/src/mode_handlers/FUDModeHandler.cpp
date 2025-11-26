// Copyright 2025 Quentin Cartier
#include "udjourney-editor/mode_handlers/FUDModeHandler.hpp"

#include <imgui.h>
#include <string>
#include <vector>

FUDModeHandler::FUDModeHandler() {
    // Load presets on construction
    fud_preset_manager_.load_available_presets();
}

void FUDModeHandler::initialize_presets() {
    if (fud_preset_manager_.has_presets() && selected_fud_preset_.empty()) {
        // Select first preset by default
        const auto& presets = fud_preset_manager_.get_presets();
        if (!presets.empty()) {
            selected_fud_preset_ = presets[0].type_id;
        }
    }
}

void FUDModeHandler::render() {
    // Ensure we have valid FUD presets loaded and selected
    initialize_presets();

    ImGui::Text("FUD Elements");
    ImGui::Separator();

    render_fud_preset_selector();

    ImGui::Spacing();

    if (selected_fud_) {
        render_fud_properties();
    } else {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                           "Click on a FUD element to edit");
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                           "or use 'Add FUD' button");
    }
}

void FUDModeHandler::render_fud_preset_selector() {
    const auto& presets = fud_preset_manager_.get_presets();

    if (presets.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                           "No FUD presets found!");
        ImGui::TextWrapped("Create FUD preset files in assets/fuds/");
        return;
    }

    ImGui::Text("FUD Type:");

    // Dropdown for FUD preset selection
    if (ImGui::BeginCombo("##FUDPreset", selected_fud_preset_.c_str())) {
        for (const auto& preset : presets) {
            bool is_selected = (selected_fud_preset_ == preset.type_id);
            std::string label = preset.display_name + " (" +
                                fud_category_to_string(preset.category) + ")";

            if (ImGui::Selectable(label.c_str(), is_selected)) {
                selected_fud_preset_ = preset.type_id;
            }

            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }

            // Show description as tooltip
            if (!preset.description.empty() && ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", preset.description.c_str());
            }
        }
        ImGui::EndCombo();
    }

    // Show preset info
    const FUDPreset* current_preset =
        fud_preset_manager_.get_preset(selected_fud_preset_);
    if (current_preset) {
        ImGui::TextColored(
            ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
            "Category: %s",
            fud_category_to_string(current_preset->category).c_str());
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                           "Default Size: %.0fx%.0f px",
                           current_preset->default_size.x,
                           current_preset->default_size.y);
    }

    ImGui::Spacing();

    // Add FUD button
    if (ImGui::Button("Add FUD", ImVec2(150, 0))) {
        add_fud_requested_ = true;
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Add a new FUD element to the level");
    }
}

void FUDModeHandler::render_fud_properties() {
    if (!selected_fud_) {
        return;
    }

    ImGui::Separator();
    ImGui::Text("Selected FUD");

    // Name
    char name_buf[256];
    strncpy(name_buf, selected_fud_->name.c_str(), sizeof(name_buf) - 1);
    name_buf[sizeof(name_buf) - 1] = '\0';

    if (ImGui::InputText("Name", name_buf, sizeof(name_buf))) {
        selected_fud_->name = name_buf;
    }

    // Type (read-only)
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                       "Type: %s",
                       selected_fud_->type_id.c_str());

    // Anchor position
    ImGui::Text("Anchor:");
    const char* anchor_names[] = {"TopLeft",
                                  "TopCenter",
                                  "TopRight",
                                  "MiddleLeft",
                                  "MiddleCenter",
                                  "MiddleRight",
                                  "BottomLeft",
                                  "BottomCenter",
                                  "BottomRight"};
    int current_anchor = static_cast<int>(selected_fud_->anchor);

    if (ImGui::Combo("##Anchor", &current_anchor, anchor_names, 9)) {
        selected_fud_->anchor = static_cast<FUDAnchor>(current_anchor);
    }

    // Offset
    ImGui::DragFloat2("Offset (px)", &selected_fud_->offset.x, 1.0f);

    // Size
    ImGui::DragFloat2(
        "Size (px)", &selected_fud_->size.x, 1.0f, 10.0f, 1000.0f);

    // Visibility
    ImGui::Checkbox("Visible", &selected_fud_->visible);

    ImGui::Spacing();

    // Properties (JSON editor - simplified)
    if (ImGui::CollapsingHeader("Custom Properties")) {
        ImGui::TextWrapped("Properties: %s",
                           selected_fud_->properties.dump(2).c_str());

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
                "Custom properties are loaded from preset schema");
        }

        // TODO: Add property editor based on schema from preset
    }

    ImGui::Spacing();

    // Delete button
    if (ImGui::Button("Delete FUD", ImVec2(150, 0))) {
        delete_selected_fud_ = true;
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Remove this FUD element from the level");
    }
}

FUDElement FUDModeHandler::create_fud_from_preset() const {
    const FUDPreset* preset =
        fud_preset_manager_.get_preset(selected_fud_preset_);

    if (!preset) {
        // Return default FUD if preset not found
        return FUDElement("New FUD",
                          "unknown",
                          FUDAnchor::TopLeft,
                          ImVec2(10, 10),
                          ImVec2(100, 30));
    }

    // Create FUD with preset defaults
    FUDElement fud;
    fud.name = preset->display_name;
    fud.type_id = preset->type_id;
    fud.anchor = preset->default_anchor;
    fud.offset = ImVec2(10, 10);  // Small offset from anchor
    fud.size = preset->default_size;
    fud.visible = true;

    // Initialize properties from schema defaults
    fud.properties = nlohmann::json::object();
    if (preset->properties_schema.is_object()) {
        for (auto& [key, value] : preset->properties_schema.items()) {
            if (value.is_object() && value.contains("default")) {
                fud.properties[key] = value["default"];
            }
        }
    }

    return fud;
}
