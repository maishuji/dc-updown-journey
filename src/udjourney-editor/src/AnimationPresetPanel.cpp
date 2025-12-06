// Copyright 2025 Quentin Cartier
#include "udjourney-editor/AnimationPresetPanel.hpp"

#include <imgui.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>

#include <nlohmann/json.hpp>
#include "ImGuiFileDialog.h"
#include <udj-core/CoreUtils.hpp>

namespace udjourney {
namespace editor {

AnimationPresetPanel::AnimationPresetPanel() {
    // Initialize with empty preset
    current_preset_.preset_name = "new_preset";
    std::strncpy(preset_name_buffer_,
                 current_preset_.preset_name.c_str(),
                 sizeof(preset_name_buffer_) - 1);
}

AnimationPresetPanel::~AnimationPresetPanel() { cleanup_textures(); }

void AnimationPresetPanel::draw() {
    if (!is_open_) return;

    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Animation Preset Editor", &is_open_)) {
        draw_toolbar();

        ImGui::Separator();

        // Split into left and right panels
        ImGui::BeginChild("LeftPanel", ImVec2(300, 0), true);
        draw_preset_list();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
        if (selected_animation_index_ >= 0 &&
            selected_animation_index_ <
                static_cast<int>(current_preset_.animations.size())) {
            draw_animation_editor();
        } else {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                               "Select an animation to edit");
        }
        ImGui::EndChild();
    }
    ImGui::End();

    // File dialogs (must be outside the main window)
    if (ImGuiFileDialog::Instance()->Display("LoadAnimPresetDlg",
                                             ImGuiWindowFlags_None,
                                             ImVec2(1000, 700),
                                             ImVec2(FLT_MAX, FLT_MAX))) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filepath =
                ImGuiFileDialog::Instance()->GetFilePathName();
            load_preset(filepath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("SaveAnimPresetDlg",
                                             ImGuiWindowFlags_None,
                                             ImVec2(1000, 700),
                                             ImVec2(FLT_MAX, FLT_MAX))) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filepath =
                ImGuiFileDialog::Instance()->GetFilePathName();
            save_preset(filepath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    // Sprite sheet selection dialog
    if (ImGuiFileDialog::Instance()->Display("ChooseSpriteSheetDlg",
                                             ImGuiWindowFlags_None,
                                             ImVec2(1000, 700),
                                             ImVec2(FLT_MAX, FLT_MAX))) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filepath =
                ImGuiFileDialog::Instance()->GetFilePathName();

            // Update the sprite filename buffer and config
            if (selected_animation_index_ >= 0 &&
                selected_animation_index_ <
                    static_cast<int>(current_preset_.animations.size())) {
                auto& sprite_cfg =
                    current_preset_.animations[selected_animation_index_]
                        .sprite_config;

                // Convert absolute path to relative path from ASSETS_BASE_PATH
                std::string relative_path = filepath;
                try {
                    // Get the real romdisk path (resolve the assets symlink)
                    std::string romdisk_path =
                        std::filesystem::canonical("assets").string();
                    if (!romdisk_path.empty() && romdisk_path.back() != '/') {
                        romdisk_path += '/';
                    }

                    // If the filepath is absolute and within romdisk, make it
                    // relative
                    if (filepath.find(romdisk_path) == 0) {
                        relative_path = filepath.substr(romdisk_path.length());
                    }
                } catch (const std::filesystem::filesystem_error&) {
                    // If we can't resolve the path, use it as-is
                }

                sprite_cfg.filename = relative_path;
                std::strncpy(sprite_filename_buffer_,
                             relative_path.c_str(),
                             sizeof(sprite_filename_buffer_) - 1);
                has_unsaved_changes_ = true;
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

void AnimationPresetPanel::draw_toolbar() {
    // Preset name
    ImGui::Text("Preset Name:");
    ImGui::SameLine();
    if (ImGui::InputText(
            "##PresetName", preset_name_buffer_, sizeof(preset_name_buffer_))) {
        current_preset_.preset_name = preset_name_buffer_;
        has_unsaved_changes_ = true;
    }

    ImGui::SameLine();
    if (has_unsaved_changes_) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "*");
    }

    ImGui::SameLine(ImGui::GetWindowWidth() - 400);

    // File operations
    if (ImGui::Button("New")) {
        create_new_preset("new_preset");
    }

    ImGui::SameLine();
    if (ImGui::Button("Load")) {
        auto config = IGFD::FileDialogConfig();
        ImGuiFileDialog::Instance()->OpenDialog(
            "LoadAnimPresetDlg", "Load Animation Preset", ".json", config);
    }

    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        if (!current_filepath_.empty()) {
            save_preset(current_filepath_);
        } else {
            auto config = IGFD::FileDialogConfig();
            ImGuiFileDialog::Instance()->OpenDialog(
                "SaveAnimPresetDlg", "Save Animation Preset", ".json", config);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Save As...")) {
        auto config = IGFD::FileDialogConfig();
        ImGuiFileDialog::Instance()->OpenDialog(
            "SaveAnimPresetDlg", "Save Animation Preset", ".json", config);
    }
}

void AnimationPresetPanel::draw_preset_list() {
    ImGui::Text("Animations (%zu)", current_preset_.animations.size());

    if (ImGui::Button("Add Animation", ImVec2(-1, 0))) {
        add_animation_state();
    }

    ImGui::Separator();

    // List all animations
    for (int i = 0; i < static_cast<int>(current_preset_.animations.size());
         ++i) {
        const auto& anim = current_preset_.animations[i];

        bool is_selected = (i == selected_animation_index_);
        std::string label =
            anim.name + " (ID: " + std::to_string(anim.state_id) + ")";

        if (ImGui::Selectable(label.c_str(), is_selected)) {
            selected_animation_index_ = i;

            // Update the edit buffers when selecting an animation
            std::strncpy(animation_name_buffer_,
                         anim.name.c_str(),
                         sizeof(animation_name_buffer_) - 1);
            animation_name_buffer_[sizeof(animation_name_buffer_) - 1] = '\0';

            std::strncpy(sprite_filename_buffer_,
                         anim.sprite_config.filename.c_str(),
                         sizeof(sprite_filename_buffer_) - 1);
            sprite_filename_buffer_[sizeof(sprite_filename_buffer_) - 1] = '\0';

            // Reset preview state when changing animation
            is_playing_ = false;
            current_frame_index_ = 0;
            frame_timer_ = 0.0f;
        }

        // Right-click context menu
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Duplicate")) {
                duplicate_selected_animation();
            }
            if (ImGui::MenuItem("Delete")) {
                remove_selected_animation();
            }
            ImGui::EndPopup();
        }
    }
}

