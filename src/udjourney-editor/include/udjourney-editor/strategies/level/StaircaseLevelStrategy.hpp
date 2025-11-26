// Copyright 2025 Quentin Cartier
#pragma once
#include "udjourney-editor/strategies/level/LevelCreationStrategy.hpp"

struct StaircaseLevelStrategy : public LevelCreationStrategy {
    explicit StaircaseLevelStrategy(int step_width = 3, int step_height = 1);
    ~StaircaseLevelStrategy() override = default;

    void create(Level& level, int tiles_x, int tiles_y) override;

 private:
    int step_width_;
    int step_height_;
};
