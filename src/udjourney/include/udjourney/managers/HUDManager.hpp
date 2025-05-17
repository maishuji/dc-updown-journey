// Copyright 2025 Quentin Cartier
#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <utility>

#include "raylib/raylib.h"
#include "udjourney/hud/HUDComponent.hpp"

class HUDManager {
 public:
    void add(std::unique_ptr<HUDComponent> ioHudComponent);
    void update(float deltaTime);
    void draw() const;
 private:
    std::vector<std::unique_ptr<HUDComponent>> m_components;
};
