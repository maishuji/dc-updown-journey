// Copyright 2025 Quentin Cartier
#pragma once

class HUDComponent {
 public:
    virtual void update(float deltaTime) = 0;
    virtual void draw() const = 0;  // Changed from render() to draw()
    virtual ~HUDComponent() = default;
};
