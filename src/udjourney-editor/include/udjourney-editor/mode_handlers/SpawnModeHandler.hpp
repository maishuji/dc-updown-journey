// Copyright 2025 Quentin Cartier
#pragma once

#include "udjourney-editor/mode_handlers/IModeHandler.hpp"

/**
 * @brief Handler for Player Spawn edit mode
 */
class SpawnModeHandler : public IModeHandler {
 public:
    SpawnModeHandler();

    void render() override;
    void set_scale(float scale) override { scale_ = scale; }

 private:
    float scale_ = 1.0f;
};
