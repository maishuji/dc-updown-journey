// Copyright 2025 Quentin Cartier
#pragma once

#include <string>

#include "udjourney-editor/fud/IFUDRenderer.hpp"

/**
 * @brief Renderer for scrollable list FUD elements
 *
 * Handles rendering of scrollable_list type widgets in the editor.
 * Displays a visual representation of the list with sample items.
 */
class ScrollableListFUDRenderer : public IFUDRenderer {
 public:
    void render(const FUDElement& fud, ImDrawList* draw_list,
                const ImVec2& fud_pos, const ImVec2& fud_end,
                bool is_selected) override;

 private:
    /**
     * @brief Extract data source from properties
     * @param fud The FUD element
     * @return Data source string (levels, settings, scores, custom)
     */
    std::string get_data_source(const FUDElement& fud) const;

    /**
     * @brief Extract item height from properties
     * @param fud The FUD element
     * @return Item height in pixels
     */
    int get_item_height(const FUDElement& fud) const;

    /**
     * @brief Extract font size from properties
     * @param fud The FUD element
     * @return Font size for main text
     */
    int get_font_size(const FUDElement& fud) const;

    /**
     * @brief Extract visible items count from properties
     * @param fud The FUD element
     * @return Number of visible items
     */
    int get_visible_items(const FUDElement& fud) const;
};
