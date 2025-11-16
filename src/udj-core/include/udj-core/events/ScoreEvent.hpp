// Copyright 2025 Quentin Cartier
#pragma once

#include <iostream>
#include <string>

#include "udj-core/events/EventDispatcher.hpp"
#include "udj-core/events/EventPlugin.hpp"

namespace udjourney::core::events {

class ScoreEvent : public IEvent {
 public:
    static constexpr char const* TYPE = "ScoreEvent";

    explicit ScoreEvent(int value) : value(value) {}

    [[nodiscard]] std::string type() const override { return TYPE; }

    int value;
};

}  // namespace udjourney::core::events
