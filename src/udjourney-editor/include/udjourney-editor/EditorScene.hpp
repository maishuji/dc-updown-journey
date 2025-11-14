// Copyright 2025 Quentin Cartier
#pragma once

#include <imgui.h>

#include <cmath>
#include <memory>

#include "udjourney-editor/Level.hpp"
#include "udjourney-editor/TilePanel.hpp"
#include "udjourney-editor/ui/PlatformContextPopup.hpp"

class EditorScene {
 public:
    EditorScene();
    // Main rendering function
    void render(Level& level, TilePanel& tile_panel);

    virtual ~EditorScene();

    // Configuration
    void set_tile_size(float size) noexcept { tile_size_ = size; }
    void set_offset(float offset_x, float offset_y) noexcept {
        offset_x_ = offset_x;
        offset_y_ = offset_y;
    }

    // Selection state
    bool is_selecting() const noexcept { return selecting_; }
    bool is_selection_done() const noexcept { return selection_done_; }

 private:
    // Scene configuration
    float tile_size_ = 32.0f;
    float offset_x_ = 300.0f;
    float offset_y_ = 0.0f;

    // Selection state
    bool selecting_ = false;
    bool selection_done_ = false;
    ImVec2 selection_start_;
    ImVec2 selection_end_;

    // Internal rendering methods
    void setup_scene_window(const ImGuiIO& io);
    void render_grid(Level& level, ImDrawList* draw_list, const ImVec2& origin);
    void render_platforms(Level& level, TilePanel& tile_panel,
                          ImDrawList* draw_list, const ImVec2& origin);
    void render_player_spawn(Level& level, ImDrawList* draw_list,
                             const ImVec2& origin);
    void handle_mouse_input(Level& level, TilePanel& tile_panel,
                            ImDrawList* draw_list, const ImVec2& origin);
    void handle_tile_mode_input(Level& level, TilePanel& tile_panel,
                                ImDrawList* draw_list, const ImVec2& origin);
    void handle_platform_mode_input(Level& level, TilePanel& tile_panel,
                                    const ImVec2& mouse_pos,
                                    const ImVec2& origin);
    void handle_spawn_mode_input(Level& level, const ImVec2& mouse_pos,
                                 const ImVec2& origin);
    void handle_monster_mode_input(Level& level, TilePanel& tile_panel,
                                   const ImVec2& mouse_pos,
                                   const ImVec2& origin, bool left_clicked,
                                   bool right_clicked);
    void render_monsters(Level& level, TilePanel& tile_panel,
                         ImDrawList* draw_list, const ImVec2& origin);
    void render_selection(ImDrawList* draw_list);
    void apply_selection_to_tiles(Level& level, TilePanel& tile_panel,
                                  ImDrawList* draw_list, const ImVec2& origin);

    // Helper methods
    bool is_tile_in_selection(const ImVec2& tile_top_left,
                              const ImVec2& tile_bottom_right,
                              const ImVec2& selection_min,
                              const ImVec2& selection_max) const;
    ImVec2 screen_to_tile_pos(const ImVec2& screen_pos,
                              const ImVec2& origin) const;
    ImU32 get_platform_color(PlatformBehaviorType behavior,
                             PlatformFeatureType feature) const;

    struct PImpl;
    std::unique_ptr<PImpl> pimpl_;

    // Platform context popup
    PlatformContextPopup platform_popup_;
};