void AnimationPresetPanel::draw_animation_editor() {
    auto& anim = current_preset_.animations[selected_animation_index_];

    ImGui::Text("Edit Animation: %s", anim.name.c_str());
    ImGui::Separator();

    // Animation basic properties
    ImGui::InputText(
        "Name", animation_name_buffer_, sizeof(animation_name_buffer_));
    if (std::strcmp(animation_name_buffer_, anim.name.c_str()) != 0) {
        anim.name = animation_name_buffer_;
        has_unsaved_changes_ = true;
    }

    if (ImGui::InputInt("State ID", &anim.state_id)) {
        has_unsaved_changes_ = true;
    }

    ImGui::Separator();

    // Tabs for different sections
    if (ImGui::BeginTabBar("AnimationTabs")) {
        if (ImGui::BeginTabItem("Sprite Config")) {
            draw_sprite_config_editor();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Collision Bounds")) {
            draw_collision_bounds_editor();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Preview")) {
            draw_preview();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void AnimationPresetPanel::draw_collision_bounds_editor() {
    auto& anim = current_preset_.animations[selected_animation_index_];
    auto& bounds = anim.collision_bounds;

    ImGui::Text("Collision Bounds");
    ImGui::Separator();

    ImGui::Text("Define the collision box for this animation state.");
    ImGui::Text("Coordinates are relative to the sprite position.");

    ImGui::Spacing();

    bool changed = false;
    changed |= ImGui::DragFloat(
        "Offset X", &bounds.offset_x, 0.5f, 0.0f, 128.0f, "%.1f");
    changed |= ImGui::DragFloat(
        "Offset Y", &bounds.offset_y, 0.5f, 0.0f, 128.0f, "%.1f");
    changed |=
        ImGui::DragFloat("Width", &bounds.width, 0.5f, 1.0f, 128.0f, "%.1f");
    changed |=
        ImGui::DragFloat("Height", &bounds.height, 0.5f, 1.0f, 128.0f, "%.1f");

    if (changed) {
        has_unsaved_changes_ = true;
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Quick presets
    ImGui::Text("Quick Presets:");
    if (ImGui::Button("Full Sprite")) {
        bounds.offset_x = 0;
        bounds.offset_y = 0;
        bounds.width = anim.sprite_config.sprite_width;
        bounds.height = anim.sprite_config.sprite_height;
        has_unsaved_changes_ = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Centered 50%")) {
        bounds.offset_x = anim.sprite_config.sprite_width * 0.25f;
        bounds.offset_y = anim.sprite_config.sprite_height * 0.25f;
        bounds.width = anim.sprite_config.sprite_width * 0.5f;
        bounds.height = anim.sprite_config.sprite_height * 0.5f;
        has_unsaved_changes_ = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Bottom Half")) {
        bounds.offset_x = anim.sprite_config.sprite_width * 0.25f;
        bounds.offset_y = anim.sprite_config.sprite_height * 0.5f;
        bounds.width = anim.sprite_config.sprite_width * 0.5f;
        bounds.height = anim.sprite_config.sprite_height * 0.5f;
        has_unsaved_changes_ = true;
    }

    ImGui::Spacing();

    // Visual preview of bounds
    ImGui::Text("Visual Preview:");
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size(200, 200);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Draw sprite outline
    float sprite_w = anim.sprite_config.sprite_width;
    float sprite_h = anim.sprite_config.sprite_height;
    float scale =
        std::min(canvas_size.x / sprite_w, canvas_size.y / sprite_h) * 0.8f;

    ImVec2 sprite_min(canvas_pos.x + (canvas_size.x - sprite_w * scale) * 0.5f,
                      canvas_pos.y + (canvas_size.y - sprite_h * scale) * 0.5f);
    ImVec2 sprite_max(sprite_min.x + sprite_w * scale,
                      sprite_min.y + sprite_h * scale);

    // Try to render the actual sprite texture if available
    if (!anim.sprite_config.filename.empty() &&
        !anim.sprite_config.frames.empty()) {
        Texture2D texture = load_texture_cached(anim.sprite_config.filename);

        if (texture.id > 0) {
            const auto& first_frame = anim.sprite_config.frames[0];

            // Calculate source rectangle from sprite sheet
            Rectangle source = {static_cast<float>(first_frame.col * sprite_w),
                                static_cast<float>(first_frame.row * sprite_h),
                                sprite_w,
                                sprite_h};

            // Check if frame is valid
            if (source.x + source.width <= texture.width &&
                source.y + source.height <= texture.height) {
                // Convert raylib texture to ImGui texture
                void* tex_id =
                    reinterpret_cast<void*>(static_cast<uintptr_t>(texture.id));

                // Calculate UV coordinates
                ImVec2 uv0(source.x / texture.width, source.y / texture.height);
                ImVec2 uv1((source.x + source.width) / texture.width,
                           (source.y + source.height) / texture.height);

                // Display size
                ImVec2 size(sprite_w * scale, sprite_h * scale);

                // Draw the sprite texture
                draw_list->AddImage(tex_id, sprite_min, sprite_max, uv0, uv1);
            } else {
                // Fallback to outline if frame is invalid
                draw_list->AddRect(
                    sprite_min, sprite_max, IM_COL32(100, 100, 100, 255));
            }
        } else {
            // Fallback to outline if texture can't be loaded
            draw_list->AddRect(
                sprite_min, sprite_max, IM_COL32(100, 100, 100, 255));
        }
    } else {
        // Draw sprite outline only if no texture
        draw_list->AddRect(
            sprite_min, sprite_max, IM_COL32(100, 100, 100, 255));
    }

    // Draw collision bounds
    ImVec2 bounds_min(sprite_min.x + bounds.offset_x * scale,
                      sprite_min.y + bounds.offset_y * scale);
    ImVec2 bounds_max(bounds_min.x + bounds.width * scale,
                      bounds_min.y + bounds.height * scale);

    draw_list->AddRectFilled(bounds_min, bounds_max, IM_COL32(255, 0, 0, 80));
    draw_list->AddRect(
        bounds_min, bounds_max, IM_COL32(255, 0, 0, 255), 0.0f, 0, 2.0f);

    ImGui::Dummy(canvas_size);
}

void AnimationPresetPanel::draw_sprite_config_editor() {
    auto& anim = current_preset_.animations[selected_animation_index_];
    auto& sprite_cfg = anim.sprite_config;

    ImGui::Text("Sprite Configuration");
    ImGui::Separator();

    // Filename with browse button
    ImGui::Text("Sprite Sheet:");
    ImGui::InputText("##SpriteSheet",
                     sprite_filename_buffer_,
                     sizeof(sprite_filename_buffer_));
    if (std::strcmp(sprite_filename_buffer_, sprite_cfg.filename.c_str()) !=
        0) {
        sprite_cfg.filename = sprite_filename_buffer_;
        has_unsaved_changes_ = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Browse...##SpriteBrowse")) {
        auto config = IGFD::FileDialogConfig();
        config.path = udj::core::filesystem::get_assets_path("");
        ImGuiFileDialog::Instance()->OpenDialog("ChooseSpriteSheetDlg",
                                                "Select Sprite Sheet",
                                                ".png,.jpg,.bmp",
                                                config);
    }

    // Sprite dimensions
    bool changed = false;
    changed |= ImGui::InputInt("Sprite Width", &sprite_cfg.sprite_width);
    changed |= ImGui::InputInt("Sprite Height", &sprite_cfg.sprite_height);

    if (changed) {
        has_unsaved_changes_ = true;
    }

    ImGui::Separator();

    // Animation properties
    changed = false;
    changed |= ImGui::DragFloat("Frame Duration",
                                &sprite_cfg.frame_duration,
                                0.01f,
                                0.01f,
                                5.0f,
                                "%.3f");
    changed |= ImGui::Checkbox("Loop", &sprite_cfg.loop);

    if (changed) {
        has_unsaved_changes_ = true;
    }

    ImGui::Separator();

    // Frames list
    ImGui::Text("Frames (%zu)", sprite_cfg.frames.size());

    if (ImGui::Button("Add Frame")) {
        sprite_cfg.frames.push_back({0, 0});
        has_unsaved_changes_ = true;
    }

    ImGui::Spacing();

    // Frame editor
    for (size_t i = 0; i < sprite_cfg.frames.size(); ++i) {
        ImGui::PushID(i);

        bool frame_changed = false;
        ImGui::Text("Frame %zu:", i);

        ImGui::SetNextItemWidth(80);
        frame_changed |=
            ImGui::DragInt("Row", &sprite_cfg.frames[i].row, 0.1f, 0, 100);
        ImGui::SameLine();

        ImGui::SetNextItemWidth(80);
        frame_changed |=
            ImGui::DragInt("Col", &sprite_cfg.frames[i].col, 0.1f, 0, 100);

        ImGui::SameLine();
        if (ImGui::Button("X")) {
            sprite_cfg.frames.erase(sprite_cfg.frames.begin() + i);
            has_unsaved_changes_ = true;
            ImGui::PopID();
            break;
        }

        if (frame_changed) {
            has_unsaved_changes_ = true;
        }

        ImGui::PopID();
    }

    ImGui::Separator();
    ImGui::Text("Sprite Preview");

    // Try to load and display the sprite sheet
    if (!sprite_cfg.filename.empty()) {
        Texture2D texture = load_texture_cached(sprite_cfg.filename);

        if (texture.id > 0) {
            // Display sprite sheet info
            ImGui::Text("Texture: %dx%d", texture.width, texture.height);

            // Calculate how many sprites fit in the sheet
            int cols = sprite_cfg.sprite_width > 0
                           ? texture.width / sprite_cfg.sprite_width
                           : 0;
            int rows = sprite_cfg.sprite_height > 0
                           ? texture.height / sprite_cfg.sprite_height
                           : 0;
            ImGui::Text("Grid: %dx%d (%d sprites)", cols, rows, cols * rows);

            ImGui::Spacing();

            // Preview current frame if we have frames
            if (!sprite_cfg.frames.empty()) {
                ImGui::Text("Current Frame Preview:");

                // Start horizontal scrolling region
                ImGui::BeginChild(
                    "FramePreviewScroll",
                    ImVec2(0, sprite_cfg.sprite_height * 2.0f + 50),
                    false,
                    ImGuiWindowFlags_HorizontalScrollbar);

                for (size_t frame_idx = 0; frame_idx < sprite_cfg.frames.size();
                     ++frame_idx) {
                    const auto& frame = sprite_cfg.frames[frame_idx];

                    // Calculate source rectangle from sprite sheet
                    Rectangle source = {
                        static_cast<float>(frame.col * sprite_cfg.sprite_width),
                        static_cast<float>(frame.row *
                                           sprite_cfg.sprite_height),
                        static_cast<float>(sprite_cfg.sprite_width),
                        static_cast<float>(sprite_cfg.sprite_height)};
                    // Only show preview if coordinates are valid
                    if (source.x + source.width <= texture.width &&
                        source.y + source.height <= texture.height) {
                        ImGui::BeginGroup();
                        ImGui::Text("Frame %zu:", frame_idx);

                        // Convert raylib texture to ImGui texture
                        void* tex_id = reinterpret_cast<void*>(
                            static_cast<uintptr_t>(texture.id));

                        // Calculate UV coordinates for the sprite in the sheet
                        ImVec2 uv0(source.x / texture.width,
                                   source.y / texture.height);
                        ImVec2 uv1((source.x + source.width) / texture.width,
                                   (source.y + source.height) / texture.height);

                        // Display size (scaled for preview)
                        float scale = 2.0f;
                        ImVec2 size(sprite_cfg.sprite_width * scale,
                                    sprite_cfg.sprite_height * scale);

                        // Store the image position before drawing
                        ImVec2 image_pos = ImGui::GetCursorScreenPos();
                        ImGui::Image(tex_id, size, uv0, uv1);
                        ImGui::EndGroup();

                        // Show collision bounds overlay if enabled
                        if (show_collision_bounds_) {
                            auto& bounds = anim.collision_bounds;
                            if (bounds.is_valid()) {
                                ImDrawList* draw_list =
                                    ImGui::GetWindowDrawList();

                                ImVec2 bounds_min(
                                    image_pos.x + bounds.offset_x * scale,
                                    image_pos.y + bounds.offset_y * scale);
                                ImVec2 bounds_max(
                                    bounds_min.x + bounds.width * scale,
                                    bounds_min.y + bounds.height * scale);

                                draw_list->AddRect(bounds_min,
                                                   bounds_max,
                                                   IM_COL32(255, 0, 0, 255),
                                                   0.0f,
                                                   0,
                                                   2.0f);
                            }
                        }
                    } else {
                        ImGui::BeginGroup();
                        ImGui::Text("Frame %zu:", frame_idx);
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                                           "Out of bounds");
                        ImGui::EndGroup();
                    }

                    // Keep frames on the same line
                    if (frame_idx < sprite_cfg.frames.size() - 1) {
                        ImGui::SameLine();
                    }
                }

                ImGui::EndChild();
            } else {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                                   "No frames defined");
            }
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                               "Failed to load texture: %s",
                               sprite_cfg.filename.c_str());
        }
    } else {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                           "No sprite sheet selected");
    }
}

