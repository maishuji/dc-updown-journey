// Copyright 2025 Quentin Cartier
#include "udjourney-editor/mode_handlers/PlatformModeHandler.hpp"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <vector>

#include "ImGuiFileDialog.h"

namespace {
constexpr const char* kChoosePlatformTextureDlgKey = "ChoosePlatformTextureDlg";

std::filesystem::path find_assets_dir() {
    namespace fs = std::filesystem;

    fs::path cur = fs::current_path();
    for (int i = 0; i < 10; ++i) {
        fs::path candidate = cur / "assets";
        std::error_code ec;
        if (fs::exists(candidate, ec) && fs::is_directory(candidate, ec)) {
            return fs::weakly_canonical(candidate, ec);
        }

        if (!cur.has_parent_path()) {
            break;
        }
        cur = cur.parent_path();
    }
    return {};
}

std::string make_asset_relative_or_keep(const std::string& filepath) {
    namespace fs = std::filesystem;

    fs::path assets_dir = find_assets_dir();
    if (assets_dir.empty()) {
        return filepath;
    }

    std::error_code ec;
    fs::path file_path = fs::weakly_canonical(fs::path(filepath), ec);
    if (ec) {
        return filepath;
    }

    auto rel = file_path.lexically_relative(assets_dir);
    if (rel.empty() || rel.native().find("..") == 0) {
        return filepath;
    }
    return rel.generic_string();
}
}  // namespace

PlatformModeHandler::PlatformModeHandler() {}

void PlatformModeHandler::render() {
    if (selected_platform_) {
        render_editor();
    } else {
        render_creator();
    }
}

void PlatformModeHandler::render_file_dialogs() {
    if (ImGuiFileDialog::Instance()->Display(kChoosePlatformTextureDlgKey,
                                             ImGuiWindowFlags_None,
                                             ImVec2(1000, 700),
                                             ImVec2(FLT_MAX, FLT_MAX))) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filepath =
                ImGuiFileDialog::Instance()->GetFilePathName();

            std::string relative_path = make_asset_relative_or_keep(filepath);

            if (texture_dialog_target_ == TextureDialogTarget::NewPlatforms) {
                new_platform_texture_file_ = relative_path;
                std::snprintf(new_texture_file_buf_,
                              sizeof(new_texture_file_buf_),
                              "%s",
                              relative_path.c_str());
            } else if (selected_platform_) {
                selected_platform_->texture_file = relative_path;
                texture_platform_ = selected_platform_;
                std::snprintf(texture_file_buf_,
                              sizeof(texture_file_buf_),
                              "%s",
                              relative_path.c_str());
            }
        }
        ImGuiFileDialog::Instance()->Close();
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

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextWrapped(
            "Default texture for newly created platforms (asset-relative). "
            "Empty = solid color.");

        if (ImGui::InputText("Texture##new",
                             new_texture_file_buf_,
                             sizeof(new_texture_file_buf_))) {
            new_platform_texture_file_ = new_texture_file_buf_;
        }

        ImGui::SameLine();
        if (ImGui::Button("Browse...##new")) {
            texture_dialog_target_ = TextureDialogTarget::NewPlatforms;
            auto config = IGFD::FileDialogConfig();
            std::filesystem::path assets_dir = find_assets_dir();
            if (!assets_dir.empty()) {
                config.path = assets_dir.string();
            }
            ImGuiFileDialog::Instance()->OpenDialog(
                kChoosePlatformTextureDlgKey,
                "Choose Platform Texture",
                ".png,.PNG",
                config);
        }

        if (ImGui::Button("Clear Texture##new", ImVec2(-1, 0))) {
            new_texture_file_buf_[0] = '\0';
            new_platform_texture_file_.clear();
        }

        ImGui::Checkbox("Tile Texture##new", &new_platform_texture_tiled_);
    }
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

    if (ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (texture_platform_ != selected_platform_) {
            texture_platform_ = selected_platform_;
            std::snprintf(texture_file_buf_,
                          sizeof(texture_file_buf_),
                          "%s",
                          selected_platform_->texture_file.c_str());
        }

        ImGui::TextWrapped(
            "Optional texture file (asset-relative). Empty = solid color.");
        if (ImGui::InputText(
                "Texture", texture_file_buf_, sizeof(texture_file_buf_))) {
            selected_platform_->texture_file = texture_file_buf_;
        }

        ImGui::SameLine();
        if (ImGui::Button("Browse...")) {
            texture_dialog_target_ = TextureDialogTarget::SelectedPlatform;
            auto config = IGFD::FileDialogConfig();
            std::filesystem::path assets_dir = find_assets_dir();
            if (!assets_dir.empty()) {
                config.path = assets_dir.string();
            }
            ImGuiFileDialog::Instance()->OpenDialog(
                kChoosePlatformTextureDlgKey,
                "Choose Platform Texture",
                ".png,.PNG",
                config);
        }

        if (ImGui::Button("Clear Texture", ImVec2(-1, 0))) {
            texture_file_buf_[0] = '\0';
            selected_platform_->texture_file.clear();
        }

        ImGui::Checkbox("Tile Texture", &selected_platform_->texture_tiled);
    }

    ImGui::Separator();
    if (ImGui::Button("Done Editing", ImVec2(-1, 0))) {
        selected_platform_ = nullptr;
        texture_platform_ = nullptr;
    }
}
