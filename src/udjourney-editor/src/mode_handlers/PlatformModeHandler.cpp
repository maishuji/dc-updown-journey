// Copyright 2025 Quentin Cartier
#include "udjourney-editor/mode_handlers/PlatformModeHandler.hpp"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <vector>
#include <memory>
#include <unordered_map>

#include "ImGuiFileDialog.h"
#include "udjourney-editor/PlatformPresetManager.hpp"

// Texture cache for preset previews
static std::unordered_map<std::string, Texture2D> preset_texture_cache;

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

PlatformModeHandler::PlatformModeHandler() :
    platform_preset_manager_(
        std::make_unique<udjourney::editor::PlatformPresetManager>()) {
    // Initialize with first preset if available
    auto preset_names = platform_preset_manager_->get_preset_names();
    if (!preset_names.empty()) {
        selected_platform_preset_ = preset_names[0];
        selected_preset_index_ = 0;
    }
}

const udjourney::editor::PlatformPresetInfo*
PlatformModeHandler::get_selected_preset_info() const {
    if (selected_platform_preset_.empty() || !platform_preset_manager_) {
        return nullptr;
    }
    return platform_preset_manager_->get_preset(selected_platform_preset_);
}

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

            if (selected_platform_) {
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
    if (feature_downward_spikes_)
        features.push_back(PlatformFeatureType::DownwardSpikes);
    if (feature_checkpoint_)
        features.push_back(PlatformFeatureType::Checkpoint);
    return features;
}

