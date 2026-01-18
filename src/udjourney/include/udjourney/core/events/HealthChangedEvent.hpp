// Copyright 2025 Quentin Cartier
#pragma once

#include <string>

#include "udjourney/core/events/EventDispatcher.hpp"
#include "udjourney/core/events/EventPlugin.hpp"

namespace udjourney::core::events {

class HealthChangedEvent : public IEvent {
 public:
    static constexpr char const* TYPE = "HealthChangedEvent";

    HealthChangedEvent(int current_health, int max_health) :
        current_health(current_health), max_health(max_health) {}

    [[nodiscard]] std::string type() const override { return TYPE; }

    // NOTE: These are in HealthComponent units (player uses half-hearts).
    int current_health;
    int max_health;
};

}  // namespace udjourney::core::events
