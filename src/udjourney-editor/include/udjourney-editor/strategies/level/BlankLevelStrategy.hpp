#pragma once
#include "udjourney-editor/strategies/level/LevelCreationStrategy.hpp"

struct BlankLevelStrategy : public LevelCreationStrategy {
    ~BlankLevelStrategy() override = default;
    void create(Level& level, int tiles_x, int tiles_y) override;
};
