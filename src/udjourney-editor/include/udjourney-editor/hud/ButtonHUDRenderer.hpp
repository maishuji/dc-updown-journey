// Copyright 2025 Quentin Cartier
#pragma once

#include <string>

#include "udjourney-editor/hud/IHUDRenderer.hpp"

/**
 * @brief Renderer for button HUD elements
 *
 * Handles rendering of all button types: menu_button, small_button,
 * large_button, icon_button, and textured_button.
 *
 * Displays the button's text property centered within the button bounds,
 * using the configured font size and text color.
 */
class ButtonHUDRenderer : public IHUDRenderer {
 public:
    void render(const HUDElement& hud, ImDrawList* draw_list,
                const ImVec2& fud_pos, const ImVec2& fud_end,
                bool is_selected) override;

 private:
    /**
     * @brief Extract button text from properties
     * @param hud The HUD element
     * @return Button text string, or "Button" as default
     */
    std::string get_button_text(const HUDElement& hud) const;

    /**
     * @brief Extract font size from properties
     * @param hud The HUD element
     * @return Font size, or 24 as default
     */
    int get_font_size(const HUDElement& hud) const;

    /**
     * @brief Extract text color from properties
     * @param hud The HUD element
     * @return ImGui color, or white as default
     */
    ImU32 get_text_color(const HUDElement& hud) const;
};
