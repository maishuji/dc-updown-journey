// Copyright 2025 Quentin Cartier
#pragma once
#include <string>

class HUDComponent {
 public:
    virtual std::string get_type() const = 0;
    virtual void update(float deltaTime) = 0;
    virtual void draw() const = 0;  // Changed from render() to draw()
    virtual ~HUDComponent() = default;
};
