#pragma once

#include <raylib/raylib.h>

#include <string>

#include "udjourney/hud/HUDComponent.hpp"

class DialogBoxHUD : public HUDComponent {
 public:
    explicit DialogBoxHUD(Rectangle iRect);
    void update(float deltaTime) override;

    void draw() const override;

    std::string get_type() const override { return "DialogBoxHUD"; }

 private:
    Rectangle m_rect;
    std::string m_text;
};