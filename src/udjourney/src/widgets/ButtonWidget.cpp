// Copyright 2025 Quentin Cartier
#include <iostream>
#include <string>
#include <udj-core/CoreUtils.hpp>
#include <nlohmann/json.hpp>

#include "udjourney/widgets/ButtonWidget.hpp"
#include "udjourney/ActionDispatcher.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/managers/TextureManager.hpp"

// Helper function to parse color from JSON array string
static Color parse_color_from_property(const std::string& color_str) {
    try {
        // Parse JSON array string like "[255,255,0]" or "[255,255,0,255]"
        auto color_json = nlohmann::json::parse(color_str);
        if (color_json.is_array() && color_json.size() >= 3) {
            int r = color_json[0].get<int>();
            int g = color_json[1].get<int>();
            int b = color_json[2].get<int>();
            int a = (color_json.size() >= 4) ? color_json[3].get<int>() : 255;
            return Color{static_cast<unsigned char>(r),
                         static_cast<unsigned char>(g),
                         static_cast<unsigned char>(b),
                         static_cast<unsigned char>(a)};
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing color: " << e.what() << std::endl;
    }
    return WHITE;  // Default fallback
}

ButtonWidget::ButtonWidget(const IGame& game,
                           const udjourney::scene::FUDData& fud) :
    IWidget(game),
    text_("Button"),
    font_size_(24),
    normal_color_(WHITE),
    hover_color_(YELLOW),
    click_color_(GREEN),
    focused_color_(YELLOW),
    bg_color_(ColorAlpha(BLACK, 0.7f)),
    border_thickness_(2) {
    // Calculate anchor position based on screen size (same as
    // Game::draw_fuds_())
    const float kBaseWidth = 640.0f;
    const float kBaseHeight = 480.0f;

    float anchor_x = 0.0f;
    float anchor_y = 0.0f;

    switch (fud.anchor) {
        case udjourney::scene::FUDAnchor::TopLeft:
            anchor_x = 0;
            anchor_y = 0;
            break;
        case udjourney::scene::FUDAnchor::TopCenter:
            anchor_x = kBaseWidth / 2;
            anchor_y = 0;
            break;
        case udjourney::scene::FUDAnchor::TopRight:
            anchor_x = kBaseWidth;
            anchor_y = 0;
            break;
        case udjourney::scene::FUDAnchor::MiddleLeft:
            anchor_x = 0;
            anchor_y = kBaseHeight / 2;
            break;
        case udjourney::scene::FUDAnchor::MiddleCenter:
            anchor_x = kBaseWidth / 2;
            anchor_y = kBaseHeight / 2;
            break;
        case udjourney::scene::FUDAnchor::MiddleRight:
            anchor_x = kBaseWidth;
            anchor_y = kBaseHeight / 2;
            break;
        case udjourney::scene::FUDAnchor::BottomLeft:
            anchor_x = 0;
            anchor_y = kBaseHeight;
            break;
        case udjourney::scene::FUDAnchor::BottomCenter:
            anchor_x = kBaseWidth / 2;
            anchor_y = kBaseHeight;
            break;
        case udjourney::scene::FUDAnchor::BottomRight:
            anchor_x = kBaseWidth;
            anchor_y = kBaseHeight;
            break;
    }

    // Set rectangle from FUD: anchor + offset
    rect_ = Rectangle{anchor_x + fud.offset_x,
                      anchor_y + fud.offset_y,
                      fud.size_x,
                      fud.size_y};

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
            border_thickness_ =
                std::stoi(fud.properties.at("border_thickness"));
        }
        if (fud.properties.count("selectable")) {
            std::string selectable_str = fud.properties.at("selectable");
            is_selectable_ =
                (selectable_str == "true" || selectable_str == "1");
        }

        // Parse colors if provided
        if (fud.properties.count("normal_color")) {
            normal_color_ =
                parse_color_from_property(fud.properties.at("normal_color"));
        }
        if (fud.properties.count("hover_color")) {
            hover_color_ =
                parse_color_from_property(fud.properties.at("hover_color"));
        }
        if (fud.properties.count("click_color")) {
            click_color_ =
                parse_color_from_property(fud.properties.at("click_color"));
        }
        if (fud.properties.count("focused_color")) {
            focused_color_ =
                parse_color_from_property(fud.properties.at("focused_color"));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading button properties: " << e.what()
                  << std::endl;
    }

    // Load button textures if configured
    load_button_textures(fud);
}

