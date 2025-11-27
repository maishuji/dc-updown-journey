// Copyright 2025 Quentin Cartier
#include "udjourney-editor/mode_handlers/FUDModeHandler.hpp"

#include <imgui.h>
#include <raylib/raylib.h>
#include <cstdio>
#include <string>
#include <vector>
#include <unordered_map>

// Texture cache for sprite sheet previews
static std::unordered_map<std::string, Texture2D> sprite_preview_cache;

static Texture2D load_sprite_preview(const std::string& sprite_sheet) {
    auto it = sprite_preview_cache.find(sprite_sheet);
    if (it != sprite_preview_cache.end()) {
        return it->second;
    }

    std::string full_path = "assets/" + sprite_sheet;
    Texture2D texture = LoadTexture(full_path.c_str());
    if (texture.id != 0) {
        sprite_preview_cache[sprite_sheet] = texture;
    }
    return texture;
}

FUDModeHandler::FUDModeHandler() {
    // Load presets on construction
    fud_preset_manager_.load_available_presets();
    ui_atlas_manager_.load_presets();
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

    // Properties (JSON editor based on schema)
    if (ImGui::CollapsingHeader("Custom Properties")) {
        render_property_editor();
    }

    ImGui::Spacing();

    // Sprite selection UI
    if (ImGui::CollapsingHeader("Background Sprite",
                                ImGuiTreeNodeFlags_DefaultOpen)) {
        render_sprite_selector("Background", true);

        // Render mode dropdown
        if (!selected_fud_->background_sheet.empty()) {
            ImGui::Text("Render Mode:");
            const char* modes[] = {"Stretch", "Tile", "Center"};
            int current_mode =
                static_cast<int>(selected_fud_->background_render_mode);
            if (ImGui::Combo("##BgRenderMode", &current_mode, modes, 3)) {
                selected_fud_->background_render_mode =
                    static_cast<FUDImageRenderMode>(current_mode);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip(
                    "Stretch: Scale to fit\nTile: Repeat pattern\nCenter: No "
                    "scaling");
            }
        }
    }

    if (ImGui::CollapsingHeader("Foreground Sprite")) {
        render_sprite_selector("Foreground", false);

        // Render mode dropdown
        if (!selected_fud_->foreground_sheet.empty()) {
            ImGui::Text("Render Mode:");
            const char* modes[] = {"Stretch", "Tile", "Center"};
            int current_mode =
                static_cast<int>(selected_fud_->foreground_render_mode);
            if (ImGui::Combo("##FgRenderMode", &current_mode, modes, 3)) {
                selected_fud_->foreground_render_mode =
                    static_cast<FUDImageRenderMode>(current_mode);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip(
                    "Stretch: Scale to fit\nTile: Repeat pattern\nCenter: No "
                    "scaling");
            }
        }
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

void FUDModeHandler::render_sprite_selector(const char* label,
                                            bool is_background) {
    if (!selected_fud_ || !ui_atlas_manager_.has_presets()) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                           "No UI atlas presets available");
        ImGui::TextWrapped("Create UI atlas preset files in assets/ui_atlas/");
        return;
    }

    const auto& presets = ui_atlas_manager_.get_presets();
    std::string& current_sheet = is_background
                                     ? selected_fud_->background_sheet
                                     : selected_fud_->foreground_sheet;
    int& current_row = is_background ? selected_fud_->background_tile_row
                                     : selected_fud_->foreground_tile_row;
    int& current_col = is_background ? selected_fud_->background_tile_col
                                     : selected_fud_->foreground_tile_col;
    int& current_width = is_background ? selected_fud_->background_tile_width
                                       : selected_fud_->foreground_tile_width;
    int& current_height = is_background ? selected_fud_->background_tile_height
                                        : selected_fud_->foreground_tile_height;
    int& current_tile_size = is_background
                                 ? selected_fud_->background_tile_size
                                 : selected_fud_->foreground_tile_size;

    // Show current selection info
    if (!current_sheet.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                           "Current: %s [%d,%d] %dx%d",
                           current_sheet.c_str(),
                           current_col,
                           current_row,
                           current_width,
                           current_height);

        if (ImGui::Button("Clear")) {
            current_sheet.clear();
        }
        ImGui::SameLine();
    }

    ImGui::Text("Select Sprite:");

    // Display presets as clickable grid
    const float tile_size = 64.0f;
    const float padding = 4.0f;
    const float available_width = ImGui::GetContentRegionAvail().x;
    const int tiles_per_row =
        static_cast<int>((available_width + padding) / (tile_size + padding));

    if (tiles_per_row > 0) {
        for (size_t i = 0; i < presets.size(); ++i) {
            ImGui::PushID(
                static_cast<int>(i + 1000 + (is_background ? 0 : 500)));

            const auto& preset = presets[i];
            int& selected_idx = is_background ? selected_background_sprite_
                                              : selected_foreground_sprite_;
            bool is_selected = (selected_idx == static_cast<int>(i));

            // Load texture for preview
            Texture2D texture = load_sprite_preview(preset.sprite_sheet);

            if (texture.id != 0) {
                // Calculate UV coordinates for the tile in sprite sheet
                float tile_w = static_cast<float>(preset.tile_size);
                float tile_h = static_cast<float>(preset.tile_size);
                float u0 = (preset.tile_col * tile_w) / texture.width;
                float v0 = (preset.tile_row * tile_h) / texture.height;
                float u1 = u0 + ((preset.tile_width * tile_w) / texture.width);
                float v1 =
                    v0 + ((preset.tile_height * tile_h) / texture.height);

                // Highlight selected preset
                ImVec4 tint_color = is_selected
                                        ? ImVec4(0.5f, 1.0f, 0.5f, 1.0f)
                                        : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                ImVec4 border_color = is_selected
                                          ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f)
                                          : ImVec4(0.4f, 0.4f, 0.4f, 0.5f);

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                      ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                      ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
                ImGui::PushStyleColor(ImGuiCol_Border, border_color);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);

                char button_id[64];
                std::snprintf(button_id, sizeof(button_id), "##sprite_%zu", i);

                if (ImGui::ImageButton(button_id,
                                       static_cast<ImTextureID>(texture.id),
                                       ImVec2(tile_size, tile_size),
                                       ImVec2(u0, v0),
                                       ImVec2(u1, v1),
                                       ImVec4(0, 0, 0, 0),
                                       tint_color)) {
                    // Apply sprite to FUD
                    current_sheet = preset.sprite_sheet;
                    current_tile_size = preset.tile_size;
                    current_row = preset.tile_row;
                    current_col = preset.tile_col;
                    current_width = preset.tile_width;
                    current_height = preset.tile_height;
                    selected_idx = static_cast<int>(i);

                    // Auto-adjust FUD size to match sprite dimensions
                    float sprite_width = preset.tile_width * preset.tile_size;
                    float sprite_height = preset.tile_height * preset.tile_size;
                    selected_fud_->size = ImVec2(sprite_width, sprite_height);
                }

                ImGui::PopStyleVar();
                ImGui::PopStyleColor(4);

                // Show tooltip with sprite info
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s", preset.name.c_str());
                    if (!preset.category.empty()) {
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                                           "Category: %s",
                                           preset.category.c_str());
                    }
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                                       "Tiles: %dx%d (size: %d)",
                                       preset.tile_width,
                                       preset.tile_height,
                                       preset.tile_size);
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                                       "Position: [%d, %d]",
                                       preset.tile_col,
                                       preset.tile_row);
                    if (!preset.description.empty()) {
                        ImGui::Separator();
                        ImGui::TextWrapped("%s", preset.description.c_str());
                    }
                    ImGui::EndTooltip();
                }
            } else {
                // Fallback to text button if texture fails to load
                if (ImGui::Button(preset.name.c_str(),
                                  ImVec2(tile_size, tile_size))) {
                    current_sheet = preset.sprite_sheet;
                    current_tile_size = preset.tile_size;
                    current_row = preset.tile_row;
                    current_col = preset.tile_col;
                    current_width = preset.tile_width;
                    current_height = preset.tile_height;
                    selected_idx = static_cast<int>(i);
                }
            }

            // Layout grid
            if ((i + 1) % tiles_per_row != 0 && i < presets.size() - 1) {
                ImGui::SameLine(0.0f, padding);
            }

            ImGui::PopID();
        }
    }
}

