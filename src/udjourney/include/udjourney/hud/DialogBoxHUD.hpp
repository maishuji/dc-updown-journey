// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "udjourney/hud/HUDComponent.hpp"

class DialogBoxHUD : public HUDComponent {
 public:
    explicit DialogBoxHUD(Rectangle iRect);
    ~DialogBoxHUD() override;

    void update(float deltaTime) override;

    void handle_input() override {
        // Handle input if needed, e.g., close dialog on key press
        if (IsKeyPressed(KEY_ENTER)) {
            next_page();
        } else if (IsKeyPressed(KEY_ESCAPE)) {
            m_on_finished_callback();
        }
    }

    void draw() const override;

    [[nodiscard]] std::string get_type() const override {
        return "DialogBoxHUD";
    }
    void set_text(const std::string& text);
    void set_text(std::string&& text);

    [[nodiscard]] const std::vector<std::string>& get_wrapped_lines() const {
        return m_wrapped_lines;
    }

    [[nodiscard]] const std::string& get_text() const { return m_text; }
    void next_page();

    void set_on_finished_callback(std::function<void()> callback);
    void set_on_next_callback(std::function<void()> callback);

 private:
    Rectangle m_rect;
    std::vector<std::string> m_wrapped_lines;
    std::string m_text;
    std::function<void()> m_on_finished_callback = []() {};
    std::function<void()> m_on_next_callback = []() {};
    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;
};
