// Copyright 2025 Quentin Cartier
#include "udjourney-editor/mode_handlers/BackgroundModeHandler.hpp"

#include <imgui.h>
#include <cstdio>
#include <cstring>

#include <string>
#include <unordered_map>

#include "raylib/raylib.h"
#include "udjourney-editor/background/BackgroundLayer.hpp"

// Texture cache for preview thumbnails
static std::unordered_map<std::string, Texture2D> preview_texture_cache;

static Texture2D load_preview_texture(const std::string& sprite_sheet) {
    auto it = preview_texture_cache.find(sprite_sheet);
    if (it != preview_texture_cache.end()) {
        return it->second;
    }

    std::string full_path = "assets/" + sprite_sheet;
    Texture2D texture = LoadTexture(full_path.c_str());
    if (texture.id != 0) {
        preview_texture_cache[sprite_sheet] = texture;
    }
    return texture;
}

BackgroundModeHandler::BackgroundModeHandler(
    BackgroundManager* bg_manager,
    BackgroundObjectPresetManager* preset_manager) :
    background_manager_(bg_manager),
    background_preset_manager_(preset_manager) {
    std::strncpy(new_layer_name_, "New Layer", sizeof(new_layer_name_) - 1);
}

void BackgroundModeHandler::render() {
    if (!background_manager_ || !background_preset_manager_) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1),
                           "Background system not initialized");
        return;
    }

    render_layer_list();
    ImGui::Separator();
    render_object_controls();
    render_delete_confirmation();
}

void BackgroundModeHandler::render_layer_list() {
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
        ImGui::InputInt("Depth##new_layer", &new_layer_depth_);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Background Layer Rendering Order");
            ImGui::Separator();
            ImGui::TextWrapped(
                "Controls which background layer appears in front or behind "
                "OTHER background layers.\n\n"
                "- Lower values (e.g., 0, 1) render first (furthest back)\n"
                "- Higher values (e.g., 10, 100) render last (closest)\n"
                "- Multiple layers can share the same depth value\n\n"
                "Note: ALL background layers always render behind the game "
                "scene (platforms, monsters, player).");
            ImGui::EndTooltip();
        }
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

        // Right-click context menu for layer
        if (ImGui::BeginPopupContextItem()) {
            ImGui::Text("Layer: %s", layers[i].get_name().c_str());
            ImGui::Separator();

            if (ImGui::MenuItem("Move Up", nullptr, false, i > 0)) {
                background_manager_->move_layer_up(i);
            }

            if (ImGui::MenuItem(
                    "Move Down", nullptr, false, i < layers.size() - 1)) {
                background_manager_->move_layer_down(i);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete Layer")) {
                layer_to_delete_ = i;
                layer_object_count_ = layers[i].get_objects().size();
                show_delete_layer_confirmation_ = true;
            }

            ImGui::EndPopup();
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
            layer_to_delete_ = i;
            layer_object_count_ = layers[i].get_objects().size();
            show_delete_layer_confirmation_ = true;
        }

        ImGui::PopID();
    }

    if (layers.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No layers yet");
    }

    ImGui::Separator();

    // Layer Properties Section
    if (selected.has_value()) {
        BackgroundLayer* layer =
            background_manager_->get_layer(selected.value());
        if (layer) {
            ImGui::Text("Layer Properties");

            char name_buf[128];
            std::strncpy(
                name_buf, layer->get_name().c_str(), sizeof(name_buf) - 1);
            name_buf[sizeof(name_buf) - 1] = '\0';
            if (ImGui::InputText("Layer Name", name_buf, sizeof(name_buf))) {
                layer->set_name(name_buf);
            }

            char texture_buf[256];
            std::strncpy(texture_buf,
                         layer->get_texture_file().c_str(),
                         sizeof(texture_buf) - 1);
            texture_buf[sizeof(texture_buf) - 1] = '\0';
            if (ImGui::InputText(
                    "Texture File", texture_buf, sizeof(texture_buf))) {
                layer->set_texture_file(texture_buf);
            }

            float parallax = layer->get_parallax_factor();
            if (ImGui::SliderFloat("Parallax Factor", &parallax, 0.0f, 1.0f)) {
                layer->set_parallax_factor(parallax);
            }

            int depth = layer->get_depth();
            if (ImGui::InputInt("Depth##selected_layer", &depth)) {
                layer->set_depth(depth);
            }

            ImGui::Separator();
            ImGui::Text("Auto-Scroll Settings");

            bool auto_scroll = layer->get_auto_scroll_enabled();
            if (ImGui::Checkbox("Auto Scroll Enabled", &auto_scroll)) {
                layer->set_auto_scroll_enabled(auto_scroll);
            }

            float scroll_speed_x = layer->get_scroll_speed_x();
            if (ImGui::InputFloat("Scroll Speed X", &scroll_speed_x)) {
                layer->set_scroll_speed_x(scroll_speed_x);
            }

            float scroll_speed_y = layer->get_scroll_speed_y();
            if (ImGui::InputFloat("Scroll Speed Y", &scroll_speed_y)) {
                layer->set_scroll_speed_y(scroll_speed_y);
            }

            bool repeat = layer->get_repeat();
            if (ImGui::Checkbox("Repeat", &repeat)) {
                layer->set_repeat(repeat);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip(
                    "Enable infinite looping when scrolling (otherwise stops "
                    "at texture edge)");
            }

            ImGui::Separator();
        }
    }
}

