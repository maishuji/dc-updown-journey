// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>

#include <string>
#include <vector>

#include "udjourney/hud/HUDComponent.hpp"

class DialogBoxHUD : public HUDComponent {
 public:
    explicit DialogBoxHUD(Rectangle iRect);
    void update(float deltaTime) override;

    void draw() const override;

    [[nodiscard]] std::string get_type() const override {
        return "DialogBoxHUD";
    }

 private:
    Rectangle m_rect;
    std::vector<std::string> m_wrapped_lines;
    std::string m_text;
};
