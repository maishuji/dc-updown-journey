// Copyright 2025 Quentin Cartier
#pragma once+
#include <imgui.h>

#include <string>

namespace color {
extern const ImU32 kColorRed;
extern const ImU32 kColorGreen;
extern const ImU32 kColorBlue;
extern const ImU32 kColorOrange;
extern const ImU32 kColorLightGreen;
extern const ImU32 kColorPurple;
}  // namespace color

class TilePanel {
 public:
    void draw();
    inline ImU32 get_current_color() const noexcept { return cur_color; }
    void set_button(const std::string& iId, ImU32 color);
    inline void set_scale(float scale) noexcept { this->scale = scale; }

 private:
    float scale = 1.0f;  // Default scale
    ImU32 cur_color = IM_COL32(255, 255, 255,
                               255);  // Default color for the current selection
};