// Copyright 2025 Quentin Cartier
#pragma once

#include <vector>
#include <string>

struct BackgroundObject {
    std::string sprite_name;  // Can be preset name or custom sprite path
    float x = 0.0f;
    float y = 0.0f;
    float scale = 1.0f;
    float rotation = 0.0f;

    // Preset tile information (for rendering from sprite sheet)
    std::string sprite_sheet;  // Path to sprite sheet
    int tile_size = 128;       // Size of each tile
    int tile_row = 0;          // Row in sprite sheet
    int tile_col = 0;          // Column in sprite sheet
};

class BackgroundLayer {
 public:
    BackgroundLayer() = default;
    BackgroundLayer(const std::string& name, float parallax_factor, int depth);

    // Getters
    const std::string& get_name() const { return name_; }
    const std::string& get_texture_file() const { return texture_file_; }
    float get_parallax_factor() const { return parallax_factor_; }
    int get_depth() const { return depth_; }
    const std::vector<BackgroundObject>& get_objects() const {
        return objects_;
    }
    std::vector<BackgroundObject>& get_objects_mut() { return objects_; }

    // Auto-scroll getters
    bool get_auto_scroll_enabled() const { return auto_scroll_enabled_; }
    float get_scroll_speed_x() const { return scroll_speed_x_; }
    float get_scroll_speed_y() const { return scroll_speed_y_; }
    bool get_repeat() const { return repeat_; }

    // Setters
    void set_name(const std::string& name) { name_ = name; }
    void set_texture_file(const std::string& file) { texture_file_ = file; }
    void set_parallax_factor(float factor) { parallax_factor_ = factor; }
    void set_depth(int depth) { depth_ = depth; }

    // Auto-scroll setters
    void set_auto_scroll_enabled(bool enabled) {
        auto_scroll_enabled_ = enabled;
    }
    void set_scroll_speed_x(float speed) { scroll_speed_x_ = speed; }
    void set_scroll_speed_y(float speed) { scroll_speed_y_ = speed; }
    void set_repeat(bool repeat) { repeat_ = repeat; }

    // Object management
    void add_object(const BackgroundObject& obj);
    void remove_object(size_t index);
    void clear_objects();

 private:
    std::string name_ = "New Layer";
    std::string texture_file_;
    float parallax_factor_ = 0.5f;  // 0.0 = static, 1.0 = moves with foreground
    int depth_ = 0;                 // Lower depth = rendered first (background)
    std::vector<BackgroundObject> objects_;
    // Auto-scroll properties
    bool auto_scroll_enabled_ = true;
    float scroll_speed_x_ = 0.0f;
    float scroll_speed_y_ = 0.0f;
    bool repeat_ = false;  // True = infinite loop, False = stop at edge
};
