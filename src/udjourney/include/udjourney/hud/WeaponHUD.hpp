// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>

#include <string>

#include "udjourney/core/events/EventDispatcher.hpp"
#include "udjourney/hud/HUDComponent.hpp"

class WeaponHUD : public HUDComponent {
 public:
    explicit WeaponHUD(
        Vector2 position,
        udjourney::core::events::EventDispatcher& ioEventDispatcher);

    [[nodiscard]] std::string get_type() const override { return "WeaponHUD"; }

    void update(float deltaTime) override {
        // Optional animation/logic
    }

    void draw() const override;

 private:
    void set_weapon_name(const std::string& weapon_name);

    std::string m_weapon_name = "";
    Vector2 m_position;
};