void AnimationPresetPanel::draw_preview() {
    ImGui::Text("Animation Preview");
    ImGui::Separator();

    // Check if we have a selected animation
    if (selected_animation_index_ < 0 ||
        selected_animation_index_ >=
            static_cast<int>(current_preset_.animations.size())) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                           "No animation selected");
        return;
    }

    auto& anim = current_preset_.animations[selected_animation_index_];
    auto& sprite_cfg = anim.sprite_config;

    // Check if we have frames
    if (sprite_cfg.frames.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                           "No frames defined for this animation");
        return;
    }

    // Load texture
    Texture2D texture = load_texture_cached(sprite_cfg.filename);
    if (texture.id <= 0) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                           "Failed to load texture: %s",
                           sprite_cfg.filename.c_str());
        return;
    }

    // Update animation timer if playing
    if (is_playing_) {
        frame_timer_ += GetFrameTime();
        if (frame_timer_ >= sprite_cfg.frame_duration) {
            frame_timer_ = 0.0f;
            current_frame_index_++;
            if (current_frame_index_ >=
                static_cast<int>(sprite_cfg.frames.size())) {
                if (sprite_cfg.loop) {
                    current_frame_index_ = 0;
                } else {
                    current_frame_index_ =
                        static_cast<int>(sprite_cfg.frames.size()) - 1;
                    is_playing_ = false;
                }
            }
        }
    }

    // Clamp frame index
    if (current_frame_index_ >= static_cast<int>(sprite_cfg.frames.size())) {
        current_frame_index_ = 0;
    }
    if (current_frame_index_ < 0) {
        current_frame_index_ = 0;
    }

    // Playback controls
    ImGui::Text("Playback Controls:");
    if (ImGui::Button(is_playing_ ? "Stop" : "Play")) {
        is_playing_ = !is_playing_;
        if (is_playing_ && current_frame_index_ >=
                               static_cast<int>(sprite_cfg.frames.size()) - 1) {
            current_frame_index_ = 0;
            frame_timer_ = 0.0f;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Previous Frame")) {
        is_playing_ = false;
        current_frame_index_--;
        if (current_frame_index_ < 0) {
            current_frame_index_ =
                static_cast<int>(sprite_cfg.frames.size()) - 1;
        }
        frame_timer_ = 0.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Next Frame")) {
        is_playing_ = false;
        current_frame_index_++;
        if (current_frame_index_ >=
            static_cast<int>(sprite_cfg.frames.size())) {
            current_frame_index_ = 0;
        }
        frame_timer_ = 0.0f;
    }

    ImGui::Text(
        "Frame: %d / %zu", current_frame_index_ + 1, sprite_cfg.frames.size());

    ImGui::Spacing();
    ImGui::Separator();

    // Preview settings
    ImGui::SliderFloat("Preview Scale", &preview_scale_, 0.5f, 5.0f);
    ImGui::Checkbox("Show Collision Bounds", &show_collision_bounds_);

    ImGui::Spacing();
    ImGui::Separator();

    // Get current frame
    const auto& frame = sprite_cfg.frames[current_frame_index_];

    // Calculate source rectangle from sprite sheet
    Rectangle source = {
        static_cast<float>(frame.col * sprite_cfg.sprite_width),
        static_cast<float>(frame.row * sprite_cfg.sprite_height),
        static_cast<float>(sprite_cfg.sprite_width),
        static_cast<float>(sprite_cfg.sprite_height)};

    // Check if frame is valid
    if (source.x + source.width > texture.width ||
        source.y + source.height > texture.height) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                           "Current frame is out of bounds!");
        return;
    }

    // Display the frame
    ImGui::Text("Current Frame Preview:");

    // Convert raylib texture to ImGui texture
    void* tex_id = reinterpret_cast<void*>(static_cast<uintptr_t>(texture.id));

    // Calculate UV coordinates for the sprite in the sheet
    ImVec2 uv0(source.x / texture.width, source.y / texture.height);
    ImVec2 uv1((source.x + source.width) / texture.width,
               (source.y + source.height) / texture.height);

    // Display size (scaled for preview)
    ImVec2 size(sprite_cfg.sprite_width * preview_scale_,
                sprite_cfg.sprite_height * preview_scale_);

    // Store position for drawing collision bounds
    ImVec2 image_pos = ImGui::GetCursorScreenPos();

    ImGui::Image(tex_id, size, uv0, uv1);

    // Draw collision bounds overlay if enabled
    if (show_collision_bounds_) {
        auto& bounds = anim.collision_bounds;
        if (bounds.is_valid()) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            ImVec2 bounds_min(image_pos.x + bounds.offset_x * preview_scale_,
                              image_pos.y + bounds.offset_y * preview_scale_);
            ImVec2 bounds_max(bounds_min.x + bounds.width * preview_scale_,
                              bounds_min.y + bounds.height * preview_scale_);

            draw_list->AddRect(bounds_min,
                               bounds_max,
                               IM_COL32(255, 0, 0, 255),
                               0.0f,
                               0,
                               2.0f);

            // Draw bounds info
            ImGui::Spacing();
            ImGui::Text("Collision Bounds:");
            ImGui::Text(
                "  Offset: (%.0f, %.0f)", bounds.offset_x, bounds.offset_y);
            ImGui::Text("  Size: %.0f x %.0f", bounds.width, bounds.height);
        } else {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                               "No collision bounds defined");
        }
    }
}

