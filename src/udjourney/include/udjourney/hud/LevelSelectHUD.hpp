// Copyright 2025 Quentin Cartier
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "raylib/raylib.h"
#include "udjourney/hud/HUDComponent.hpp"

class LevelSelectHUD : public HUDComponent {
 public:
    LevelSelectHUD(Rectangle rect, const std::string& levels_dir);
    ~LevelSelectHUD() override = default;

    void update(float deltaTime) override;
    void draw() const override;
    void handle_input() override;
    [[nodiscard]] std::string get_type() const override {
        return "LevelSelectHUD";
    }

    // Callbacks
    void set_on_level_selected_callback(
        std::function<void(const std::string&)> callback);
    void set_on_cancelled_callback(std::function<void()> callback);

 private:
    void scan_levels_directory();
    void draw_menu() const;

    Rectangle m_rect;
    std::string m_levels_dir;
    std::vector<std::string> m_level_files;
    int m_selected_index = 0;

    std::function<void(const std::string&)> m_on_level_selected;
    std::function<void()> m_on_cancelled;

    // Input mapping for navigation
    bool m_up_pressed_last_frame = false;
    bool m_down_pressed_last_frame = false;
    bool m_enter_pressed_last_frame = false;
    bool m_escape_pressed_last_frame = false;
};
