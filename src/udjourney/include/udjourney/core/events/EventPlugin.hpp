// Copyright 2025 Quentin Cartier
#pragma once

#include "udjourney/core/events/EventDispatcher.hpp"

namespace udjourney::core::events {

class IEventPlugin {
 public:
    virtual ~IEventPlugin() = default;
    virtual void register_events(EventDispatcher& dispatcher) = 0;
};

}  // namespace udjourney::core::events
