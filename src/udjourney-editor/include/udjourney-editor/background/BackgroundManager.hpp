// Copyright 2025 Quentin Cartier
#pragma once

#include <string>
#include <vector>
#include <optional>

#include "udjourney-editor/background/BackgroundLayer.hpp"

class BackgroundManager {
 public:
    static constexpr int MAX_LAYERS = 5;

    BackgroundManager() = default;
    ~BackgroundManager() = default;

    // Layer management
    bool add_layer(
        const BackgroundLayer& layer);  // Returns false if max layers reached
    void remove_layer(size_t index);
    void move_layer_up(size_t index);
    void move_layer_down(size_t index);
    size_t get_layer_count() const { return layers_.size(); }
    bool can_add_layer() const { return layers_.size() < MAX_LAYERS; }

    // Layer access
    const std::vector<BackgroundLayer>& get_layers() const { return layers_; }
    std::vector<BackgroundLayer>& get_layers_mut() { return layers_; }
    BackgroundLayer* get_layer(size_t index);
    const BackgroundLayer* get_layer(size_t index) const;

    // Object management
    void add_object(size_t layer_idx, const BackgroundObject& obj);
    void remove_object(size_t layer_idx, size_t obj_idx);

    // Selection
    void select_layer(size_t index);
    void clear_selection();
    std::optional<size_t> get_selected_layer() const { return selected_layer_; }

    // Persistence
    void load_from_file(const std::string& filename);
    void save_to_file(const std::string& filename) const;

    // Clear all
    void clear();

 private:
    std::vector<BackgroundLayer> layers_;
    std::optional<size_t> selected_layer_;

    void sort_layers_by_depth();
};
