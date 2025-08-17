// Copyright 2025 Quentin Cartier
#pragma once

#include <imgui.h>

#include <iostream>
#include <vector>

struct Cell {
    ImU32 color = IM_COL32(2550, 255, 255, 255);  // Default color for the cell
};

struct Level {
    std::vector<Cell> tiles;
    size_t row_cnt = 0;
    size_t col_cnt = 0;

    void clear() {
        tiles.clear();
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

    void reserve(size_t new_size) { tiles.reserve(new_size); }
};