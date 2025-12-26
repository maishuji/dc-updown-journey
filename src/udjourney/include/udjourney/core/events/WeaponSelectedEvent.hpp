// Copyright 2025 Quentin Cartier
#pragma once

#include <string>

#include "udjourney/core/events/EventDispatcher.hpp"
#include "udjourney/core/events/EventPlugin.hpp"

namespace udjourney::core::events {

class WeaponSelectedEvent : public IEvent {
 public:
    static constexpr char const* TYPE = "WeaponSelectedEvent";

    explicit WeaponSelectedEvent(std::string weapon_name) :
        weapon_name(std::move(weapon_name)) {}

    [[nodiscard]] std::string type() const override { return TYPE; }

    std::string weapon_name;
};

}  // namespace udjourney::core::events