void AnimationPresetPanel::add_animation_state() {
    animation::AnimationStateConfig new_anim;
    new_anim.name = "new_animation";
    new_anim.state_id = current_preset_.animations.size() + 10;
    new_anim.sprite_config.filename = "sprite.png";
    new_anim.sprite_config.sprite_width = 64;
    new_anim.sprite_config.sprite_height = 64;
    new_anim.sprite_config.frame_duration = 0.1f;
    new_anim.sprite_config.loop = true;
    new_anim.sprite_config.frames.push_back({0, 0});

    current_preset_.animations.push_back(new_anim);
    selected_animation_index_ =
        static_cast<int>(current_preset_.animations.size()) - 1;

    std::strncpy(animation_name_buffer_,
                 new_anim.name.c_str(),
                 sizeof(animation_name_buffer_) - 1);
    std::strncpy(sprite_filename_buffer_,
                 new_anim.sprite_config.filename.c_str(),
                 sizeof(sprite_filename_buffer_) - 1);

    has_unsaved_changes_ = true;
}

void AnimationPresetPanel::remove_selected_animation() {
    if (selected_animation_index_ >= 0 &&
        selected_animation_index_ <
            static_cast<int>(current_preset_.animations.size())) {
        current_preset_.animations.erase(current_preset_.animations.begin() +
                                         selected_animation_index_);

        if (selected_animation_index_ >=
            static_cast<int>(current_preset_.animations.size())) {
            selected_animation_index_ =
                static_cast<int>(current_preset_.animations.size()) - 1;
        }

        has_unsaved_changes_ = true;
    }
}

