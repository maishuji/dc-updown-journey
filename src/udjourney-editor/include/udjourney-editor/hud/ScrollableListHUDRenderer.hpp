// Copyright 2025 Quentin Cartier
#pragma once

#include <string>

#include "udjourney-editor/hud/IHUDRenderer.hpp"

/**
 * @brief Renderer for scrollable list HUD elements
 *
 * Handles rendering of scrollable_list type widgets in the editor.
 * Displays a visual representation of the list with sample items.
 */
class ScrollableListHUDRenderer : public IHUDRenderer {
 public:
    void render(const HUDElement& hud, ImDrawList* draw_list,
                const ImVec2& fud_pos, const ImVec2& fud_end,
                bool is_selected) override;

 private:
    /**
     * @brief Extract data source from properties
     * @param hud The HUD element
     * @return Data source string (levels, settings, scores, custom)
     */
    std::string get_data_source(const HUDElement& hud) const;

    /**
     * @brief Extract item height from properties
     * @param hud The HUD element
     * @return Item height in pixels
     */
    int get_item_height(const HUDElement& hud) const;

    /**
     * @brief Extract font size from properties
     * @param hud The HUD element
     * @return Font size for main text
     */
    int get_font_size(const HUDElement& hud) const;

    /**
     * @brief Extract visible items count from properties
     * @param hud The HUD element
     * @return Number of visible items
     */
    int get_visible_items(const HUDElement& hud) const;
};
