// Copyright 2025 Quentin Cartier
#pragma once

#include <imgui.h>

/**
 * @brief Base interface for all editor mode handlers
 *
 * Each edit mode (Tiles, Platforms, Background, etc.) implements this interface
 * to provide mode-specific UI and behavior in a decoupled way.
 */
class IModeHandler {
 public:
    virtual ~IModeHandler() = default;

    /**
     * @brief Render the mode-specific UI panel
     */
    virtual void render() = 0;

    /**
     * @brief Set the UI scale factor
     * @param scale The scale multiplier for UI elements
     */
    virtual void set_scale(float scale) = 0;

    /**
     * @brief Called when this mode becomes active
     */
    virtual void on_activate() {}

    /**
     * @brief Called when this mode is deactivated
     */
    virtual void on_deactivate() {}
};