void ButtonWidget::draw() const {
    // Widgets are drawn in screen space (no camera offset needed for UI
    // screens)
    Rectangle screen_rect = rect_;

    if (use_textures_) {
        // Draw texture-based button
        Texture2D current_texture = get_current_texture();
        Rectangle source_rect = get_current_source_rect();

        if (current_texture.id != 0) {
            // If no source rect defined, use full texture
            if (source_rect.width == 0 || source_rect.height == 0) {
                source_rect =
                    Rectangle{0,
                              0,
                              static_cast<float>(current_texture.width),
                              static_cast<float>(current_texture.height)};
            }

            // Draw stretched texture to fill button rect
            DrawTexturePro(current_texture,
                           source_rect,
                           screen_rect,
                           Vector2{0, 0},
                           0.0f,
                           WHITE);
        } else {
            // Fallback: draw colored rectangle if texture not loaded
            DrawRectangleRec(screen_rect, bg_color_);
            DrawRectangleLinesEx(screen_rect,
                                 static_cast<float>(border_thickness_),
                                 normal_color_);
        }
    } else {
        // Draw classic colored button
        Color border_color = normal_color_;

        if (is_pressed_) {
            border_color = click_color_;
        } else if (is_focused_) {
            border_color = focused_color_;
        } else if (is_hovered_) {
            border_color = hover_color_;
        }

        // Draw background
        DrawRectangleRec(screen_rect, bg_color_);

        // Draw border (thicker if focused)
        int border_width =
            is_focused_ ? border_thickness_ * 2 : border_thickness_;
        DrawRectangleLinesEx(
            screen_rect, static_cast<float>(border_width), border_color);
    }

    // Draw text centered (for both texture and colored buttons)
    Color text_color = normal_color_;
    if (is_pressed_) {
        text_color = click_color_;
    } else if (is_focused_) {
        text_color = focused_color_;
    } else if (is_hovered_) {
        text_color = hover_color_;
    }

    int text_width = MeasureText(text_.c_str(), font_size_);
    int text_x =
        static_cast<int>(screen_rect.x + (screen_rect.width - text_width) / 2);
    int text_y =
        static_cast<int>(screen_rect.y + (screen_rect.height - font_size_) / 2);

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
        std::cout << "[DEBUG] Button clicked with action: " << action_
                  << std::endl;
        // Execute the action through ActionDispatcher
        ActionDispatcher::execute(action_, const_cast<IGame*>(&get_game()));
    } else {
        std::cout << "[DEBUG] Button clicked but no action set!" << std::endl;
    }
    is_pressed_ = true;
}

void ButtonWidget::on_hover() { is_hovered_ = true; }

void ButtonWidget::on_focus() { is_focused_ = true; }

bool ButtonWidget::contains_point(Vector2 point) const {
    // Point is already in game coordinates from Game::update's mouse
    // transformation Just check collision directly
    return CheckCollisionPointRec(point, rect_);
}

