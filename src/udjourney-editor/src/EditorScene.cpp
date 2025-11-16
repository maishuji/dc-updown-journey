// Copyright 2025 Quentin Cartier
#include "udjourney-editor/EditorScene.hpp"

#include <raylib/raylib.h>

#include <cctype>

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

// Simple texture cache for editor
static std::unordered_map<std::string, Texture2D> texture_cache;

Texture2D load_texture_cached(const std::string& filename) {
    auto iter = texture_cache.find(filename);
    if (iter != texture_cache.end()) {
        return iter->second;
    }

    // Load texture from assets directory
    std::string full_path = "assets/" + filename;
    Texture2D texture = LoadTexture(full_path.c_str());
    if (texture.id != 0) {  // Valid texture
        texture_cache[filename] = texture;
    }
    return texture;
}

// Forward declarations
void render_monster_cursor_preview(TilePanel& tile_panel, ImDrawList* draw_list,
                                   const ImVec2& mouse_pos,
                                   const ImVec2& origin, float tile_size);

struct EditorScene::PImpl {
    // Implementation details can be added here in the future
};

EditorScene::EditorScene() : pimpl_(std::make_unique<PImpl>()) {}

EditorScene::~EditorScene() = default;

void render_cursor_(Level& level, TilePanel& tile_panel, ImDrawList* draw_list,
                    const ImVec2& origin) {
    auto pos = ImGui::GetMousePos();
    ImVec2 cursor_pos = origin;

    // Handle different edit modes
    switch (tile_panel.get_edit_mode()) {
        case EditMode::Monsters: {
            // Show monster sprite preview for monster mode
            render_monster_cursor_preview(tile_panel,
                                          draw_list,
                                          pos,
                                          origin,
                                          32.0f);  // Use constant tile size
            break;
        }
        case EditMode::Platforms: {
            // Show platform preview
            draw_list->AddRectFilled(
                ImVec2(pos.x, pos.y),
                ImVec2(pos.x + tile_panel.get_platform_size().x * 50,
                       pos.y + tile_panel.get_platform_size().y * 50),
                IM_COL32(255, 255, 0, 128));  // Semi-transparent yellow
            break;
        }
        default:
            // Default cursor for other modes
            break;
    }
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

    // Render monsters
    render_monsters(level, tile_panel, draw_list, origin);

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
        case EditMode::Monsters:
            handle_monster_mode_input(level,
                                      tile_panel,
                                      mouse_pos,
                                      origin,
                                      left_clicked,
                                      right_clicked);
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
                IM_COL32(255, 255, 255, 150));  // Semi-transparent red
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
            IM_COL32(255, 255, 255, 150));  // Semi-transparent green
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

void EditorScene::render_monsters(Level& level, TilePanel& tile_panel,
                                  ImDrawList* draw_list, const ImVec2& origin) {
    for (const auto& monster : level.monsters) {
        // Calculate monster position on screen (consistent with grid and
        // platforms)
        ImVec2 monster_pos = ImVec2(origin.x + monster.tile_x * tile_size_,
                                    origin.y + monster.tile_y * tile_size_);

        // Draw monster as a filled circle
        draw_list->AddCircleFilled(ImVec2(monster_pos.x + tile_size_ / 2,
                                          monster_pos.y + tile_size_ / 2),
                                   tile_size_ / 3,  // Radius
                                   monster.color,
                                   12);

        // Draw monster type indicator (first letter)
        char type_char = monster.preset_name.empty()
                             ? '?'
                             : std::toupper(monster.preset_name[0]);
        draw_list->AddText(ImVec2(monster_pos.x + tile_size_ / 2 - 4,
                                  monster_pos.y + tile_size_ / 2 - 6),
                           IM_COL32(255, 255, 255, 255),  // White text
                           &type_char,
                           &type_char + 1);

        // Highlight selected monster
        if (tile_panel.get_selected_monster() == &monster) {
            draw_list->AddCircle(
                ImVec2(monster_pos.x + tile_size_ / 2,
                       monster_pos.y + tile_size_ / 2),
                tile_size_ / 2,              // Outer highlight circle
                IM_COL32(255, 255, 0, 255),  // Yellow highlight
                12,                          // Segments
                3.0f);
        }
    }
}

