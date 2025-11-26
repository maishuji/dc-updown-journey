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

    // Setters
    void set_name(const std::string& name) { name_ = name; }
    void set_texture_file(const std::string& file) { texture_file_ = file; }
    void set_parallax_factor(float factor) { parallax_factor_ = factor; }
    void set_depth(int depth) { depth_ = depth; }

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
};
