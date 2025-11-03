// Copyright 2025 Quentin Cartier
#include "udjourney-editor/EditorScene.hpp"

struct EditorScene::PImpl {
    // Implementation details can be added here in the future
};

EditorScene::EditorScene() : pimpl_(std::make_unique<PImpl>()) {}

EditorScene::~EditorScene() = default;

void render_cursor_(Level& level, TilePanel& tile_panel, ImDrawList* draw_list,
                    const ImVec2& origin) {
    auto pos = ImGui::GetMousePos();
    ImVec2 cursor_pos = origin;
    draw_list->AddRectFilled(
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + tile_panel.get_platform_size().x * 50,
               pos.y + tile_panel.get_platform_size().y * 50),
        IM_COL32(255, 255, 0, 255));
}

void EditorScene::render(Level& level, TilePanel& tile_panel) {
    ImGuiIO& io = ImGui::GetIO();

    // Scene View window - docking system will manage position and size
    if (!ImGui::Begin("Scene View")) {
        ImGui::End();
        return;
    }

    // Get draw list and origin for rendering
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();

    // Render the grid (background)
    render_grid(level, draw_list, origin);

    // Render platforms on top of grid
    render_platforms(level, tile_panel, draw_list, origin);

    // Render player spawn position
    render_player_spawn(level, draw_list, origin);

    // Handle mouse input and selection based on current mode
    handle_mouse_input(level, tile_panel, draw_list, origin);

    render_cursor_(level, tile_panel, draw_list, origin);

    // Reserve space for ImGui layout
    ImGui::Dummy(
        ImVec2(level.col_cnt * tile_size_, level.row_cnt * tile_size_));

    ImGui::End();

    // Render platform context popup (must be outside the Scene View window)
    platform_popup_.render();

    // Handle platform deletion from popup
    if (platform_popup_.wants_to_delete_platform()) {
        EditorPlatform* to_delete = platform_popup_.get_platform_to_delete();
        if (to_delete) {
            // Clear selection if we're deleting the selected platform
            if (tile_panel.get_selected_platform() == to_delete) {
                tile_panel.set_selected_platform(nullptr);
            }
            level.remove_platform_at(to_delete->tile_x, to_delete->tile_y);
        }
        platform_popup_.close();
    }
}

void EditorScene::setup_scene_window(const ImGuiIO& io) {
    offset_y_ = ImGui::GetFrameHeight();

    ImGui::SetNextWindowPos(ImVec2(offset_x_, offset_y_), ImGuiCond_Always);
    ImGui::SetNextWindowSize(
        ImVec2(io.DisplaySize.x - offset_x_, io.DisplaySize.y - offset_y_),
        ImGuiCond_Always);
}

void EditorScene::render_grid(Level& level, ImDrawList* draw_list,
                              const ImVec2& origin) {
    for (int y = 0; y < static_cast<int>(level.row_cnt); ++y) {
        for (int x = 0; x < static_cast<int>(level.col_cnt); ++x) {
            ImVec2 top_left =
                ImVec2(origin.x + x * tile_size_, origin.y + y * tile_size_);
            ImVec2 bottom_right =
                ImVec2(top_left.x + tile_size_, top_left.y + tile_size_);

            // Draw filled tile with its color
            draw_list->AddRectFilled(top_left,
                                     bottom_right,
                                     level.tiles[y * level.col_cnt + x].color);

            // Draw grid lines
            draw_list->AddRect(
                top_left, bottom_right, IM_COL32(200, 200, 200, 255));
        }
    }
}

void EditorScene::handle_mouse_input(Level& level, TilePanel& tile_panel,
                                     ImDrawList* draw_list,
                                     const ImVec2& origin) {
    ImVec2 mouse_pos = ImGui::GetMousePos();
    bool hovered = ImGui::IsWindowHovered();
    bool left_clicked = ImGui::IsMouseClicked(0);
    bool right_clicked = ImGui::IsMouseClicked(1);

    if (!hovered || (!left_clicked && !right_clicked)) {
        // Still need to handle tile mode drag behavior
        if (tile_panel.get_edit_mode() == EditMode::Tiles) {
            handle_tile_mode_input(level, tile_panel, draw_list, origin);
        }
        return;
    }

    if (right_clicked) {
        // No action for right click in current modes except platform mode
        if (tile_panel.get_edit_mode() != EditMode::Platforms) {
            // Open context menu
            ImGui::OpenPopup("MyPopup");
            return;
        }
    }

    // Handle input based on current edit mode
    switch (tile_panel.get_edit_mode()) {
        case EditMode::Tiles:
            if (left_clicked) {
                handle_tile_mode_input(level, tile_panel, draw_list, origin);
            }
            break;
        case EditMode::Platforms:
            handle_platform_mode_input(level, tile_panel, mouse_pos, origin);
            break;
        case EditMode::PlayerSpawn:
            if (left_clicked) {
                handle_spawn_mode_input(level, mouse_pos, origin);
            }
            break;
    }
}

