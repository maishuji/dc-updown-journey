// Copyright 2025 Quentin Cartier
#pragma once
#include <raylib/raylib.h>
#include <string>
#include <map>
#include "udjourney/widgets/IWidget.hpp"
#include "udjourney/scene/Scene.hpp"
namespace udjourney {
/**
 * @brief Button widget for menus and UI
 *
 * Renders a clickable button with text, handles hover effects,
 * and triggers actions through the ActionDispatcher when clicked.
 *
 * Supports texture-based backgrounds with different states:
 * - idle: Default button appearance
 * - hover: When mouse is over the button
 * - focused: When button is keyboard-selected
 * - pressed: When button is being clicked
 */
class ButtonWidget : public IWidget {
 public:
    /**
     * @brief Construct button from HUD data
     * @param game Reference to game
     * @param hud HUD element containing button properties
     */
    ButtonWidget(const IGame& game, const udjourney::scene::HUDData& hud);

    // IActor interface
    void draw() const override;
    void update(float delta) override;
    void process_input() override;
    void set_rectangle(Rectangle rect) override { rect_ = rect; }
    Rectangle get_rectangle() const override { return rect_; }
    bool check_collision(const IActor& other) const override { return false; }

    // IWidget interface
    void on_click() override;
    void on_hover() override;
    void on_focus() override;
    bool contains_point(Vector2 point) const override;

 private:
    // Layout
    Rectangle rect_;

    // Text rendering
    std::string text_;
    int font_size_;
    Color normal_color_;
    Color hover_color_;
    Color click_color_;
    Color focused_color_;

    // Fallback colors (when no textures)
    Color bg_color_;
    int border_thickness_;

    // Texture-based rendering
    bool use_textures_ = false;
    Texture2D idle_texture_{0};
    Texture2D hover_texture_{0};
    Texture2D focused_texture_{0};
    Texture2D pressed_texture_{0};

    // Sprite sheet info (if using sheet)
    std::string sprite_sheet_path_;
    Rectangle idle_source_rect_{0, 0, 0, 0};
    Rectangle hover_source_rect_{0, 0, 0, 0};
    Rectangle focused_source_rect_{0, 0, 0, 0};
    Rectangle pressed_source_rect_{0, 0, 0, 0};

    // State
    bool is_pressed_ = false;

    // Helper methods
    void load_button_textures(const udjourney::scene::HUDData& hud);
    Texture2D get_current_texture() const;
    Rectangle get_current_source_rect() const;
};
}  // namespace udjourney
