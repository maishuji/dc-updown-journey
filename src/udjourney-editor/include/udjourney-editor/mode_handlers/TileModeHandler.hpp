// Copyright 2025 Quentin Cartier
#pragma once

#include <imgui.h>

#include "udjourney-editor/mode_handlers/IModeHandler.hpp"

/**
 * @brief Handler for Tile edit mode
 */
class TileModeHandler : public IModeHandler {
 public:
    TileModeHandler();

    void render() override;
    void set_scale(float scale) override { scale_ = scale; }

    // Tile-specific API
    ImU32 get_current_color() const { return current_color_; }

 private:
    float scale_ = 1.0f;
    ImU32 current_color_ = IM_COL32(255, 255, 255, 255);

    void render_tile_button(const char* name, ImU32 color);
};
