// Copyright 2025 Quentin Cartier
#include "udjourney-editor/EditorScene.hpp"

#include <raylib/raylib.h>

#include <cctype>
#include <cmath>

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>
#include "udj-core/CoreUtils.hpp"

// Simple texture cache for editor
static std::unordered_map<std::string, Texture2D> texture_cache;

// Helper function to draw dashed lines
static void draw_dashed_line(ImDrawList* draw_list, const ImVec2& p1,
                             const ImVec2& p2, ImU32 color, float thickness,
                             float dash_size) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float length = std::sqrt(dx * dx + dy * dy);

    if (length < 0.001f) return;

    float nx = dx / length;
    float ny = dy / length;

    float pos = 0.0f;
    bool draw_dash = true;

    while (pos < length) {
        float segment_length = std::min(dash_size, length - pos);

        if (draw_dash) {
            ImVec2 start(p1.x + nx * pos, p1.y + ny * pos);
            ImVec2 end(p1.x + nx * (pos + segment_length),
                       p1.y + ny * (pos + segment_length));
            draw_list->AddLine(start, end, color, thickness);
        }

        pos += segment_length;
        draw_dash = !draw_dash;
    }
}

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

void EditorScene::render(Level& level, TilePanel& tile_panel,
                         BackgroundManager* bg_manager,
                         BackgroundObjectPresetManager* bg_preset_manager) {
    ImGuiIO& io = ImGui::GetIO();

    // Scene View window - docking system will manage position and size
    if (!ImGui::Begin("Scene View")) {
        ImGui::End();
        return;
    }

    // Get draw list and origin for rendering
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();

    // Render the grid (background) - skip when placing background objects
    bool is_placing_bg = tile_panel.get_edit_mode() == EditMode::Background &&
                         tile_panel.is_background_placing_mode();
    if (!is_placing_bg) {
        render_grid(level, draw_list, origin);
    }

    // Render background objects first (behind everything)
    if (bg_manager) {
        render_background(bg_manager, draw_list, origin, level);
    }

    // Render platforms on top of grid
    render_platforms(level, tile_panel, draw_list, origin);

    // Render monsters
    render_monsters(level, tile_panel, draw_list, origin);

    // Render player spawn position
    render_player_spawn(level, draw_list, origin);

    // Handle mouse input and selection based on current mode
    handle_mouse_input(level, tile_panel, draw_list, origin);

    // Show background object preview if in placing mode
    if (bg_manager && bg_preset_manager &&
        tile_panel.get_edit_mode() == EditMode::Background &&
        tile_panel.is_background_placing_mode()) {
        render_background_placement_preview(bg_manager,
                                            bg_preset_manager,
                                            tile_panel,
                                            draw_list,
                                            origin,
                                            level);
    }

    // Handle background object selection when NOT in placement mode
    if (bg_manager && tile_panel.get_edit_mode() == EditMode::Background &&
        !tile_panel.is_background_placing_mode() && ImGui::IsWindowHovered()) {
        const auto& layers = bg_manager->get_layers();
        ImVec2 mouse_pos = ImGui::GetMousePos();
        bool left_clicked = ImGui::IsMouseClicked(0);
        bool right_clicked = ImGui::IsMouseClicked(1);
        bool delete_pressed = ImGui::IsKeyPressed(ImGuiKey_Delete);

        constexpr float SCREEN_WIDTH = 640.0f;
        float bg_horizontal_offset = SCREEN_WIDTH / 2.0f;

        // Check for object selection on left click or right click
        if (left_clicked || right_clicked) {
            bool found = false;
            // Search in reverse order (top to bottom) to select topmost object
            for (int layer_idx = layers.size() - 1; layer_idx >= 0 && !found;
                 --layer_idx) {
                const auto& layer_objects = layers[layer_idx].get_objects();
                for (int obj_idx = layer_objects.size() - 1;
                     obj_idx >= 0 && !found;
                     --obj_idx) {
                    const auto& obj = layer_objects[obj_idx];
                    float scaled_size = obj.tile_size * obj.scale;
                    ImVec2 obj_screen_pos(
                        origin.x + obj.x - bg_horizontal_offset,
                        origin.y + obj.y);

                    // Check if mouse is within object bounds
                    if (mouse_pos.x >= obj_screen_pos.x &&
                        mouse_pos.x <= obj_screen_pos.x + scaled_size &&
                        mouse_pos.y >= obj_screen_pos.y &&
                        mouse_pos.y <= obj_screen_pos.y + scaled_size) {
                        selected_bg_layer_idx_ = layer_idx;
                        selected_bg_object_idx_ = obj_idx;
                        found = true;

                        // Open context menu on right click
                        if (right_clicked) {
                            ImGui::OpenPopup("BackgroundObjectContextMenu");
                        }
                    }
                }
            }
            // Clear selection if clicked on empty space
            if (!found && left_clicked) {
                selected_bg_layer_idx_ = -1;
                selected_bg_object_idx_ = -1;
            }
        }

        // Delete selected object on Delete key
        if (delete_pressed && selected_bg_layer_idx_ >= 0 &&
            selected_bg_object_idx_ >= 0) {
            bg_manager->remove_object(selected_bg_layer_idx_,
                                      selected_bg_object_idx_);
            selected_bg_layer_idx_ = -1;
            selected_bg_object_idx_ = -1;
        }
    }

    render_cursor_(level, tile_panel, draw_list, origin);

    // Reserve space for ImGui layout
    ImGui::Dummy(
        ImVec2(level.col_cnt * tile_size_, level.row_cnt * tile_size_));

    // Background object context menu (must be inside Scene View window)
    if (ImGui::BeginPopup("BackgroundObjectContextMenu")) {
        if (selected_bg_layer_idx_ >= 0 && selected_bg_object_idx_ >= 0 &&
            bg_manager) {
            ImGui::Text("Background Object");
            ImGui::Separator();

            if (ImGui::MenuItem("Delete", "Delete")) {
                bg_manager->remove_object(selected_bg_layer_idx_,
                                          selected_bg_object_idx_);
                selected_bg_layer_idx_ = -1;
                selected_bg_object_idx_ = -1;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }

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

    // Render screen bounds outline
    ImVec2 screen_top_left = ImVec2(origin.x, origin.y);
    ImVec2 screen_bottom_right =
        ImVec2(origin.x + 640, origin.y + level.row_cnt * tile_size_);
    draw_list->AddRect(screen_top_left,
                       screen_bottom_right,
                       IM_COL32(255, 0, 0, 255),
                       0.0f,
                       0,
                       2.0f);
}

void EditorScene::setup_scene_window(const ImGuiIO& io) {
    offset_y_ = ImGui::GetFrameHeight();

    ImGui::SetNextWindowPos(ImVec2(offset_x_, offset_y_), ImGuiCond_Always);
    ImGui::SetNextWindowSize(
        ImVec2(io.DisplaySize.x - offset_x_, io.DisplaySize.y - offset_y_),
        ImGuiCond_Always);
}

void EditorScene::render_background(BackgroundManager* bg_manager,
                                    ImDrawList* draw_list, const ImVec2& origin,
                                    const Level& level) {
    if (!bg_manager) return;

    const auto& layers = bg_manager->get_layers();
    auto selected = bg_manager->get_selected_layer();

    // Draw spannable bounds for selected layer
    if (selected.has_value() && selected.value() < layers.size()) {
        const auto& selected_layer = layers[selected.value()];
        float parallax_factor = selected_layer.get_parallax_factor();

        // Calculate spannable area based on parallax
        // Scene dimensions
        constexpr float SCREEN_WIDTH = 640.0f;
        constexpr float SCREEN_HEIGHT = 480.0f;
        float level_height = level.row_cnt * tile_size_;

        // For vertical scrolling:
        // max_scroll = level_height - SCREEN_HEIGHT (if level is taller than
        // screen)
        float max_scroll = std::max(0.0f, level_height - SCREEN_HEIGHT);

        // Background rendering: screen_y = obj.y - camera_y * (1 -
        // parallax_factor) For object to be visible: 0 <= screen_y <=
        // SCREEN_HEIGHT When camera = 0: need obj.y >= 0 and obj.y <=
        // SCREEN_HEIGHT When camera = max_scroll: need obj.y >= max_scroll * (1
        // - parallax_factor)
        //                           and obj.y <= max_scroll * (1 -
        //                           parallax_factor) + SCREEN_HEIGHT
        //
        // So spannable range is: [0, max_scroll * (1 - parallax_factor) +
        // SCREEN_HEIGHT]
        //
        // With parallax_factor = 0.0 (follows camera): spannable = level_height
        // (full scene) With parallax_factor = 1.0 (static): spannable =
        // SCREEN_HEIGHT (just screen)
        // With parallax_factor = 0.5: spannable = level_height * 0.5 +
        // SCREEN_HEIGHT
        //
        // Add small margin (10% of screen height) for smooth coverage at edges
        float base_spannable_height =
            max_scroll * (1.0f - parallax_factor) + SCREEN_HEIGHT;
        float spannable_height =
            base_spannable_height + SCREEN_HEIGHT * 0.1f;  // Small margin

        // Center background horizontally - offset by half screen width
        // This makes the center of the scene (320px) align with center of
        // background
        float bg_horizontal_offset = SCREEN_WIDTH / 2.0f;

        // Draw spannable bounds rectangle (centered)
        ImVec2 bounds_min = ImVec2(origin.x - bg_horizontal_offset, origin.y);
        ImVec2 bounds_max =
            ImVec2(origin.x + SCREEN_WIDTH + bg_horizontal_offset,
                   origin.y + spannable_height);

        // Draw semi-transparent fill
        draw_list->AddRectFilled(
            bounds_min, bounds_max, IM_COL32(100, 150, 255, 30));

        // Draw border
        draw_list->AddRect(bounds_min,
                           bounds_max,
                           IM_COL32(100, 150, 255, 200),
                           0.0f,
                           0,
                           2.0f);

        // Draw exact mapping bounds (without margin) as a dashed line
        ImVec2 exact_bounds_min =
            ImVec2(origin.x - bg_horizontal_offset, origin.y);
        ImVec2 exact_bounds_max =
            ImVec2(origin.x + SCREEN_WIDTH + bg_horizontal_offset,
                   origin.y + base_spannable_height);

        // Draw dashed rectangle for exact bounds
        draw_dashed_line(draw_list,
                         ImVec2(exact_bounds_min.x, exact_bounds_max.y),
                         ImVec2(exact_bounds_max.x, exact_bounds_max.y),
                         IM_COL32(255, 255, 0, 200),
                         2.0f,
                         8.0f);  // Yellow dashed line at bottom

        // Draw label with parallax info
        char label[128];
        snprintf(label,
                 sizeof(label),
                 "Layer: %s\nParallax: %.2f\nSpannable: %.0f x %.0f",
                 selected_layer.get_name().c_str(),
                 parallax_factor,
                 SCREEN_WIDTH,
                 spannable_height);
        draw_list->AddText(ImVec2(bounds_min.x + 5, bounds_min.y + 5),
                           IM_COL32(255, 255, 255, 255),
                           label);

        // Draw parallax mapping guide lines every 5 tiles
        // These show which part of background is visible at different scene
        // positions
        constexpr float TILE_SIZE = 32.0f;
        constexpr int LINE_INTERVAL = 5;  // Draw line every 5 tiles

        for (int tile_y = 0; tile_y <= static_cast<int>(level.row_cnt);
             tile_y += LINE_INTERVAL) {
            float scene_y = tile_y * TILE_SIZE;

            // Skip if beyond level bounds
            if (scene_y > level_height) break;

            // Calculate camera position at this scene position
            // Camera follows player, which is at this y position
            float camera_y = std::max(0.0f, std::min(scene_y, max_scroll));

            // Calculate where this maps to on the background
            // background_y = scene_y - camera_y * (1 - parallax_factor)
            float bg_y = camera_y * (1.0f - parallax_factor);

            // Draw dashed line from scene position to background position
            // Background is centered, so extend lines to show full width
            ImVec2 scene_left(origin.x, origin.y + scene_y);
            ImVec2 scene_right(origin.x + SCREEN_WIDTH, origin.y + scene_y);
            ImVec2 bg_left(origin.x - bg_horizontal_offset, origin.y + bg_y);
            ImVec2 bg_right(origin.x + SCREEN_WIDTH + bg_horizontal_offset,
                            origin.y + bg_y);

            // Color based on position
            ImU32 line_color = IM_COL32(255, 200, 100, 150);

            // Draw dashed horizontal line on scene (red border area)
            draw_dashed_line(
                draw_list, scene_left, scene_right, line_color, 2.0f, 10.0f);

            // Draw dashed horizontal line on background (blue spannable area)
            draw_dashed_line(
                draw_list, bg_left, bg_right, line_color, 2.0f, 10.0f);

            // Draw connecting vertical line showing the mapping
            ImVec2 connect_start(origin.x + SCREEN_WIDTH + 5,
                                 origin.y + scene_y);
            ImVec2 connect_end(origin.x + SCREEN_WIDTH + 5, origin.y + bg_y);
            draw_dashed_line(draw_list,
                             connect_start,
                             connect_end,
                             IM_COL32(150, 255, 150, 180),
                             1.5f,
                             8.0f);

            // Draw label showing the tile position
            char pos_label[32];
            snprintf(pos_label, sizeof(pos_label), "T%d", tile_y);
            draw_list->AddText(
                ImVec2(origin.x + SCREEN_WIDTH + 10, origin.y + scene_y - 8),
                IM_COL32(255, 255, 255, 200),
                pos_label);
        }
    }

    // Render layers in depth order (already sorted by BackgroundManager)
    for (const auto& layer : layers) {
        const auto& objects = layer.get_objects();

        for (const auto& obj : objects) {
            // Skip if no sprite sheet specified
            if (obj.sprite_sheet.empty()) continue;

            // Load texture from cache
            Texture2D texture = load_texture_cached(obj.sprite_sheet);
            if (texture.id == 0) continue;  // Skip if texture failed to load

            // Calculate source rectangle (tile in sprite sheet)
            Rectangle source = {
                static_cast<float>(obj.tile_col * obj.tile_size),
                static_cast<float>(obj.tile_row * obj.tile_size),
                static_cast<float>(obj.tile_size),
                static_cast<float>(obj.tile_size)};

            // Calculate destination position and size
            // Center background horizontally by offsetting left by half screen
            // width
            constexpr float SCREEN_WIDTH = 640.0f;
            float bg_horizontal_offset = SCREEN_WIDTH / 2.0f;
            float scaled_size = obj.tile_size * obj.scale;
            ImVec2 pos = ImVec2(origin.x + obj.x - bg_horizontal_offset,
                                origin.y + obj.y);
            ImVec2 size = ImVec2(scaled_size, scaled_size);

            // Convert Raylib texture to ImGui UV coordinates
            ImVec2 uv0(source.x / texture.width, source.y / texture.height);
            ImVec2 uv1((source.x + source.width) / texture.width,
                       (source.y + source.height) / texture.height);

            // Draw the sprite
            draw_list->AddImage(static_cast<ImTextureID>(texture.id),
                                pos,
                                ImVec2(pos.x + size.x, pos.y + size.y),
                                uv0,
                                uv1,
                                IM_COL32(255, 255, 255, 255));

            // Draw bounding box for selected layer
            auto selected = bg_manager->get_selected_layer();

            auto yellow = IM_COL32(255, 255, 0, 200);

            if (selected.has_value()) {
                if (&layer == &layers[selected.value()]) {
                    draw_list->AddRect(pos,
                                       ImVec2(pos.x + size.x, pos.y + size.y),
                                       yellow,
                                       0.0f,
                                       0,
                                       2.0f);
                }
            }
        }
    }

    // Draw selection indicator for selected background object
    if (bg_manager && selected_bg_layer_idx_ >= 0 &&
        selected_bg_object_idx_ >= 0) {
        const auto& layers = bg_manager->get_layers();
        constexpr float SCREEN_WIDTH = 640.0f;
        float bg_horizontal_offset = SCREEN_WIDTH / 2.0f;

        if (selected_bg_layer_idx_ < static_cast<int>(layers.size())) {
            const auto& layer_objects =
                layers[selected_bg_layer_idx_].get_objects();
            if (selected_bg_object_idx_ <
                static_cast<int>(layer_objects.size())) {
                const auto& obj = layer_objects[selected_bg_object_idx_];
                float scaled_size = obj.tile_size * obj.scale;
                ImVec2 obj_screen_pos(origin.x + obj.x - bg_horizontal_offset,
                                      origin.y + obj.y);

                // Draw bright selection outline
                draw_list->AddRect(obj_screen_pos,
                                   ImVec2(obj_screen_pos.x + scaled_size,
                                          obj_screen_pos.y + scaled_size),
                                   IM_COL32(0, 255, 0, 255),
                                   0.0f,
                                   0,
                                   3.0f);
            }
        }
    }

    // DRAW CENTER LINE LAST (so it appears on top of everything)
    if (selected.has_value() && selected.value() < layers.size()) {
        const auto& selected_layer = layers[selected.value()];
        float parallax_factor = selected_layer.get_parallax_factor();

        constexpr float SCREEN_WIDTH = 640.0f;
        constexpr float SCREEN_HEIGHT = 480.0f;
        float level_height = level.row_cnt * tile_size_;
        float max_scroll = std::max(0.0f, level_height - SCREEN_HEIGHT);
        float base_spannable_height =
            max_scroll * (1.0f - parallax_factor) + SCREEN_HEIGHT;
        float spannable_height = base_spannable_height + SCREEN_HEIGHT * 0.1f;
        float bg_horizontal_offset = SCREEN_WIDTH / 2.0f;

        // Scene center: middle of the red-bordered platform area
        float scene_center_x = origin.x + SCREEN_WIDTH / 2.0f;
        float scene_center_y = origin.y + level_height / 2.0f;

        // Background layer center: middle of the blue spannable area
        // Horizontally: center of spannable width
        float bg_center_x = origin.x + bg_horizontal_offset;
        float bg_center_y = origin.y + spannable_height / 2.0f;

        // Draw pink diagonal line connecting the two centers
        draw_list->AddLine(ImVec2(scene_center_x, scene_center_y),
                           ImVec2(bg_center_x, bg_center_y),
                           IM_COL32(255, 100, 200, 220),
                           3.0f);

        // Add "C" label in the middle of the line
        draw_list->AddText(ImVec2((scene_center_x + bg_center_x) / 2.0f - 5,
                                  (scene_center_y + bg_center_y) / 2.0f - 10),
                           IM_COL32(255, 100, 200, 255),
                           "C");

        // Add "Sc" label at scene center endpoint
        draw_list->AddText(ImVec2(scene_center_x + 5, scene_center_y - 10),
                           IM_COL32(255, 100, 200, 255),
                           "Sc");

        // Add "Bg" label at background center endpoint
        draw_list->AddText(ImVec2(bg_center_x + 5, bg_center_y - 10),
                           IM_COL32(255, 100, 200, 255),
                           "Bg");
    }
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
        case EditMode::Background:
            if (left_clicked) {
                handle_background_mode_input(tile_panel, mouse_pos, origin);
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

void EditorScene::handle_background_mode_input(TilePanel& tile_panel,
                                               const ImVec2& mouse_pos,
                                               const ImVec2& origin) {
    // This will be called when user clicks in scene view
    // The actual object placement needs to happen in the render method
    // where we have access to bg_manager
}

void EditorScene::render_background_placement_preview(
    BackgroundManager* bg_manager,
    BackgroundObjectPresetManager* bg_preset_manager, TilePanel& tile_panel,
    ImDrawList* draw_list, const ImVec2& origin, Level& level) {
    // Get mouse position
    ImVec2 mouse_pos = ImGui::GetMousePos();
    bool hovered = ImGui::IsWindowHovered();
    bool left_clicked = ImGui::IsMouseClicked(0);

    // Get selected layer
    auto selected = bg_manager->get_selected_layer();
    if (!selected.has_value()) {
        return;
    }

    // Get preset info
    int preset_idx = tile_panel.get_selected_background_preset_idx();
    float scale = tile_panel.get_background_object_scale();

    if (preset_idx < 0) {
        return;
    }

    // Calculate position relative to origin
    // Background is centered horizontally, so adjust for offset
    constexpr float BG_HORIZONTAL_OFFSET = 320.0f;  // Half of 640
    float x = mouse_pos.x - origin.x + BG_HORIZONTAL_OFFSET;
    float y = mouse_pos.y - origin.y;

    // Snap to 32-pixel grid
    constexpr float GRID_SIZE = 32.0f;
    float snapped_x = std::floor(x / GRID_SIZE) * GRID_SIZE;
    float snapped_y = std::floor(y / GRID_SIZE) * GRID_SIZE;

    // Calculate spannable bounds based on parallax factor
    constexpr float SCREEN_WIDTH = 640.0f;
    constexpr float SCREEN_HEIGHT = 480.0f;

    const auto& layers = bg_manager->get_layers();
    const auto& selected_layer = layers[selected.value()];
    float parallax_factor = selected_layer.get_parallax_factor();

    float level_height = level.row_cnt * GRID_SIZE;
    float max_scroll = std::max(0.0f, level_height - SCREEN_HEIGHT);
    float base_spannable_height =
        max_scroll * (1.0f - parallax_factor) + SCREEN_HEIGHT;
    float spannable_height =
        base_spannable_height + SCREEN_HEIGHT * 0.1f;  // Small margin

    // Get preset data to check bounds
    const auto& presets = bg_preset_manager->get_presets();
    if (preset_idx >= static_cast<int>(presets.size())) {
        return;
    }
    const auto& preset = presets[preset_idx];
    float preview_size = preset.tile_size * scale;

    // Allow objects to be partially outside bounds (at least 25% must be
    // within) This allows objects to extend beyond edges for better visual
    // coverage
    float min_overlap = preview_size * 0.25f;

    // Background is centered, so spannable width extends from -320 to 960
    // (total 1280)
    float spannable_width_left = -SCREEN_WIDTH / 2.0f;
    float spannable_width_right = SCREEN_WIDTH + SCREEN_WIDTH / 2.0f;

    // Check if at least part of the object is within spannable area
    bool within_bounds =
        (snapped_x + preview_size >= spannable_width_left - preview_size +
                                         min_overlap &&  // Allow left edge
         snapped_x <=
             spannable_width_right - min_overlap &&  // Allow right edge
         snapped_y + preview_size >=
             0 - preview_size + min_overlap &&          // Allow top edge
         snapped_y <= spannable_height - min_overlap);  // Allow bottom edge

    // Don't show preview or allow placement if out of bounds
    if (!within_bounds) {
        return;
    }

    // Convert back to screen coordinates for rendering
    // Subtract offset because background is centered
    ImVec2 snapped_screen_pos(origin.x + snapped_x - BG_HORIZONTAL_OFFSET,
                              origin.y + snapped_y);

    // Draw preview sprite at mouse position
    if (hovered) {
        // Load the sprite sheet texture
        std::string texture_path = preset.sprite_sheet;
        Texture2D texture = load_texture_cached(texture_path);

        if (texture.id > 0) {
            // Calculate UV coordinates for the tile
            float tile_size_f = static_cast<float>(preset.tile_size);
            float u0 = (preset.tile_col * tile_size_f) / texture.width;
            float v0 = (preset.tile_row * tile_size_f) / texture.height;
            float u1 = u0 + (tile_size_f / texture.width);
            float v1 = v0 + (tile_size_f / texture.height);

            // Draw semi-transparent preview sprite at snapped position
            ImVec2 p_min(snapped_screen_pos.x, snapped_screen_pos.y);
            ImVec2 p_max(snapped_screen_pos.x + preview_size,
                         snapped_screen_pos.y + preview_size);

            draw_list->AddImage(static_cast<ImTextureID>(texture.id),
                                p_min,
                                p_max,
                                ImVec2(u0, v0),
                                ImVec2(u1, v1),
                                IM_COL32(255, 255, 255, 180));

            // Draw outline around preview
            draw_list->AddRect(
                p_min, p_max, IM_COL32(255, 255, 0, 200), 0.0f, 0, 2.0f);
        } else {
            // Fallback to circle if texture fails to load
            float preview_size = 64.0f * scale;
            ImVec2 center(snapped_screen_pos.x + preview_size / 2,
                          snapped_screen_pos.y + preview_size / 2);
            draw_list->AddCircle(
                center, preview_size / 2, IM_COL32(255, 255, 0, 200), 32, 2.0f);
        }

        // Handle click to place object
        if (left_clicked && bg_preset_manager) {
            auto selected_layer = bg_manager->get_selected_layer();
            if (!selected_layer.has_value()) {
                return;
            }

            int selected_layer_idx = static_cast<int>(selected_layer.value());
            const auto& presets = bg_preset_manager->get_presets();

            if (preset_idx < static_cast<int>(presets.size())) {
                const auto& preset = presets[preset_idx];

                // Create background object from preset
                BackgroundObject obj;
                obj.sprite_name = preset.name;
                obj.x = snapped_x;
                obj.y = snapped_y;
                obj.scale = scale;
                obj.rotation = 0.0f;
                obj.sprite_sheet = preset.sprite_sheet;
                obj.tile_size = preset.tile_size;
                obj.tile_row = preset.tile_row;
                obj.tile_col = preset.tile_col;

                // Add to selected layer
                bg_manager->add_object(selected_layer_idx, obj);
            }
        }
    }

    // Handle cancel with ESC or right-click
    if (ImGui::IsKeyPressed(ImGuiKey_Escape) ||
        ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        tile_panel.clear_background_placing_mode();
    }
}
