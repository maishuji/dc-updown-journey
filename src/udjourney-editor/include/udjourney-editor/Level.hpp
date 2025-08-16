#pragma once

#include <imgui.h>

#include <iostream>
#include <vector>

struct Cell {
    ImU32 color = IM_COL32(2550, 255, 255, 255);  // Default color for the cell
};

struct Level {
    std::vector<Cell> tiles;
    size_t rows = 0;
    size_t cols = 0;

    void clear() {
        tiles.clear();
        rows = 0;
        cols = 0;
    }

    void resize(size_t new_rows, size_t new_cols) {
        rows = new_rows;
        cols = new_cols;
        tiles.resize(rows * cols);
    }

    void push_back(const Cell& cell) {
        if (tiles.size() < rows * cols) {
            tiles.push_back(cell);
        } else {
            std::cerr << "Cannot add more cells than the defined size."
                      << std::endl;
        }
    }

    void reserve(size_t new_size) { tiles.reserve(new_size); }
};