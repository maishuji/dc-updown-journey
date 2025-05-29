// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>

#include <functional>
#include <string>
#include <vector>

#include "udjourney/hud/HUDComponent.hpp"

class DialogBoxHUD : public HUDComponent {
 public:
    explicit DialogBoxHUD(Rectangle iRect);
    void update(float deltaTime) override;

    void handle_input() override {
        // Handle input if needed, e.g., close dialog on key press
        if (IsKeyPressed(KEY_ENTER)) {
            m_on_finished_callback();
        }
    }

    void draw() const override;

    [[nodiscard]] std::string get_type() const override {
        return "DialogBoxHUD";
    }

    void set_on_finished_callback(std::function<void()> callback);

 private:
    Rectangle m_rect;
    std::vector<std::string> m_wrapped_lines;
    std::string m_text;
    std::function<void()> m_on_finished_callback = []() {};
};
