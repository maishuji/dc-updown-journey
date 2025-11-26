// Copyright 2025 Quentin Cartier
// NewLevelPopup.hpp
#pragma once
#include "udjourney-editor/Level.hpp"
#include "udjourney-editor/strategies/level/LevelCreationStrategy.hpp"

enum class LevelTemplate { Blank, Staircase };

struct NewLevelPopup {
    int rows = 20;
    int cols = 20;
    bool show = false;
    LevelTemplate selected_template = LevelTemplate::Blank;

    // Reference to your level system
    Level* level = nullptr;
    LevelCreationStrategy* strategy = nullptr;

    void open() { show = true; }
    void render();
};
