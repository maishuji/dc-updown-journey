// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>

#include <functional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "udjourney/scene/Scene.hpp"
#include "udjourney/widgets/IWidget.hpp"

/**
 * @brief Generic scrollable list widget that can display any type of items
 *
 * Configuration through FUD properties:
 * - "data_source": Type of data to display (e.g., "levels", "settings",
 * "scores")
 * - "item_action": Action template with placeholders (e.g.,
 * "load_level:{filename}")
 * - "item_height": Height of each item in pixels
 * - "font_size": Font size for item text
 * - "subtitle_font_size": Font size for subtitle text
 * - "visible_items": Number of items visible at once (optional)
 *
 * Action template placeholders:
 * - {id} - Item ID
 * - {filename} - Item metadata["filename"]
 * - {index} - Selected index
 */
class ScrollableListWidget : public IWidget {
 public:
    struct ListItem {
        std::string id;            // Unique identifier
        std::string display_text;  // Main text to display
        std::string subtitle;      // Optional subtitle/description
        std::string icon;          // Optional icon path
        nlohmann::json metadata;   // Custom data (difficulty, score, etc.)
        bool selectable;           // Can this item be selected?
        bool locked;               // Is this item locked/disabled?
        Color text_color;          // Custom text color
    };

    using DataProvider = std::function<std::vector<ListItem>()>;
    using ItemSelectedCallback = std::function<void(const ListItem&, IGame*)>;

    ScrollableListWidget(const IGame& game,
                         const udjourney::scene::FUDData& fud);

    // IActor interface
    void set_rectangle(Rectangle rect) override { rect_ = rect; }
    Rectangle get_rectangle() const override { return rect_; }
    bool check_collision(const IActor& other) const override { return false; }

    void draw() const override;
    void update(float delta) override;
    void process_input() override;

    bool contains_point(Vector2 point) const override;

    void on_click() override;
    void on_hover() override;
    void on_focus() override;

    // Scrolling controls
    void scroll_up();
    void scroll_down();
    void scroll_to(int index);
    void page_up();
    void page_down();

    // Data management
    void set_data_provider(DataProvider provider);
    void set_item_selected_callback(ItemSelectedCallback callback);
    void refresh_data();

    int get_selected_index() const { return selected_index_; }
    const ListItem* get_selected_item() const;

 private:
    void load_data_from_source(const std::string& source_type);
    void draw_list_item(const ListItem& item, Rectangle item_rect,
                        bool is_selected, bool is_hovered) const;
    void execute_item_action(const ListItem& item);

    Rectangle rect_;
    std::vector<ListItem> items_;

    // Data source configuration
    std::string data_source_;
    std::string item_action_template_;
    DataProvider data_provider_;
    ItemSelectedCallback on_item_selected_;

    // Scroll state
    int selected_index_;
    int scroll_offset_;  // Scroll position in pixels
    int item_height_;    // Height of each item
    int visible_items_;  // Number of visible items

    // Styling
    Color bg_color_;
    Color selected_color_;
    Color hover_color_;
    Color locked_color_;
    Color border_color_;
    int font_size_;
    int subtitle_font_size_;
    int padding_;
    int border_thickness_;

    // Smooth scrolling
    float scroll_animation_target_;
    float scroll_animation_speed_;
};
