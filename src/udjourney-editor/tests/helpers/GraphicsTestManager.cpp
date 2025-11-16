// Copyright 2025 Quentin Cartier
#ifdef EDITOR_TESTING

#include "helpers/GraphicsTestManager.hpp"
#include <raylib.h>

void GraphicsTestManager::init_if_needed() {
    if (!initialized_) {
        // Initialize raylib with minimal window for testing
        SetConfigFlags(FLAG_WINDOW_HIDDEN);  // Hidden window for tests
        InitWindow(1, 1, "Test Window");
        initialized_ = true;
    }
}

void GraphicsTestManager::cleanup() {
    if (initialized_) {
        CloseWindow();
        initialized_ = false;
    }
}

#endif  // EDITOR_TESTING
