// Copyright 2025 Quentin Cartier
#pragma once

#include <string>

namespace udjourney::core::events {

class IEvent {
 public:
    virtual ~IEvent() = default;
    [[nodiscard]] virtual std::string type() const = 0;
};

}  // namespace udjourney::core::events
