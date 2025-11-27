// Copyright 2025 Quentin Cartier
#include "udjourney/widgets/ButtonWidget.hpp"
#include "udjourney/ActionDispatcher.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include <iostream>

ButtonWidget::ButtonWidget(const IGame& game, const udjourney::scene::FUDData& fud)
    : IWidget(game),
      text_("Button"),
      font_size_(24),
      normal_color_(WHITE),
      hover_color_(YELLOW),
      click_color_(GREEN),
      bg_color_(ColorAlpha(BLACK, 0.7f)),
      border_thickness_(2) {
    
    // Set rectangle from FUD
    rect_ = Rectangle{
        fud.offset_x,
        fud.offset_y,
        fud.size_x,
        fud.size_y
    };
    
    // Load properties from FUD
    try {
        if (fud.properties.count("text")) {
            text_ = fud.properties.at("text");
        }
        if (fud.properties.count("button_action")) {
            action_ = fud.properties.at("button_action");
        }
        if (fud.properties.count("font_size")) {
            font_size_ = std::stoi(fud.properties.at("font_size"));
        }
        if (fud.properties.count("border_thickness")) {
            border_thickness_ = std::stoi(fud.properties.at("border_thickness"));
        }
        
        // Parse colors if provided
        if (fud.properties.count("normal_color")) {
            auto color_str = fud.properties.at("normal_color");
            // Parse color from array format [r,g,b,a]
            // Simple implementation - you may want to enhance this
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading button properties: " << e.what() << std::endl;
    }
}

void ButtonWidget::draw() const {
    // Convert to screen coordinates
    Rectangle screen_rect = rect_;
    screen_rect.x -= get_game().get_rectangle().x;
    screen_rect.y -= get_game().get_rectangle().y;
    
    // Choose color based on state
    Color text_color = normal_color_;
    Color border_color = normal_color_;
    
    if (is_pressed_) {
        text_color = click_color_;
        border_color = click_color_;
    } else if (is_focused_) {
        // Focused (selected) state - use yellow and thicker border
        text_color = hover_color_;
        border_color = hover_color_;
    } else if (is_hovered_) {
        text_color = hover_color_;
        border_color = hover_color_;
    }
    
    // Draw background
    DrawRectangleRec(screen_rect, bg_color_);
    
    // Draw border (thicker if focused)
    int border_width = is_focused_ ? border_thickness_ * 2 : border_thickness_;
    DrawRectangleLinesEx(screen_rect, static_cast<float>(border_width), border_color);
    
    // Draw text centered
    int text_width = MeasureText(text_.c_str(), font_size_);
    int text_x = static_cast<int>(screen_rect.x + (screen_rect.width - text_width) / 2);
    int text_y = static_cast<int>(screen_rect.y + (screen_rect.height - font_size_) / 2);
    
    DrawText(text_.c_str(), text_x, text_y, font_size_, text_color);
}

void ButtonWidget::update(float delta) {
    // Update components if any
    update_components(delta);
}

void ButtonWidget::process_input() {
    // Input is handled externally by Game
}

void ButtonWidget::on_click() {
    if (!action_.empty()) {
        // Execute the action through ActionDispatcher
        ActionDispatcher::execute(action_, const_cast<IGame*>(&get_game()));
    }
    is_pressed_ = true;
}

void ButtonWidget::on_hover() {
    is_hovered_ = true;
}

void ButtonWidget::on_focus() {
    is_focused_ = true;
}

bool ButtonWidget::contains_point(Vector2 point) const {
    // Convert point to world coordinates
    Vector2 world_point = point;
    world_point.x += get_game().get_rectangle().x;
    world_point.y += get_game().get_rectangle().y;
    
    return CheckCollisionPointRec(world_point, rect_);
}
