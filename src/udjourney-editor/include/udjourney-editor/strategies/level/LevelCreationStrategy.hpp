#pragma once
#include "udjourney-editor/Level.hpp"

struct LevelCreationStrategy {
    virtual ~LevelCreationStrategy() = default;
    virtual void create(Level& level, int tiles_x, int tiles_y) = 0;
};
