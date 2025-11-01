// Copyright 2025 Quentin Cartier
#include "udjourney-editor/strategies/level/BlankLevelStrategy.hpp"

void BlankLevelStrategy::create(Level& level, int tiles_y, int tiles_x) {
    level.clear();
    level.resize(tiles_y, tiles_x);
    // fill with empty tiles
    Cell empty_cell;
    for (int i = 0; i < tiles_x * tiles_y; ++i) {
        level.push_back(empty_cell);
    }
}
