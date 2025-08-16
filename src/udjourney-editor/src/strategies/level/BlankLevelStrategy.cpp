#include "udjourney-editor/strategies/level/BlankLevelStrategy.hpp"

void BlankLevelStrategy::create(Level& level, int tiles_x, int tiles_y) {
    level.clear();
    level.resize(tiles_x, tiles_y);
    // fill with empty tiles
    Cell empty_cell;
    for (int i = 0; i < tiles_x * tiles_y; ++i) {
        level.push_back(empty_cell);
    }
}
    