void EditorScene::handle_monster_mode_input(Level& level, TilePanel& tile_panel,
                                            const ImVec2& mouse_pos,
                                            const ImVec2& origin,
                                            bool left_clicked,
                                            bool right_clicked) {
    // Handle deletion flag from TilePanel
    if (tile_panel.should_delete_selected_monster() &&
        tile_panel.get_selected_monster()) {
        const auto* monster_to_delete = tile_panel.get_selected_monster();
        level.remove_monster_at(monster_to_delete->tile_x,
                                monster_to_delete->tile_y);
        tile_panel.clear_delete_flag();
        return;
    }

    // Convert mouse position to tile coordinates
    ImVec2 tile_pos = screen_to_tile_pos(mouse_pos, origin);
    int tile_x = static_cast<int>(std::floor(tile_pos.x));
    int tile_y = static_cast<int>(std::floor(tile_pos.y));

    // Ensure coordinates are within bounds
    if (tile_x < 0 || tile_y < 0 || tile_x >= static_cast<int>(level.col_cnt) ||
        tile_y >= static_cast<int>(level.row_cnt)) {
        return;
    }

    if (left_clicked) {
        // Check if there's already a monster at this position
        EditorMonster* existing_monster = level.get_monster_at(tile_x, tile_y);

        if (existing_monster) {
            // Select existing monster for editing
            tile_panel.set_selected_monster(existing_monster);
        } else {
            // Create new monster
            EditorMonster new_monster;
            new_monster.tile_x = tile_x;
            new_monster.tile_y = tile_y;
            new_monster.preset_name = tile_panel.get_selected_monster_preset();

            // Set color based on preset
            if (new_monster.preset_name == "goblin") {
                new_monster.color = IM_COL32(255, 0, 0, 255);  // Red
            } else if (new_monster.preset_name == "spider") {
                new_monster.color = IM_COL32(128, 0, 128, 255);  // Purple
            } else {
                new_monster.color = IM_COL32(255, 0, 0, 255);  // Default red
            }

            level.add_monster(new_monster);

            // Select the newly created monster
            EditorMonster* added_monster = level.get_monster_at(tile_x, tile_y);
            tile_panel.set_selected_monster(added_monster);
        }
    }

    if (right_clicked) {
        // Remove monster at this position
        level.remove_monster_at(tile_x, tile_y);

        // Clear selection if we deleted the selected monster
        if (tile_panel.get_selected_monster() &&
            tile_panel.get_selected_monster()->tile_x == tile_x &&
            tile_panel.get_selected_monster()->tile_y == tile_y) {
            tile_panel.set_selected_monster(nullptr);
        }
    }
}