void EditorScene::handle_tile_mode_input(Level& level, TilePanel& tile_panel,
                                         ImDrawList* draw_list,
                                         const ImVec2& origin) {
    ImVec2 mouse_pos = ImGui::GetMousePos();
    bool hovered = ImGui::IsWindowHovered();
    bool clicked = ImGui::IsMouseClicked(0);
    bool released = ImGui::IsMouseReleased(0);

    // Handle selection start
    if (hovered && clicked) {
        selecting_ = true;
        selection_done_ = false;
        selection_start_ = mouse_pos;
        selection_end_ = mouse_pos;
    }

    // Update selection during drag
    if (selecting_) {
        selection_end_ = mouse_pos;
        if (released) {
            selecting_ = false;
            selection_done_ = true;
        }
    }

    // Render active selection
    if (selecting_) {
        render_selection(draw_list);
    }

    // Apply selection to tiles when done
    if (selection_done_) {
        apply_selection_to_tiles(level, tile_panel, draw_list, origin);
    }
}

void EditorScene::handle_platform_mode_input(Level& level,
                                             TilePanel& tile_panel,
                                             const ImVec2& mouse_pos,
                                             const ImVec2& origin) {
    ImVec2 tile_pos = screen_to_tile_pos(mouse_pos, origin);
    int tile_x = static_cast<int>(tile_pos.x);
    int tile_y = static_cast<int>(tile_pos.y);

    // Check bounds
    if (tile_x < 0 || tile_x >= static_cast<int>(level.col_cnt) || tile_y < 0 ||
        tile_y >= static_cast<int>(level.row_cnt)) {
        return;
    }

    // Find existing platform at this position
    // If any do not allow left click to add/replace
    EditorPlatform* existing_platform = nullptr;
    for (auto& platform : level.platforms) {
        if (platform.tile_x == tile_x && platform.tile_y == tile_y) {
            existing_platform = &platform;
            break;
        }
    }

    // Right click opens context menu for platform
    if (ImGui::IsMouseClicked(1)) {
        if (existing_platform) {
            platform_popup_.open(existing_platform);
        }
        return;
    }

    // Left click edits existing platform
    if (ImGui::IsMouseClicked(0) && existing_platform) {
        tile_panel.set_selected_platform(existing_platform);
        return;
    }

    // Left click adds new platform (if no existing platform)
    if (ImGui::IsMouseClicked(0) && !existing_platform) {
        EditorPlatform platform;
        platform.tile_x = tile_x;
        platform.tile_y = tile_y;

        auto [width, height] = tile_panel.get_platform_size();

        platform.width_tiles = width;
        platform.height_tiles = height;
        platform.behavior_type = tile_panel.get_platform_behavior();

        // Get selected features from tile panel
        platform.features = tile_panel.get_selected_features();

        // Calculate color based on behavior and features
        PlatformFeatureType primary_feature = platform.features.empty()
                                                  ? PlatformFeatureType::None
                                                  : platform.features[0];
        platform.color =
            get_platform_color(platform.behavior_type, primary_feature);

        // Remove existing platform at this position first
        if (existing_platform) {
            level.remove_platform_at(tile_x, tile_y);
            // Clear selection if we replaced the selected platform
            if (tile_panel.get_selected_platform() == existing_platform) {
                tile_panel.set_selected_platform(nullptr);
            }
        }

        level.add_platform(platform);
    }
}

void EditorScene::handle_spawn_mode_input(Level& level, const ImVec2& mouse_pos,
                                          const ImVec2& origin) {
    ImVec2 tile_pos = screen_to_tile_pos(mouse_pos, origin);
    int tile_x = static_cast<int>(tile_pos.x);
    int tile_y = static_cast<int>(tile_pos.y);

    // Check bounds
    if (tile_x < 0 || tile_x >= static_cast<int>(level.col_cnt) || tile_y < 0 ||
        tile_y >= static_cast<int>(level.row_cnt)) {
        return;
    }

    level.player_spawn_x = tile_x;
    level.player_spawn_y = tile_y;
}

