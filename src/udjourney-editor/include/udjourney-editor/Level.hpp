// Copyright 2025 Quentin Cartier
#pragma once

#include <imgui.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "udjourney-editor/fud/FUDElement.hpp"

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

struct EditorMonster {
    int tile_x;
    int tile_y;
    std::string preset_name = "goblin";      // Default preset
    ImU32 color = IM_COL32(255, 0, 0, 255);  // Red for monsters

    // Optional overrides for health and speed (use -1 to indicate "use preset
    // default")
    int health_override = -1;
    int speed_override = -1;
};

struct Cell {
    ImU32 color = IM_COL32(255, 255, 255, 255);  // Default color for the cell
};

// Scene type enum - matches udjourney::scene::SceneType
enum class SceneType {
    LEVEL,     // Regular gameplay level with platforms/monsters
    UI_SCREEN  // UI screen for menus (title/win/gameover)
};

struct Level {
    std::vector<Cell> tiles;
    std::vector<EditorPlatform> platforms;
    std::vector<EditorMonster> monsters;
    std::vector<FUDElement> fuds;
    size_t row_cnt = 0;
    size_t col_cnt = 0;
    int player_spawn_x = 2;
    int player_spawn_y = 8;
    SceneType scene_type = SceneType::LEVEL;  // Default to level

    void clear() {
        tiles.clear();
        platforms.clear();
        monsters.clear();
        fuds.clear();
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

    void add_monster(const EditorMonster& monster) {
        monsters.push_back(monster);
    }

    void remove_monster_at(int tile_x, int tile_y) {
        monsters.erase(std::remove_if(monsters.begin(),
                                      monsters.end(),
                                      [tile_x, tile_y](const EditorMonster& m) {
                                          return m.tile_x == tile_x &&
                                                 m.tile_y == tile_y;
                                      }),
                       monsters.end());
    }

    EditorMonster* get_monster_at(int tile_x, int tile_y) {
        for (auto& monster : monsters) {
            if (monster.tile_x == tile_x && monster.tile_y == tile_y) {
                return &monster;
            }
        }
        return nullptr;
    }

    void add_fud(const FUDElement& fud) { fuds.push_back(fud); }

    void remove_fud(size_t index) {
        if (index < fuds.size()) {
            fuds.erase(fuds.begin() + index);
        }
    }

    void reserve(size_t new_size) { tiles.reserve(new_size); }
};
