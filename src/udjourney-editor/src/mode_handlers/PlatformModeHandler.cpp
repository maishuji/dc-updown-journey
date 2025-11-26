// Copyright 2025 Quentin Cartier
#include "udjourney-editor/mode_handlers/PlatformModeHandler.hpp"

#include <algorithm>
#include <vector>

PlatformModeHandler::PlatformModeHandler() {}

void PlatformModeHandler::render() {
    if (selected_platform_) {
        render_editor();
    } else {
        render_creator();
    }
}

std::vector<PlatformFeatureType> PlatformModeHandler::get_selected_features()
    const {
    std::vector<PlatformFeatureType> features;
    if (feature_spikes_) features.push_back(PlatformFeatureType::Spikes);
    if (feature_checkpoint_)
        features.push_back(PlatformFeatureType::Checkpoint);
    return features;
}

void PlatformModeHandler::render_creator() {
    ImGui::Text("Platform Creator");
    ImGui::Separator();

    ImGui::TextWrapped("Behavior for new platforms:");
    if (ImGui::RadioButton(
            "Static", platform_behavior_ == PlatformBehaviorType::Static)) {
        platform_behavior_ = PlatformBehaviorType::Static;
    }
    if (ImGui::RadioButton(
            "Horizontal",
            platform_behavior_ == PlatformBehaviorType::Horizontal)) {
        platform_behavior_ = PlatformBehaviorType::Horizontal;
    }
    if (ImGui::RadioButton(
            "Eight Turn",
            platform_behavior_ == PlatformBehaviorType::EightTurnHorizontal)) {
        platform_behavior_ = PlatformBehaviorType::EightTurnHorizontal;
    }
    if (ImGui::RadioButton(
            "Oscillating Size",
            platform_behavior_ == PlatformBehaviorType::OscillatingSize)) {
        platform_behavior_ = PlatformBehaviorType::OscillatingSize;
    }

    if (ImGui::CollapsingHeader("Size", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat(
            "Width", &platform_size_.x, 0.5f, 5.0f, "%.1f tiles");
        ImGui::SliderFloat(
            "Height", &platform_size_.y, 0.5f, 3.0f, "%.1f tiles");
    }

    ImGui::Separator();
    ImGui::TextWrapped("Features for new platforms:");
    ImGui::Checkbox("Spikes", &feature_spikes_);
    ImGui::Checkbox("Checkpoint", &feature_checkpoint_);

    ImGui::Separator();
    ImGui::TextWrapped("Controls:");
    ImGui::BulletText("Left click: Create platform");
    ImGui::BulletText("Right click: Delete platform");
    ImGui::BulletText("Ctrl+Left click: Edit platform");
}

void PlatformModeHandler::render_editor() {
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Platform Editor");
    ImGui::Separator();

    if (!selected_platform_) return;

    ImGui::Text("Position: (%d, %d)",
                selected_platform_->tile_x,
                selected_platform_->tile_y);

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
        bool has_spikes = std::find(selected_platform_->features.begin(),
                                    selected_platform_->features.end(),
                                    PlatformFeatureType::Spikes) !=
                          selected_platform_->features.end();
        bool has_checkpoint = std::find(selected_platform_->features.begin(),
                                        selected_platform_->features.end(),
                                        PlatformFeatureType::Checkpoint) !=
                              selected_platform_->features.end();

        if (ImGui::Checkbox("Spikes##edit", &has_spikes)) {
            if (has_spikes) {
                if (std::find(selected_platform_->features.begin(),
                              selected_platform_->features.end(),
                              PlatformFeatureType::Spikes) ==
                    selected_platform_->features.end()) {
                    selected_platform_->features.push_back(
                        PlatformFeatureType::Spikes);
                }
            } else {
                selected_platform_->features.erase(
                    std::remove(selected_platform_->features.begin(),
                                selected_platform_->features.end(),
                                PlatformFeatureType::Spikes),
                    selected_platform_->features.end());
            }
        }

        if (ImGui::Checkbox("Checkpoint##edit", &has_checkpoint)) {
            if (has_checkpoint) {
                if (std::find(selected_platform_->features.begin(),
                              selected_platform_->features.end(),
                              PlatformFeatureType::Checkpoint) ==
                    selected_platform_->features.end()) {
                    selected_platform_->features.push_back(
                        PlatformFeatureType::Checkpoint);
                }
            } else {
                selected_platform_->features.erase(
                    std::remove(selected_platform_->features.begin(),
                                selected_platform_->features.end(),
                                PlatformFeatureType::Checkpoint),
                    selected_platform_->features.end());
            }
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Done Editing", ImVec2(-1, 0))) {
        selected_platform_ = nullptr;
    }
}