void FUDModeHandler::render_property_editor() {
    if (!selected_fud_) {
        return;
    }

    // Get the preset to access properties_schema
    const FUDPreset* preset =
        fud_preset_manager_.get_preset(selected_fud_->type_id);
    if (!preset || preset->properties_schema.empty()) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                           "No properties defined for this FUD type");
        return;
    }

    const auto& schema = preset->properties_schema;

    // Iterate through all properties in the schema
    for (auto it = schema.begin(); it != schema.end(); ++it) {
        const std::string& prop_name = it.key();
        const auto& prop_def = it.value();

        if (!prop_def.is_object()) continue;

        std::string type = prop_def.value("type", "string");
        std::string description = prop_def.value("description", "");

        // Ensure property exists in selected_fud_, initialize with default if
        // not
        if (!selected_fud_->properties.contains(prop_name)) {
            if (prop_def.contains("default")) {
                selected_fud_->properties[prop_name] = prop_def["default"];
            }
        }

        ImGui::PushID(prop_name.c_str());

        // Render appropriate UI control based on type
        if (type == "integer") {
            int value = selected_fud_->properties[prop_name].get<int>();
            if (ImGui::InputInt(prop_name.c_str(), &value)) {
                selected_fud_->properties[prop_name] = value;
            }
        } else if (type == "float" || type == "number") {
            float value = selected_fud_->properties[prop_name].get<float>();
            if (ImGui::InputFloat(prop_name.c_str(), &value)) {
                selected_fud_->properties[prop_name] = value;
            }
        } else if (type == "boolean") {
            bool value = selected_fud_->properties[prop_name].get<bool>();
            if (ImGui::Checkbox(prop_name.c_str(), &value)) {
                selected_fud_->properties[prop_name] = value;
            }
        } else if (type == "string") {
            std::string value =
                selected_fud_->properties[prop_name].get<std::string>();
            char buffer[256];
            strncpy(buffer, value.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0';

            if (ImGui::InputText(prop_name.c_str(), buffer, sizeof(buffer))) {
                selected_fud_->properties[prop_name] = std::string(buffer);
            }
        } else if (type == "object") {
            // For objects (like sprite configs), show as collapsing header
            if (ImGui::TreeNode(prop_name.c_str())) {
                auto& obj = selected_fud_->properties[prop_name];

                // Common sprite object fields
                if (obj.contains("sheet")) {
                    std::string sheet = obj["sheet"].get<std::string>();
                    char buffer[256];
                    strncpy(buffer, sheet.c_str(), sizeof(buffer) - 1);
                    buffer[sizeof(buffer) - 1] = '\0';

                    if (ImGui::InputText("sheet", buffer, sizeof(buffer))) {
                        obj["sheet"] = std::string(buffer);
                    }
                }

                if (obj.contains("tile_size")) {
                    int tile_size = obj["tile_size"].get<int>();
                    if (ImGui::InputInt("tile_size", &tile_size)) {
                        obj["tile_size"] = tile_size;
                    }
                }

                if (obj.contains("tile_row")) {
                    int tile_row = obj["tile_row"].get<int>();
                    if (ImGui::InputInt("tile_row", &tile_row)) {
                        obj["tile_row"] = tile_row;
                    }
                }

                if (obj.contains("tile_col")) {
                    int tile_col = obj["tile_col"].get<int>();
                    if (ImGui::InputInt("tile_col", &tile_col)) {
                        obj["tile_col"] = tile_col;
                    }
                }

                ImGui::TreePop();
            }
        }

        // Show description as tooltip
        if (!description.empty() && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", description.c_str());
        }

        ImGui::PopID();
    }
}
