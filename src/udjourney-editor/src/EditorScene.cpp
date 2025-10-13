// Copyright 2025 Quentin Cartier
#include "udjourney-editor/EditorScene.hpp"

void EditorScene::render(Level& level, TilePanel& tile_panel) {
    ImGuiIO& io = ImGui::GetIO();
    
    // Set up the scene window
    setup_scene_window(io);
    
    ImGui::Begin("Scene View",
                 nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoCollapse);

    // Get draw list and origin for rendering
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();

    // Render the grid
    render_grid(level, draw_list, origin);
    
    // Handle mouse input and selection
    handle_mouse_input(level, tile_panel, draw_list, origin);
    
    // Reserve space for ImGui layout
    ImGui::Dummy(ImVec2(level.col_cnt * tile_size_, level.row_cnt * tile_size_));
    
    ImGui::End();
}

void EditorScene::setup_scene_window(const ImGuiIO& io) {
    offset_y_ = ImGui::GetFrameHeight();
    
    ImGui::SetNextWindowPos(ImVec2(offset_x_, offset_y_), ImGuiCond_Always);
    ImGui::SetNextWindowSize(
        ImVec2(io.DisplaySize.x - offset_x_, io.DisplaySize.y - offset_y_),
        ImGuiCond_Always);
}

void EditorScene::render_grid(Level& level, ImDrawList* draw_list, const ImVec2& origin) {
    for (int y = 0; y < static_cast<int>(level.row_cnt); ++y) {
        for (int x = 0; x < static_cast<int>(level.col_cnt); ++x) {
            ImVec2 top_left = ImVec2(origin.x + x * tile_size_, 
                                   origin.y + y * tile_size_);
            ImVec2 bottom_right = ImVec2(top_left.x + tile_size_, 
                                       top_left.y + tile_size_);

            // Draw filled tile with its color
            draw_list->AddRectFilled(
                top_left,
                bottom_right,
                level.tiles[y * level.col_cnt + x].color);
                
            // Draw grid lines
            draw_list->AddRect(top_left, bottom_right, IM_COL32(200, 200, 200, 255));
        }
    }
}

void EditorScene::handle_mouse_input(Level& level, TilePanel& tile_panel, 
                                    ImDrawList* draw_list, const ImVec2& origin) {
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

void EditorScene::render_selection(ImDrawList* draw_list) {
    ImU32 fill_color = IM_COL32(0, 120, 255, 100);    // Semi-transparent fill
    ImU32 border_color = IM_COL32(0, 120, 255, 255);  // Solid border

    ImVec2 p_min = ImVec2(fminf(selection_start_.x, selection_end_.x),
                         fminf(selection_start_.y, selection_end_.y));
    ImVec2 p_max = ImVec2(fmaxf(selection_start_.x, selection_end_.x),
                         fmaxf(selection_start_.y, selection_end_.y));

    draw_list->AddRectFilled(p_min, p_max, fill_color);
    draw_list->AddRect(p_min, p_max, border_color, 0.0f, 0, 2.0f);
}

void EditorScene::apply_selection_to_tiles(Level& level, TilePanel& tile_panel,
                                          ImDrawList* draw_list, const ImVec2& origin) {
    ImVec2 p_min = ImVec2(fminf(selection_start_.x, selection_end_.x),
                         fminf(selection_start_.y, selection_end_.y));
    ImVec2 p_max = ImVec2(fmaxf(selection_start_.x, selection_end_.x),
                         fmaxf(selection_start_.y, selection_end_.y));

    for (int y = 0; y < static_cast<int>(level.row_cnt); ++y) {
        for (int x = 0; x < static_cast<int>(level.col_cnt); ++x) {
            ImVec2 tile_top_left = ImVec2(origin.x + x * tile_size_,
                                         origin.y + y * tile_size_);
            ImVec2 tile_bottom_right = ImVec2(tile_top_left.x + tile_size_,
                                             tile_top_left.y + tile_size_);

            // Check if tile is fully inside the selection rectangle
            if (is_tile_in_selection(tile_top_left, tile_bottom_right, p_min, p_max)) {
                // Apply current color from tile panel
                level.tiles[y * level.col_cnt + x].color = tile_panel.get_current_color();

                // Draw selection indicator
                draw_list->AddRect(tile_top_left, tile_bottom_right, 
                                 IM_COL32(255, 0, 0, 100));
            }
        }
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