void AnimationPresetPanel::duplicate_selected_animation() {
    if (selected_animation_index_ >= 0 &&
        selected_animation_index_ <
            static_cast<int>(current_preset_.animations.size())) {
        auto duplicate = current_preset_.animations[selected_animation_index_];
        duplicate.name += "_copy";
        duplicate.state_id += 1;

        current_preset_.animations.push_back(duplicate);
        selected_animation_index_ =
            static_cast<int>(current_preset_.animations.size()) - 1;

        has_unsaved_changes_ = true;
    }
}

void AnimationPresetPanel::create_new_preset(const std::string& preset_name) {
    current_preset_ = animation::AnimationPresetConfig();
    current_preset_.preset_name = preset_name;
    current_preset_.animations.clear();

    selected_animation_index_ = -1;
    current_filepath_.clear();
    has_unsaved_changes_ = false;

    std::strncpy(preset_name_buffer_,
                 preset_name.c_str(),
                 sizeof(preset_name_buffer_) - 1);
}

bool AnimationPresetPanel::load_preset(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;

        // Parse preset name
        if (j.contains("preset_name")) {
            current_preset_.preset_name = j["preset_name"].get<std::string>();
        }

        // Parse animations
        current_preset_.animations.clear();
        if (j.contains("animations") && j["animations"].is_array()) {
            for (const auto& anim_json : j["animations"]) {
                animation::AnimationStateConfig anim;

                if (anim_json.contains("name")) {
                    anim.name = anim_json["name"].get<std::string>();
                }
                if (anim_json.contains("state_id")) {
                    anim.state_id = anim_json["state_id"].get<int>();
                }

                // Load collision bounds
                if (anim_json.contains("collision_bounds")) {
                    const auto& bounds = anim_json["collision_bounds"];
                    if (bounds.contains("offset_x"))
                        anim.collision_bounds.offset_x =
                            bounds["offset_x"].get<float>();
                    if (bounds.contains("offset_y"))
                        anim.collision_bounds.offset_y =
                            bounds["offset_y"].get<float>();
                    if (bounds.contains("width"))
                        anim.collision_bounds.width =
                            bounds["width"].get<float>();
                    if (bounds.contains("height"))
                        anim.collision_bounds.height =
                            bounds["height"].get<float>();
                }

                // Load sprite config
                if (anim_json.contains("sprite_config")) {
                    const auto& sprite = anim_json["sprite_config"];
                    if (sprite.contains("filename"))
                        anim.sprite_config.filename =
                            sprite["filename"].get<std::string>();
                    if (sprite.contains("sprite_width"))
                        anim.sprite_config.sprite_width =
                            sprite["sprite_width"].get<int>();
                    if (sprite.contains("sprite_height"))
                        anim.sprite_config.sprite_height =
                            sprite["sprite_height"].get<int>();
                    if (sprite.contains("frame_duration"))
                        anim.sprite_config.frame_duration =
                            sprite["frame_duration"].get<float>();
                    if (sprite.contains("loop"))
                        anim.sprite_config.loop = sprite["loop"].get<bool>();

                    if (sprite.contains("frames") &&
                        sprite["frames"].is_array()) {
                        for (const auto& frame : sprite["frames"]) {
                            animation::FrameSpec fs;
                            if (frame.contains("row"))
                                fs.row = frame["row"].get<int>();
                            if (frame.contains("col"))
                                fs.col = frame["col"].get<int>();
                            anim.sprite_config.frames.push_back(fs);
                        }
                    }
                }

                current_preset_.animations.push_back(anim);
            }
        }

        current_filepath_ = filepath;
        has_unsaved_changes_ = false;
        selected_animation_index_ = -1;

        std::strncpy(preset_name_buffer_,
                     current_preset_.preset_name.c_str(),
                     sizeof(preset_name_buffer_) - 1);

        std::cout << "Loaded animation preset: " << filepath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing animation preset: " << e.what()
                  << std::endl;
        return false;
    }
}

