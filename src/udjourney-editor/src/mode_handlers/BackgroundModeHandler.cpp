// Copyright 2025 Quentin Cartier
#include "udjourney-editor/mode_handlers/BackgroundModeHandler.hpp"

#include <imgui.h>
#include <cstdio>
#include <cstring>

#include "udjourney-editor/background/BackgroundLayer.hpp"

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