void render_monster_cursor_preview(TilePanel& tile_panel, ImDrawList* draw_list,
                                   const ImVec2& mouse_pos,
                                   const ImVec2& origin, float tile_size) {
    // Get the selected monster preset
    const std::string& selected_preset =
        tile_panel.get_selected_monster_preset();
    if (selected_preset.empty()) {
        return;  // No preset selected
    }

    // Convert mouse position to tile coordinates (manual calculation since
    // we're outside the class)
    ImVec2 relative_pos =
        ImVec2(mouse_pos.x - origin.x, mouse_pos.y - origin.y);
    int tile_x = static_cast<int>(relative_pos.x / tile_size);
    int tile_y = static_cast<int>(relative_pos.y / tile_size);

    // Calculate the preview position (centered on tile)
    ImVec2 preview_pos =
        ImVec2(origin.x + tile_x * tile_size, origin.y + tile_y * tile_size);

    try {
        // Try to load monster preset configuration
        std::string monster_preset_path =
            "assets/monsters/" + selected_preset + ".json";

        if (!std::filesystem::exists(monster_preset_path)) {
            // Fallback: draw a simple colored circle
            draw_list->AddCircleFilled(
                ImVec2(preview_pos.x + tile_size / 2,
                       preview_pos.y + tile_size / 2),
                tile_size / 3,
                IM_COL32(255, 255, 0, 128),  // Semi-transparent yellow
                12);
            return;
        }

        // Load monster preset JSON
        std::ifstream file(monster_preset_path);
        if (!file.is_open()) {
            return;
        }

        nlohmann::json preset_json;
        file >> preset_json;

        const auto kAnimPresetKey = "animation_preset";

        // Find the animation config file
        if (!preset_json.contains(kAnimPresetKey)) {
            return;
        }

        std::string anim_config_file =
            preset_json[kAnimPresetKey].get<std::string>();
        std::string anim_config_path = "assets/animations/" + anim_config_file;

        if (!std::filesystem::exists(anim_config_path)) {
            return;
        }

        // Load animation configuration
        std::ifstream anim_file(anim_config_path);
        if (!anim_file.is_open()) {
            return;
        }

        nlohmann::json anim_json;
        anim_file >> anim_json;

        // Find the idle animation (state_id = 10)
        std::string sprite_filename;
        int sprite_width = 64;
        int sprite_height = 64;
        int idle_row = 0;
        int idle_col = 0;

        if (anim_json.contains("animations")) {
            for (const auto& anim : anim_json["animations"]) {
                if (anim.contains("state_id") &&
                    anim["state_id"].get<int>() == 10) {  // ANIM_IDLE
                    // Found idle animation
                    if (anim.contains("sprite_config")) {
                        const auto& sprite_config = anim["sprite_config"];

                        if (sprite_config.contains("filename")) {
                            sprite_filename =
                                sprite_config["filename"].get<std::string>();
                        }
                        if (sprite_config.contains("sprite_width")) {
                            sprite_width =
                                sprite_config["sprite_width"].get<int>();
                        }
                        if (sprite_config.contains("sprite_height")) {
                            sprite_height =
                                sprite_config["sprite_height"].get<int>();
                        }

                        // Get the first frame position (idle state first frame)
                        if (sprite_config.contains("frames") &&
                            sprite_config["frames"].is_array()) {
                            const auto& frames = sprite_config["frames"];
                            if (!frames.empty()) {
                                const auto& first_frame = frames[0];
                                if (first_frame.contains("row")) {
                                    idle_row = first_frame["row"].get<int>();
                                }
                                if (first_frame.contains("col")) {
                                    idle_col = first_frame["col"].get<int>();
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }

        // Load the sprite texture using simple cache
        if (!sprite_filename.empty()) {
            Texture2D sprite_texture = load_texture_cached(sprite_filename);

            if (sprite_texture.id != 0) {  // Valid texture
                // Calculate source rectangle for the first frame of idle
                // animation
                Rectangle source_rect = {
                    static_cast<float>(idle_col * sprite_width),
                    static_cast<float>(idle_row * sprite_height),
                    static_cast<float>(sprite_width),
                    static_cast<float>(sprite_height)};

                // Calculate destination rectangle (fit to tile size)
                Rectangle dest_rect = {preview_pos.x,  // Align to tile boundary
                                       preview_pos.y,  // Align to tile boundary
                                       tile_size,
                                       tile_size};

                // Use ImGui's DrawList to render the texture
                // Convert raylib texture to ImGui texture ID (OpenGL texture
                // ID)
                ImTextureID img_id =
                    static_cast<ImTextureID>(sprite_texture.id);

                // Calculate UV coordinates for the sprite frame
                ImVec2 uv0 = ImVec2(source_rect.x / sprite_texture.width,
                                    source_rect.y / sprite_texture.height);
                ImVec2 uv1 = ImVec2(
                    (source_rect.x + source_rect.width) / sprite_texture.width,
                    (source_rect.y + source_rect.height) /
                        sprite_texture.height);

                // Draw the sprite with some transparency to indicate it's a
                // preview
                draw_list->AddImage(
                    img_id,
                    ImVec2(dest_rect.x, dest_rect.y),
                    ImVec2(dest_rect.x + dest_rect.width,
                           dest_rect.y + dest_rect.height),
                    uv0,
                    uv1,
                    IM_COL32(
                        255, 255, 255, 180));  // Semi-transparent white tint
                return;
            }
        }
    } catch (const std::exception& e) {
        // Error occurred, fall back to simple preview
    }

    // Fallback: draw a simple colored circle if sprite loading failed
    draw_list->AddCircleFilled(
        ImVec2(preview_pos.x + tile_size / 2, preview_pos.y + tile_size / 2),
        tile_size / 3,
        IM_COL32(255, 255, 0, 128),  // Semi-transparent yellow
        12);
}