void ButtonWidget::load_button_textures(const udjourney::scene::FUDData& fud) {
    auto& texture_manager = TextureManager::get_instance();

    // Check if we have texture configuration
    // Format: background_sheet with multiple states defined in properties
    // Properties: idle_tile_col, idle_tile_row, hover_tile_col, hover_tile_row,
    // etc.

    bool has_texture_config =
        !fud.background_sheet.empty() || !fud.background_image.empty();

    if (!has_texture_config) {
        use_textures_ = false;
        return;
    }

    // Load sprite sheet if specified
    if (!fud.background_sheet.empty()) {
        sprite_sheet_path_ = fud.background_sheet;

        // Load the sheet texture
        idle_texture_ = texture_manager.get_texture(fud.background_sheet);
        hover_texture_ =
            idle_texture_;  // Share texture, different source rects
        focused_texture_ = idle_texture_;
        pressed_texture_ = idle_texture_;

        if (idle_texture_.id != 0) {
            use_textures_ = true;

            int tile_size = fud.background_tile_size;

            // Load idle state (default from background config)
            idle_source_rect_ = Rectangle{
                static_cast<float>(fud.background_tile_col * tile_size),
                static_cast<float>(fud.background_tile_row * tile_size),
                static_cast<float>(fud.background_tile_width * tile_size),
                static_cast<float>(fud.background_tile_height * tile_size)};

            // Try to load hover state from properties
            if (fud.properties.count("hover_tile_col") &&
                fud.properties.count("hover_tile_row")) {
                int hover_col = std::stoi(fud.properties.at("hover_tile_col"));
                int hover_row = std::stoi(fud.properties.at("hover_tile_row"));
                hover_source_rect_ = Rectangle{
                    static_cast<float>(hover_col * tile_size),
                    static_cast<float>(hover_row * tile_size),
                    static_cast<float>(fud.background_tile_width * tile_size),
                    static_cast<float>(fud.background_tile_height * tile_size)};
            } else {
                hover_source_rect_ = idle_source_rect_;  // Fallback to idle
            }

            // Try to load focused/selected state from properties
            if (fud.properties.count("focused_tile_col") &&
                fud.properties.count("focused_tile_row")) {
                int focused_col =
                    std::stoi(fud.properties.at("focused_tile_col"));
                int focused_row =
                    std::stoi(fud.properties.at("focused_tile_row"));
                focused_source_rect_ = Rectangle{
                    static_cast<float>(focused_col * tile_size),
                    static_cast<float>(focused_row * tile_size),
                    static_cast<float>(fud.background_tile_width * tile_size),
                    static_cast<float>(fud.background_tile_height * tile_size)};
            } else {
                focused_source_rect_ = hover_source_rect_;  // Fallback to hover
            }

            // Try to load pressed state from properties
            if (fud.properties.count("pressed_tile_col") &&
                fud.properties.count("pressed_tile_row")) {
                int pressed_col =
                    std::stoi(fud.properties.at("pressed_tile_col"));
                int pressed_row =
                    std::stoi(fud.properties.at("pressed_tile_row"));
                pressed_source_rect_ = Rectangle{
                    static_cast<float>(pressed_col * tile_size),
                    static_cast<float>(pressed_row * tile_size),
                    static_cast<float>(fud.background_tile_width * tile_size),
                    static_cast<float>(fud.background_tile_height * tile_size)};
            } else {
                pressed_source_rect_ = hover_source_rect_;  // Fallback to hover
            }

            std::cout << "[DEBUG] Button '" << text_
                      << "' loaded texture from sheet: " << fud.background_sheet
                      << std::endl;
        }
    } else if (!fud.background_image.empty()) {
        // Load individual images (legacy support)
        idle_texture_ = texture_manager.get_texture(fud.background_image);

        if (idle_texture_.id != 0) {
            use_textures_ = true;
            hover_texture_ = idle_texture_;
            focused_texture_ = idle_texture_;
            pressed_texture_ = idle_texture_;

            std::cout << "[DEBUG] Button '" << text_
                      << "' loaded texture: " << fud.background_image
                      << std::endl;
        }
    }
}

Texture2D ButtonWidget::get_current_texture() const {
    if (is_pressed_) {
        return pressed_texture_.id != 0 ? pressed_texture_ : idle_texture_;
    } else if (is_focused_) {
        return focused_texture_.id != 0 ? focused_texture_ : idle_texture_;
    } else if (is_hovered_) {
        return hover_texture_.id != 0 ? hover_texture_ : idle_texture_;
    }
    return idle_texture_;
}

Rectangle ButtonWidget::get_current_source_rect() const {
    if (is_pressed_) {
        return pressed_source_rect_.width > 0 ? pressed_source_rect_
                                              : idle_source_rect_;
    } else if (is_focused_) {
        return focused_source_rect_.width > 0 ? focused_source_rect_
                                              : idle_source_rect_;
    } else if (is_hovered_) {
        return hover_source_rect_.width > 0 ? hover_source_rect_
                                            : idle_source_rect_;
    }
    return idle_source_rect_;
}
