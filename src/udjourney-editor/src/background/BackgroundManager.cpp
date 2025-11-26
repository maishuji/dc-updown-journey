// Copyright 2025 Quentin Cartier
#include "udjourney-editor/background/BackgroundManager.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

bool BackgroundManager::add_layer(const BackgroundLayer& layer) {
    if (layers_.size() >= MAX_LAYERS) {
        return false;
    }
    layers_.push_back(layer);
    sort_layers_by_depth();
    return true;
}

void BackgroundManager::remove_layer(size_t index) {
    if (index < layers_.size()) {
        layers_.erase(layers_.begin() + index);
        if (selected_layer_.has_value() && selected_layer_.value() == index) {
            selected_layer_.reset();
        } else if (selected_layer_.has_value() &&
                   selected_layer_.value() > index) {
            selected_layer_ = selected_layer_.value() - 1;
        }
    }
}

void BackgroundManager::move_layer_up(size_t index) {
    if (index > 0 && index < layers_.size()) {
        std::swap(layers_[index], layers_[index - 1]);
        if (selected_layer_.has_value() && selected_layer_.value() == index) {
            selected_layer_ = index - 1;
        }
    }
}

void BackgroundManager::move_layer_down(size_t index) {
    if (index < layers_.size() - 1) {
        std::swap(layers_[index], layers_[index + 1]);
        if (selected_layer_.has_value() && selected_layer_.value() == index) {
            selected_layer_ = index + 1;
        }
    }
}

BackgroundLayer* BackgroundManager::get_layer(size_t index) {
    if (index < layers_.size()) {
        return &layers_[index];
    }
    return nullptr;
}

const BackgroundLayer* BackgroundManager::get_layer(size_t index) const {
    if (index < layers_.size()) {
        return &layers_[index];
    }
    return nullptr;
}

void BackgroundManager::add_object(size_t layer_idx,
                                   const BackgroundObject& obj) {
    if (layer_idx < layers_.size()) {
        layers_[layer_idx].add_object(obj);
    }
}

void BackgroundManager::remove_object(size_t layer_idx, size_t obj_idx) {
    if (layer_idx < layers_.size()) {
        layers_[layer_idx].remove_object(obj_idx);
    }
}

void BackgroundManager::select_layer(size_t index) {
    if (index < layers_.size()) {
        selected_layer_ = index;
    }
}

void BackgroundManager::clear_selection() { selected_layer_.reset(); }

void BackgroundManager::clear() {
    layers_.clear();
    selected_layer_.reset();
}

void BackgroundManager::sort_layers_by_depth() {
    std::sort(layers_.begin(),
              layers_.end(),
              [](const BackgroundLayer& a, const BackgroundLayer& b) {
                  return a.get_depth() < b.get_depth();
              });
}

void BackgroundManager::load_from_file(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open background file: " << filename
                      << std::endl;
            return;
        }

        nlohmann::json j;
        file >> j;

        layers_.clear();
        for (const auto& layer_json : j["layers"]) {
            BackgroundLayer layer;
            layer.set_name(layer_json.value("name", "Layer"));
            layer.set_texture_file(layer_json.value("texture_file", ""));
            layer.set_parallax_factor(
                layer_json.value("parallax_factor", 0.5f));
            layer.set_depth(layer_json.value("depth", 0));

            if (layer_json.contains("objects")) {
                for (const auto& obj_json : layer_json["objects"]) {
                    BackgroundObject obj;
                    obj.sprite_name = obj_json.value("sprite_name", "");
                    obj.x = obj_json.value("x", 0.0f);
                    obj.y = obj_json.value("y", 0.0f);
                    obj.scale = obj_json.value("scale", 1.0f);
                    obj.rotation = obj_json.value("rotation", 0.0f);

                    // Load preset tile information
                    obj.sprite_sheet = obj_json.value("sprite_sheet", "");
                    obj.tile_size = obj_json.value("tile_size", 128);
                    obj.tile_row = obj_json.value("tile_row", 0);
                    obj.tile_col = obj_json.value("tile_col", 0);

                    layer.add_object(obj);
                }
            }

            layers_.push_back(layer);
        }

        sort_layers_by_depth();
    } catch (const std::exception& e) {
        std::cerr << "Error loading background: " << e.what() << std::endl;
    }
}

void BackgroundManager::save_to_file(const std::string& filename) const {
    try {
        nlohmann::json j = to_json();
        std::ofstream file(filename);
        file << j.dump(2);
    } catch (const std::exception& e) {
        std::cerr << "Error saving background: " << e.what() << std::endl;
    }
}

nlohmann::json BackgroundManager::to_json() const {
    nlohmann::json j;
    j["layers"] = nlohmann::json::array();

    for (const auto& layer : layers_) {
        nlohmann::json layer_json;
        layer_json["name"] = layer.get_name();
        layer_json["texture_file"] = layer.get_texture_file();
        layer_json["parallax_factor"] = layer.get_parallax_factor();
        layer_json["depth"] = layer.get_depth();

        layer_json["objects"] = nlohmann::json::array();
        for (const auto& obj : layer.get_objects()) {
            nlohmann::json obj_json;
            obj_json["sprite_name"] = obj.sprite_name;
            obj_json["x"] = obj.x;
            obj_json["y"] = obj.y;
            obj_json["scale"] = obj.scale;
            obj_json["rotation"] = obj.rotation;

            // Save preset tile information
            obj_json["sprite_sheet"] = obj.sprite_sheet;
            obj_json["tile_size"] = obj.tile_size;
            obj_json["tile_row"] = obj.tile_row;
            obj_json["tile_col"] = obj.tile_col;

            layer_json["objects"].push_back(obj_json);
        }

        j["layers"].push_back(layer_json);
    }

    return j;
}

void BackgroundManager::from_json(const nlohmann::json& j) {
    try {
        // Clear existing layers
        clear();

        if (!j.contains("layers") || !j["layers"].is_array()) {
            return;
        }

        for (const auto& layer_json : j["layers"]) {
            BackgroundLayer layer;
            layer.set_name(layer_json.value("name", "Unnamed Layer"));
            layer.set_texture_file(layer_json.value("texture_file", ""));
            layer.set_parallax_factor(
                layer_json.value("parallax_factor", 1.0f));
            layer.set_depth(layer_json.value("depth", 0));

            // Load objects
            if (layer_json.contains("objects") &&
                layer_json["objects"].is_array()) {
                for (const auto& obj_json : layer_json["objects"]) {
                    BackgroundObject obj;
                    obj.sprite_name = obj_json.value("sprite_name", "");
                    obj.x = obj_json.value("x", 0.0f);
                    obj.y = obj_json.value("y", 0.0f);
                    obj.scale = obj_json.value("scale", 1.0f);
                    obj.rotation = obj_json.value("rotation", 0.0f);
                    obj.sprite_sheet = obj_json.value("sprite_sheet", "");
                    obj.tile_size = obj_json.value("tile_size", 128);
                    obj.tile_row = obj_json.value("tile_row", 0);
                    obj.tile_col = obj_json.value("tile_col", 0);

                    layer.add_object(obj);
                }
            }

            add_layer(layer);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing background JSON: " << e.what() << std::endl;
    }
}
