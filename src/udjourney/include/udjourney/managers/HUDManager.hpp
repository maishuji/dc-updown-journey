// Copyright 2025 Quentin Cartier
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "raylib/raylib.h"
#include "udjourney/hud/HUDComponent.hpp"

class HUDManager {
 public:
    void add_background_hud(std::unique_ptr<HUDComponent> ioHudComponent);
    void push_foreground_hud(std::unique_ptr<HUDComponent> ioHudComponent);
    void pop_foreground_hud();
    void update(float deltaTime);
    void draw() const;

    HUDComponent* get_component_by_type(const std::string& type_str);

 private:
    std::vector<std::unique_ptr<HUDComponent>> m_background_components;
    std::vector<std::unique_ptr<HUDComponent>> m_focus_stacks;
};