void BackgroundModeHandler::render_object_controls() {
    const auto& layers = background_manager_->get_layers();
    auto selected = background_manager_->get_selected_layer();

    // Add object controls - only if layer is selected
    if (!selected.has_value()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                           "Select a layer to add objects");
        background_placing_mode_ = false;
    } else {
        // Allows adding objects when a layer is selected

        ImGui::Text("Add Objects to: %s",
                    layers[selected.value()].get_name().c_str());

        // Show available presets as clickable list
        if (background_preset_manager_->has_presets()) {
            const auto& presets = background_preset_manager_->get_presets();

            ImGui::Text("Available Objects:");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                               "(Click to select, then click in scene)");

            // Display presets as a grid of 64x64 tiles
            const float tile_size = 64.0f;
            const float padding = 4.0f;
            const float available_width = ImGui::GetContentRegionAvail().x;
            const int tiles_per_row = static_cast<int>(
                (available_width + padding) / (tile_size + padding));

            if (tiles_per_row > 0) {
                for (size_t i = 0; i < presets.size(); ++i) {
                    ImGui::PushID(static_cast<int>(i + 500));

                    bool is_selected =
                        (selected_preset_idx_ == static_cast<int>(i)) &&
                        background_placing_mode_;

                    // Load texture for preview
                    Texture2D texture =
                        load_preview_texture(presets[i].sprite_sheet);

                    if (texture.id != 0) {
                        // Calculate UV coordinates for the tile in sprite sheet
                        float tile_w = static_cast<float>(presets[i].tile_size);
                        float tile_h = static_cast<float>(presets[i].tile_size);
                        float u0 =
                            (presets[i].tile_col * tile_w) / texture.width;
                        float v0 =
                            (presets[i].tile_row * tile_h) / texture.height;
                        float u1 = u0 + (tile_w / texture.width);
                        float v1 = v0 + (tile_h / texture.height);

                        // Highlight selected preset
                        ImVec4 tint_color =
                            is_selected ? ImVec4(0.5f, 1.0f, 0.5f, 1.0f)
                                        : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

                        ImVec4 border_color =
                            is_selected ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f)
                                        : ImVec4(0.4f, 0.4f, 0.4f, 0.5f);

                        // Draw tile as ImageButton
                        ImGui::PushStyleColor(ImGuiCol_Button,
                                              ImVec4(0, 0, 0, 0));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                              ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                              ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
                        ImGui::PushStyleColor(ImGuiCol_Border, border_color);
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,
                                            2.0f);

                        char button_id[64];
                        std::snprintf(
                            button_id, sizeof(button_id), "##bg_obj_%zu", i);

                        if (ImGui::ImageButton(
                                button_id,
                                static_cast<ImTextureID>(texture.id),
                                ImVec2(tile_size, tile_size),
                                ImVec2(u0, v0),
                                ImVec2(u1, v1),
                                ImVec4(0, 0, 0, 0),  // background
                                tint_color)) {
                            selected_preset_idx_ = static_cast<int>(i);
                            new_bg_object_scale_ = presets[i].default_scale;
                            background_placing_mode_ = true;
                        }

                        ImGui::PopStyleVar();
                        ImGui::PopStyleColor(4);

                        // Show tooltip with object name on hover
                        if (ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::Text("%s", presets[i].name.c_str());
                            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                                               "Size: %dx%d",
                                               presets[i].tile_size,
                                               presets[i].tile_size);
                            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                                               "Default scale: %.1fx",
                                               presets[i].default_scale);
                            ImGui::EndTooltip();
                        }
                    } else {
                        // Fallback to text button if texture fails to load
                        if (ImGui::Button(presets[i].name.c_str(),
                                          ImVec2(tile_size, tile_size))) {
                            selected_preset_idx_ = static_cast<int>(i);
                            new_bg_object_scale_ = presets[i].default_scale;
                            background_placing_mode_ = true;
                        }
                    }

                    // Layout grid: add separator or new line
                    if ((i + 1) % tiles_per_row != 0 &&
                        i < presets.size() - 1) {
                        ImGui::SameLine(0.0f, padding);
                    }

                    ImGui::PopID();
                }
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

void BackgroundModeHandler::render_delete_confirmation() {
    // Layer deletion confirmation popup
    if (show_delete_layer_confirmation_) {
        ImGui::OpenPopup("Delete Layer?");
        show_delete_layer_confirmation_ = false;
    }

    if (ImGui::BeginPopupModal(
            "Delete Layer?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this background layer?");
        ImGui::Text("(%zu object%s)",
                    layer_object_count_,
                    layer_object_count_ == 1 ? "" : "s");
        ImGui::Separator();

        if (ImGui::Button("Delete", ImVec2(120, 0))) {
            background_manager_->remove_layer(layer_to_delete_);
            auto selected = background_manager_->get_selected_layer();
            if (selected.has_value() && selected.value() == layer_to_delete_) {
                background_manager_->clear_selection();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
