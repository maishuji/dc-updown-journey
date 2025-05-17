// Copyright 2025 Quentin Cartier
#pragma once

#include <iostream>
#include <string>

#include "udjourney/core/events/EventDispatcher.hpp"
#include "udjourney/core/events/EventPlugin.hpp"

namespace udjourney::core::events {

class ScoreEvent : public IEvent {
 public:
    static constexpr char const* TYPE = "ScoreEvent";

    explicit ScoreEvent(int value) : value(value) {}

    [[nodiscard]] std::string type() const override { return TYPE; }

    int value;
};

}  // namespace udjourney::core::events
