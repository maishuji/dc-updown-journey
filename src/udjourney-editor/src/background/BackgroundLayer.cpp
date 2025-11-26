// Copyright 2025 Quentin Cartier
#include "udjourney-editor/background/BackgroundLayer.hpp"

#include <string>

BackgroundLayer::BackgroundLayer(const std::string& name, float parallax_factor,
                                 int depth) :
    name_(name), parallax_factor_(parallax_factor), depth_(depth) {}

void BackgroundLayer::add_object(const BackgroundObject& obj) {
    objects_.push_back(obj);
}

void BackgroundLayer::remove_object(size_t index) {
    if (index < objects_.size()) {
        objects_.erase(objects_.begin() + index);
    }
}

void BackgroundLayer::clear_objects() { objects_.clear(); }
