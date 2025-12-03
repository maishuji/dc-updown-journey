// Copyright 2025 Quentin Cartier
#pragma once
#include <raylib/raylib.h>
#include <string>
#include "udjourney/interfaces/IActor.hpp"

class IGame;

/**
 * @brief Base interface for UI widgets (buttons, sliders, etc.)
 *
 * Widgets are special actors that handle user input and can trigger
 * actions through the ActionDispatcher. They can be placed in scenes
 * like any other actor and loaded from JSON/FUD data.
 */
class IWidget : public IActor {
 public:
    explicit IWidget(const IGame& game) : IActor(game) {}
    virtual ~IWidget() = default;

    // Widget-specific interface

    /**
     * @brief Called when the widget is clicked
     */
    virtual void on_click() = 0;

    /**
     * @brief Called when the mouse hovers over the widget
     */
    virtual void on_hover() = 0;

    /**
     * @brief Called when the widget gains focus
     */
    virtual void on_focus() = 0;

    /**
     * @brief Check if a point is inside the widget's bounds
     * @param point Point to check in screen coordinates
     * @return True if point is inside widget
     */
    virtual bool contains_point(Vector2 point) const = 0;

    /**
     * @brief Get the action string associated with this widget
     * @return Action string (e.g., "start_game" or "load_level:level2")
     */
    virtual std::string get_action() const { return action_; }

    /**
     * @brief Set the action string for this widget
     * @param action Action string to execute on interaction
     */
    virtual void set_action(const std::string& action) { action_ = action; }

    /**
     * @brief Check if widget is currently focused (selected for keyboard input)
     */
    virtual bool is_focused() const { return is_focused_; }

    /**
     * @brief Set focus state
     */
    virtual void set_focused(bool focused) { is_focused_ = focused; }

    /**
     * @brief Check if widget is selectable (can be navigated with keyboard)
     */
    virtual bool is_selectable() const { return is_selectable_; }

    /**
     * @brief Set selectable state
     */
    virtual void set_selectable(bool selectable) {
        is_selectable_ = selectable;
    }

    /**
     * @brief Get widget group ID (for querying)
     */
    uint8_t get_group_id() const override { return 4; }  // WIDGET_TYPE_ID = 4

 protected:
    std::string
        action_;  // Action to execute (e.g., "start_game", "load_level:level2")
    bool is_hovered_ = false;
    bool is_focused_ = false;
    bool is_selectable_ = true;  // By default, widgets are selectable
};
