// Copyright 2025 Quentin Cartier
#pragma once

#include <imgui.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

// Forward declarations for udjourney types
enum class PlatformBehaviorType {
    Static,
    Horizontal,
    EightTurnHorizontal,
    OscillatingSize
};

enum class PlatformFeatureType { None, Spikes, Checkpoint };

struct EditorPlatform {
    int tile_x;
    int tile_y;
    float width_tiles = 1.0f;
    float height_tiles = 1.0f;
    PlatformBehaviorType behavior_type = PlatformBehaviorType::Static;
    std::vector<PlatformFeatureType> features;
    ImU32 color = IM_COL32(0, 0, 255, 255);  // Blue default
};

struct Cell {
    ImU32 color = IM_COL32(255, 255, 255, 255);  // Default color for the cell
};

struct Level {
    std::vector<Cell> tiles;
    std::vector<EditorPlatform> platforms;
    size_t row_cnt = 0;
    size_t col_cnt = 0;
    int player_spawn_x = 2;
    int player_spawn_y = 8;

    void clear() {
        tiles.clear();
        platforms.clear();
        row_cnt = 0;
        col_cnt = 0;
    }

    void resize(size_t new_rows, size_t new_cols) {
        row_cnt = new_rows;
        col_cnt = new_cols;
        tiles.resize(row_cnt * col_cnt);
    }

    void push_back(const Cell& cell) {
        if (tiles.size() < row_cnt * col_cnt) {
            tiles.push_back(cell);
        } else {
            std::cerr << "Cannot add more cells than the defined size."
                      << std::endl;
        }
    }

    void add_platform(const EditorPlatform& platform) {
        platforms.push_back(platform);
    }

    void remove_platform_at(int tile_x, int tile_y) {
        platforms.erase(
            std::remove_if(platforms.begin(),
                           platforms.end(),
                           [tile_x, tile_y](const EditorPlatform& p) {
                               return p.tile_x == tile_x && p.tile_y == tile_y;
                           }),
            platforms.end());
    }

    void reserve(size_t new_size) { tiles.reserve(new_size); }
};