void EditorScene::render_selection(ImDrawList* draw_list) {
    ImU32 fill_color = IM_COL32(0, 120, 255, 100);    // Semi-transparent fill
    ImU32 border_color = IM_COL32(0, 120, 255, 255);  // Solid border

    ImVec2 p_min = ImVec2(fminf(selection_start_.x, selection_end_.x),
                          fminf(selection_start_.y, selection_end_.y));
    ImVec2 p_max = ImVec2(fmaxf(selection_start_.x, selection_end_.x),
                          fmaxf(selection_start_.y, selection_end_.y));

    draw_list->AddRectFilled(p_min, p_max, fill_color);
    draw_list->AddRect(p_min, p_max, border_color, 0.0f, 0, 3.0f);
}

void EditorScene::apply_selection_to_tiles(Level& level, TilePanel& tile_panel,
                                           ImDrawList* draw_list,
                                           const ImVec2& origin) {
    ImVec2 p_min = ImVec2(fminf(selection_start_.x, selection_end_.x),
                          fminf(selection_start_.y, selection_end_.y));
    ImVec2 p_max = ImVec2(fmaxf(selection_start_.x, selection_end_.x),
                          fmaxf(selection_start_.y, selection_end_.y));

    for (int y = 0; y < static_cast<int>(level.row_cnt); ++y) {
        for (int x = 0; x < static_cast<int>(level.col_cnt); ++x) {
            ImVec2 tile_top_left =
                ImVec2(origin.x + x * tile_size_, origin.y + y * tile_size_);
            ImVec2 tile_bottom_right = ImVec2(tile_top_left.x + tile_size_,
                                              tile_top_left.y + tile_size_);

            // Check if tile is fully inside the selection rectangle
            if (is_tile_in_selection(
                    tile_top_left, tile_bottom_right, p_min, p_max)) {
                // Apply current color from tile panel
                level.tiles[y * level.col_cnt + x].color =
                    tile_panel.get_current_color();

                // Draw selection indicator
                draw_list->AddRect(
                    tile_top_left, tile_bottom_right, IM_COL32(255, 0, 0, 100));
            }
        }
    }
}

/** Renders spikes on the platform
 * @details
  This function draws spike indicators on the given platform area.
  It uses red triangles to represent spikes.
 * @param top_left Top-left corner of the platform
 * @param bottom_right Bottom-right corner of the platform
 * @param draw_list ImDrawList to render on
 *
*/
void render_spikes_(const ImVec2& top_left, const ImVec2& bottom_right,
                    ImDrawList& draw_list) {
    // Placeholder for spike rendering logic
    auto spike_count = 3;
    auto spike_height = bottom_right.y - top_left.y;
    auto indicator_offset_x = (bottom_right.x - top_left.x) / spike_count;

    for (int i = 0; i < spike_count; ++i) {
        draw_list.AddTriangleFilled(
            ImVec2(top_left.x + i * indicator_offset_x, top_left.y),
            ImVec2(top_left.x + (i + 1) * indicator_offset_x, top_left.y),
            ImVec2(top_left.x + i * indicator_offset_x + indicator_offset_x / 2,
                   top_left.y - spike_height),
            IM_COL32(255, 0, 0, 255) &
                IM_COL32(255, 255, 255, 150)  // Semi-transparent red
        );
    }
}

void render_checkpoint_(const ImVec2& top_left, const ImVec2& bottom_right,
                        ImDrawList& draw_list) {
    auto center_x = (top_left.x + bottom_right.x) / 2;
    auto flag_height = (bottom_right.y - top_left.y) * 1.4F;

    // Draw flag pole
    draw_list.AddLine(ImVec2(center_x, top_left.y),
                      ImVec2(center_x, top_left.y - flag_height),
                      IM_COL32(139, 69, 19, 255) & IM_COL32(255, 255, 255, 150),
                      4.0f);
    // Draw flag
    draw_list.AddRectFilled(
        ImVec2(center_x + 5, top_left.y - flag_height / 2 + 2),
        ImVec2(center_x + flag_height, top_left.y - flag_height),
        IM_COL32(0, 255, 0, 255) &
            IM_COL32(255, 255, 255, 150)  // Semi-transparent green
    );
}