bool AnimationPresetPanel::save_preset(const std::string& filepath) {
    nlohmann::json j;

    j["preset_name"] = current_preset_.preset_name;
    j["animations"] = nlohmann::json::array();

    for (const auto& anim : current_preset_.animations) {
        nlohmann::json anim_json;

        anim_json["name"] = anim.name;
        anim_json["state_id"] = anim.state_id;

        // Save collision bounds if valid
        if (anim.collision_bounds.is_valid()) {
            anim_json["collision_bounds"] = {
                {"offset_x", anim.collision_bounds.offset_x},
                {"offset_y", anim.collision_bounds.offset_y},
                {"width", anim.collision_bounds.width},
                {"height", anim.collision_bounds.height}};
        }

        // Save sprite config
        nlohmann::json sprite_json;

        // Convert absolute path to relative path
        // The saved path should be relative (e.g., "char1-run-Sheet.png")
        // so it works with both ASSETS_BASE_PATH="assets/" (Linux) and "/rd/"
        // (Dreamcast)
        std::string sprite_filename = anim.sprite_config.filename;

        // Get the real romdisk path (resolve the assets symlink)
        try {
            std::string romdisk_path =
                std::filesystem::canonical("assets").string();
            if (!romdisk_path.empty() && romdisk_path.back() != '/') {
                romdisk_path += '/';
            }

            // Debug output
            std::cout << "DEBUG save_preset:" << std::endl;
            std::cout << "  Original filename: " << sprite_filename
                      << std::endl;
            std::cout << "  Romdisk path: " << romdisk_path << std::endl;

            // If the filename is an absolute path within romdisk, make it
            // relative
            if (sprite_filename.find(romdisk_path) == 0) {
                sprite_filename = sprite_filename.substr(romdisk_path.length());
                std::cout << "  After conversion: " << sprite_filename
                          << std::endl;
            } else {
                std::cout
                    << "  No conversion - already relative or outside romdisk"
                    << std::endl;
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cout << "  Warning: Could not resolve assets path: "
                      << e.what() << std::endl;
        }

        sprite_json["filename"] = sprite_filename;
        sprite_json["sprite_width"] = anim.sprite_config.sprite_width;
        sprite_json["sprite_height"] = anim.sprite_config.sprite_height;
        sprite_json["frame_duration"] = anim.sprite_config.frame_duration;
        sprite_json["loop"] = anim.sprite_config.loop;

        sprite_json["frames"] = nlohmann::json::array();
        for (const auto& frame : anim.sprite_config.frames) {
            sprite_json["frames"].push_back(
                {{"row", frame.row}, {"col", frame.col}});
        }

        anim_json["sprite_config"] = sprite_json;
        j["animations"].push_back(anim_json);
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filepath
                  << std::endl;
        return false;
    }

    file << j.dump(2);
    file.close();

    current_filepath_ = filepath;
    has_unsaved_changes_ = false;

    std::cout << "Saved animation preset: " << filepath << std::endl;
    return true;
}

Texture2D AnimationPresetPanel::load_texture_cached(
    const std::string& filepath) {
    // Check if already in cache
    auto it = texture_cache_.find(filepath);
    if (it != texture_cache_.end() && it->second.id > 0) {
        return it->second;
    }

    // Try to load the texture
    Texture2D texture = {0};

    // Try as relative to ASSETS_BASE_PATH (the standard way for game assets)
    std::string full_path = udj::core::filesystem::get_assets_path(filepath);
    if (FileExists(full_path.c_str())) {
        texture = LoadTexture(full_path.c_str());
        if (texture.id > 0) {
            texture_cache_[filepath] = texture;
            return texture;
        }
    }

    // Try direct path (for absolute paths)
    if (FileExists(filepath.c_str())) {
        texture = LoadTexture(filepath.c_str());
        if (texture.id > 0) {
            texture_cache_[filepath] = texture;
            return texture;
        }
    }

    // Fallback: try to load anyway
    texture = LoadTexture(filepath.c_str());
    if (texture.id > 0) {
        texture_cache_[filepath] = texture;
    }

    return texture;
}

void AnimationPresetPanel::cleanup_textures() {
    for (auto& pair : texture_cache_) {
        if (pair.second.id > 0) {
            UnloadTexture(pair.second);
        }
    }
    texture_cache_.clear();
}

}  // namespace editor
}  // namespace udjourney