std::map<std::string, float> PlatformModeHandler::get_behavior_params() const {
    std::map<std::string, float> params;

    switch (platform_behavior_) {
        case PlatformBehaviorType::Horizontal:
            params["speed"] = horizontal_speed_;
            params["range"] = horizontal_range_;
            params["initial_offset"] = horizontal_initial_offset_;
            break;
        case PlatformBehaviorType::EightTurnHorizontal:
            params["speed"] = eight_turn_speed_;
            params["amplitude"] = eight_turn_amplitude_;
            break;
        case PlatformBehaviorType::OscillatingSize:
            params["speed"] = oscillating_speed_;
            params["min_scale"] = oscillating_min_scale_;
            params["max_scale"] = oscillating_max_scale_;
            break;
        case PlatformBehaviorType::CameraFollowVertical:
            params["offset"] = camera_follow_offset_;
            break;
        case PlatformBehaviorType::Static:
        default:
            // No parameters for static platforms
            break;
    }

    return params;
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
    if (ImGui::RadioButton(
            "Camera Follow Vertical",
            platform_behavior_ == PlatformBehaviorType::CameraFollowVertical)) {
        platform_behavior_ = PlatformBehaviorType::CameraFollowVertical;
    }

    if (ImGui::CollapsingHeader("Size", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat(
            "Width", &platform_size_.x, 0.5f, 5.0f, "%.1f tiles");
        ImGui::SliderFloat(
            "Height", &platform_size_.y, 0.5f, 3.0f, "%.1f tiles");
    }

    // Behavior Parameters
    if (ImGui::CollapsingHeader("Behavior Parameters",
                                ImGuiTreeNodeFlags_DefaultOpen)) {
        if (platform_behavior_ == PlatformBehaviorType::Horizontal) {
            ImGui::TextWrapped("Horizontal Movement:");
            ImGui::SliderFloat(
                "Speed##new", &horizontal_speed_, 0.5f, 10.0f, "%.1f");
            ImGui::SliderFloat(
                "Range##new", &horizontal_range_, 1.0f, 15.0f, "%.1f tiles");
            ImGui::SliderFloat("Initial Offset##new",
                               &horizontal_initial_offset_,
                               -10.0f,
                               10.0f,
                               "%.1f tiles");
        } else if (platform_behavior_ ==
                   PlatformBehaviorType::EightTurnHorizontal) {
            ImGui::TextWrapped("Eight Turn Movement:");
            ImGui::SliderFloat(
                "Speed##new_et", &eight_turn_speed_, 0.1f, 5.0f, "%.1f");
            ImGui::SliderFloat("Amplitude##new_et",
                               &eight_turn_amplitude_,
                               1.0f,
                               10.0f,
                               "%.1f tiles");
        } else if (platform_behavior_ ==
                   PlatformBehaviorType::OscillatingSize) {
            ImGui::TextWrapped("Oscillating Size:");
            ImGui::SliderFloat(
                "Speed##new_osc", &oscillating_speed_, 0.5f, 5.0f, "%.1f");
            ImGui::SliderFloat("Min Scale##new_osc",
                               &oscillating_min_scale_,
                               0.3f,
                               1.0f,
                               "%.1f");
            ImGui::SliderFloat("Max Scale##new_osc",
                               &oscillating_max_scale_,
                               1.0f,
                               2.0f,
                               "%.1f");
        } else if (platform_behavior_ ==
                   PlatformBehaviorType::CameraFollowVertical) {
            ImGui::TextWrapped("Camera Follow Vertical:");
            ImGui::SliderFloat("Offset##new_cfv",
                               &camera_follow_offset_,
                               0.0f,
                               15.0f,
                               "%.1f tiles");
            ImGui::TextWrapped(
                "Platform will stay at this offset (in tiles) from the "
                "camera's top edge.");
        } else {
            ImGui::TextDisabled(
                "Static platforms have no behavior parameters.");
        }
    }

    ImGui::Separator();
    ImGui::TextWrapped("Features for new platforms:");
    ImGui::Checkbox("Spikes", &feature_spikes_);
    ImGui::Checkbox("Downward Spikes", &feature_downward_spikes_);
    ImGui::Checkbox("Checkpoint", &feature_checkpoint_);

    ImGui::Separator();
    ImGui::TextWrapped("Controls:");
    ImGui::BulletText("Left click: Create platform");
    ImGui::BulletText("Right click: Delete platform");
    ImGui::BulletText("Ctrl+Left click: Edit platform");

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Visual", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextWrapped("Tile Render Mode:");
        if (ImGui::RadioButton("Stretch to fit", !tile_render_tiled_)) {
            tile_render_tiled_ = false;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Tile/Repeat", tile_render_tiled_)) {
            tile_render_tiled_ = true;
        }

        ImGui::Separator();
        ImGui::TextWrapped("Select platform preset:");

        if (!platform_preset_manager_) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                               "Platform preset manager not initialized!");
            return;
        }

        auto presets = platform_preset_manager_->get_presets();
        if (!presets.empty()) {
            const float tile_size = 64.0f;
            const float padding = 4.0f;
            const float available_width = ImGui::GetContentRegionAvail().x;
            const int tiles_per_row =
                std::max(1,
                         static_cast<int>((available_width + padding) /
                                          (tile_size + padding)));

            for (size_t i = 0; i < presets.size(); ++i) {
                ImGui::PushID(static_cast<int>(i));

                const auto& preset = presets[i];
                bool is_selected = (selected_platform_preset_ == preset.name);

                // Load texture for preview (with cache)
                std::string full_path = "assets/" + preset.sprite_sheet;
                Texture2D texture;
                auto it = preset_texture_cache.find(full_path);
                if (it != preset_texture_cache.end()) {
                    texture = it->second;
                } else {
                    texture = LoadTexture(full_path.c_str());
                    if (texture.id != 0) {
                        preset_texture_cache[full_path] = texture;
                    }
                }

                if (texture.id != 0) {
                    Rectangle src = preset.get_source_rect();

                    // Calculate UV coordinates
                    float u0 = src.x / texture.width;
                    float v0 = src.y / texture.height;
                    float u1 = (src.x + src.width) / texture.width;
                    float v1 = (src.y + src.height) / texture.height;

                    // Style for button
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

                    if (ImGui::ImageButton("##preset",
                                           static_cast<ImTextureID>(texture.id),
                                           ImVec2(tile_size, tile_size),
                                           ImVec2(u0, v0),
                                           ImVec2(u1, v1),
                                           ImVec4(0, 0, 0, 0),
                                           tint_color)) {
                        selected_preset_index_ = static_cast<int>(i);
                        selected_platform_preset_ = preset.name;
                    }

                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(4);

                    // Show tooltip with preset info
                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::Text("%s", preset.name.c_str());
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                                           "Sheet: %s",
                                           preset.sprite_sheet.c_str());
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                                           "Tile: [%d, %d] Size: %d",
                                           preset.tile_col,
                                           preset.tile_row,
                                           preset.tile_size);
                        ImGui::EndTooltip();
                    }
                } else {
                    // Fallback to text button if texture fails to load
                    if (ImGui::Button(preset.name.c_str(),
                                      ImVec2(tile_size, tile_size))) {
                        selected_preset_index_ = static_cast<int>(i);
                        selected_platform_preset_ = preset.name;
                    }
                }

                // Layout in grid
                if ((i + 1) % tiles_per_row != 0 && i + 1 < presets.size()) {
                    ImGui::SameLine();
                }

                ImGui::PopID();
            }
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                               "No platform presets available!");
            ImGui::TextWrapped(
                "Make sure platform_presets.json is accessible.");
        }
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
        if (ImGui::RadioButton(
                "Camera Follow Vertical##edit",
                selected_platform_->behavior_type ==
                    PlatformBehaviorType::CameraFollowVertical)) {
            selected_platform_->behavior_type =
                PlatformBehaviorType::CameraFollowVertical;
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

    // Behavior Parameters for selected platform
    if (ImGui::CollapsingHeader("Behavior Parameters",
                                ImGuiTreeNodeFlags_DefaultOpen)) {
        if (selected_platform_->behavior_type ==
            PlatformBehaviorType::Horizontal) {
            ImGui::TextWrapped("Horizontal Movement:");

            // Get or initialize parameters
            float speed = selected_platform_->behavior_params.count("speed")
                              ? selected_platform_->behavior_params["speed"]
                              : 2.0f;
            float range = selected_platform_->behavior_params.count("range")
                              ? selected_platform_->behavior_params["range"]
                              : 5.0f;
            float initial_offset =
                selected_platform_->behavior_params.count("initial_offset")
                    ? selected_platform_->behavior_params["initial_offset"]
                    : 0.0f;

            if (ImGui::SliderFloat(
                    "Speed##edit", &speed, 0.5f, 10.0f, "%.1f")) {
                selected_platform_->behavior_params["speed"] = speed;
            }
            if (ImGui::SliderFloat(
                    "Range##edit", &range, 1.0f, 15.0f, "%.1f tiles")) {
                selected_platform_->behavior_params["range"] = range;
            }
            if (ImGui::SliderFloat("Initial Offset##edit",
                                   &initial_offset,
                                   -10.0f,
                                   10.0f,
                                   "%.1f tiles")) {
                selected_platform_->behavior_params["initial_offset"] =
                    initial_offset;
            }
        } else if (selected_platform_->behavior_type ==
                   PlatformBehaviorType::EightTurnHorizontal) {
            ImGui::TextWrapped("Eight Turn Movement:");

            float speed = selected_platform_->behavior_params.count("speed")
                              ? selected_platform_->behavior_params["speed"]
                              : 1.0f;
            float amplitude =
                selected_platform_->behavior_params.count("amplitude")
                    ? selected_platform_->behavior_params["amplitude"]
                    : 4.0f;

            if (ImGui::SliderFloat(
                    "Speed##edit_et", &speed, 0.1f, 5.0f, "%.1f")) {
                selected_platform_->behavior_params["speed"] = speed;
            }
            if (ImGui::SliderFloat("Amplitude##edit_et",
                                   &amplitude,
                                   1.0f,
                                   10.0f,
                                   "%.1f tiles")) {
                selected_platform_->behavior_params["amplitude"] = amplitude;
            }
        } else if (selected_platform_->behavior_type ==
                   PlatformBehaviorType::OscillatingSize) {
            ImGui::TextWrapped("Oscillating Size:");

            float speed = selected_platform_->behavior_params.count("speed")
                              ? selected_platform_->behavior_params["speed"]
                              : 2.0f;
            float min_scale =
                selected_platform_->behavior_params.count("min_scale")
                    ? selected_platform_->behavior_params["min_scale"]
                    : 0.5f;
            float max_scale =
                selected_platform_->behavior_params.count("max_scale")
                    ? selected_platform_->behavior_params["max_scale"]
                    : 1.5f;

            if (ImGui::SliderFloat(
                    "Speed##edit_osc", &speed, 0.5f, 5.0f, "%.1f")) {
                selected_platform_->behavior_params["speed"] = speed;
            }
            if (ImGui::SliderFloat(
                    "Min Scale##edit_osc", &min_scale, 0.3f, 1.0f, "%.1f")) {
                selected_platform_->behavior_params["min_scale"] = min_scale;
            }
            if (ImGui::SliderFloat(
                    "Max Scale##edit_osc", &max_scale, 1.0f, 2.0f, "%.1f")) {
                selected_platform_->behavior_params["max_scale"] = max_scale;
            }
        } else if (selected_platform_->behavior_type ==
                   PlatformBehaviorType::CameraFollowVertical) {
            ImGui::TextWrapped("Camera Follow Vertical:");

            float offset = selected_platform_->behavior_params.count("offset")
                               ? selected_platform_->behavior_params["offset"]
                               : 5.0f;

            if (ImGui::SliderFloat(
                    "Offset##edit_cfv", &offset, 0.0f, 15.0f, "%.1f tiles")) {
                selected_platform_->behavior_params["offset"] = offset;
            }
            ImGui::TextWrapped(
                "Platform will stay at this offset (in tiles) from the "
                "camera's top edge.");
        } else {
            ImGui::TextDisabled(
                "Static platforms have no behavior parameters.");
        }
    }

    if (ImGui::CollapsingHeader("Features", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool has_spikes = std::find(selected_platform_->features.begin(),
                                    selected_platform_->features.end(),
                                    PlatformFeatureType::Spikes) !=
                          selected_platform_->features.end();
        bool has_downward_spikes = std::find(selected_platform_->features.begin(),
                                             selected_platform_->features.end(),
                                             PlatformFeatureType::DownwardSpikes) !=
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

        if (ImGui::Checkbox("Downward Spikes##edit", &has_downward_spikes)) {
            if (has_downward_spikes) {
                if (std::find(selected_platform_->features.begin(),
                              selected_platform_->features.end(),
                              PlatformFeatureType::DownwardSpikes) ==
                    selected_platform_->features.end()) {
                    selected_platform_->features.push_back(
                        PlatformFeatureType::DownwardSpikes);
                }
            } else {
                selected_platform_->features.erase(
                    std::remove(selected_platform_->features.begin(),
                                selected_platform_->features.end(),
                                PlatformFeatureType::DownwardSpikes),
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
