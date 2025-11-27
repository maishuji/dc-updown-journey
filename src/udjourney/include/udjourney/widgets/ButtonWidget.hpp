// Copyright 2025 Quentin Cartier
#pragma once
#include <raylib/raylib.h>
#include <string>
#include "udjourney/widgets/IWidget.hpp"
#include "udjourney/scene/Scene.hpp"

/**
 * @brief Button widget for menus and UI
 * 
 * Renders a clickable button with text, handles hover effects,
 * and triggers actions through the ActionDispatcher when clicked.
 */
class ButtonWidget : public IWidget {
 public:
    /**
     * @brief Construct button from FUD data
     * @param game Reference to game
     * @param fud FUD element containing button properties
     */
    ButtonWidget(const IGame& game, const udjourney::scene::FUDData& fud);
    
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
    Rectangle rect_;
    std::string text_;
    int font_size_;
    Color normal_color_;
    Color hover_color_;
    Color click_color_;
    Color bg_color_;
    int border_thickness_;
    
    // State
    bool is_pressed_ = false;
};