void EditorScene::render_platforms(Level& level, TilePanel& tile_panel,
                                   ImDrawList* draw_list,
                                   const ImVec2& origin) {
    for (const auto& platform : level.platforms) {
        ImVec2 center =
            ImVec2(origin.x + platform.tile_x * tile_size_ + tile_size_ / 2,
                   origin.y + platform.tile_y * tile_size_ + tile_size_ / 2);

        // used to draw the center tile (for reference)
        ImVec2 unit_top_left =
            ImVec2(center.x - tile_size_ / 2, center.y - tile_size_ / 2);
        ImVec2 unit_bottom_right =
            ImVec2(origin.x + (platform.tile_x + 1) * tile_size_,
                   origin.y + (platform.tile_y + 1) * tile_size_);

        // use to draw the preview of the platform size
        ImVec2 top_left =
            ImVec2(center.x - platform.width_tiles * tile_size_ / 2,
                   center.y - platform.height_tiles * tile_size_ / 2);
        ImVec2 bottom_right =
            ImVec2(center.x + platform.width_tiles * tile_size_ / 2,
                   center.y + platform.height_tiles * tile_size_ / 2);

        // Draw platform with its color
        auto preview_color = platform.color;
        preview_color &= IM_COL32(255, 255, 255, 100);
        draw_list->AddRectFilled(top_left, bottom_right, preview_color);

        // Draw unit tile (for reference)
        draw_list->AddRectFilled(
            unit_top_left, unit_bottom_right, platform.color);

        // Draw platform border (highlighted if selected)
        bool is_selected = (tile_panel.get_selected_platform() == &platform);
        ImU32 border_color =
            is_selected ? IM_COL32(255, 255, 0, 255) : IM_COL32(0, 0, 0, 255);
        float border_thickness = is_selected ? 3.0f : 2.0f;
        draw_list->AddRect(
            top_left, bottom_right, border_color, 0.0f, 0, border_thickness);

        // Draw feature indicators
        if (!platform.features.empty()) {
            for (const auto& feature : platform.features) {
                switch (feature) {
                    case PlatformFeatureType::Spikes:
                        // Draw red triangles for spikes
                        render_spikes_(top_left, bottom_right, *draw_list);
                        break;
                    case PlatformFeatureType::Checkpoint:
                        // Draw green flag for checkpoint
                        render_checkpoint_(top_left, bottom_right, *draw_list);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void EditorScene::render_player_spawn(Level& level, ImDrawList* draw_list,
                                      const ImVec2& origin) {
    ImVec2 spawn_pos = ImVec2(origin.x + level.player_spawn_x * tile_size_,
                              origin.y + level.player_spawn_y * tile_size_);
    ImVec2 spawn_end =
        ImVec2(spawn_pos.x + tile_size_, spawn_pos.y + tile_size_);

    // Draw player spawn as a bright yellow circle
    ImVec2 center =
        ImVec2(spawn_pos.x + tile_size_ / 2, spawn_pos.y + tile_size_ / 2);
    draw_list->AddCircleFilled(
        center, tile_size_ / 3, IM_COL32(255, 255, 0, 200));
    draw_list->AddCircle(
        center, tile_size_ / 3, IM_COL32(255, 255, 0, 255), 0, 2.0f);
}

ImVec2 EditorScene::screen_to_tile_pos(const ImVec2& screen_pos,
                                       const ImVec2& origin) const {
    return ImVec2((screen_pos.x - origin.x) / tile_size_,
                  (screen_pos.y - origin.y) / tile_size_);
}

ImU32 EditorScene::get_platform_color(PlatformBehaviorType behavior,
                                      PlatformFeatureType feature) const {
    // Color based on feature first, then behavior
    switch (feature) {
        case PlatformFeatureType::Spikes:
            return IM_COL32(255, 0, 0, 255);  // Red for spikes
        case PlatformFeatureType::Checkpoint:
            return IM_COL32(255, 165, 0, 255);  // Orange for checkpoint
        default:
            break;
    }

    // Color based on behavior
    switch (behavior) {
        case PlatformBehaviorType::Horizontal:
            return IM_COL32(0, 255, 0, 255);  // Green
        case PlatformBehaviorType::EightTurnHorizontal:
            return IM_COL32(255, 165, 0, 255);  // Orange
        case PlatformBehaviorType::OscillatingSize:
            return IM_COL32(128, 0, 128, 255);  // Purple
        case PlatformBehaviorType::Static:
        default:
            return IM_COL32(0, 0, 255, 255);  // Blue
    }
}

bool EditorScene::is_tile_in_selection(const ImVec2& tile_top_left,
                                       const ImVec2& tile_bottom_right,
                                       const ImVec2& selection_min,
                                       const ImVec2& selection_max) const {
    return tile_top_left.x >= selection_min.x &&
           tile_bottom_right.x <= selection_max.x &&
           tile_top_left.y >= selection_min.y &&
           tile_bottom_right.y <= selection_max.y;
}