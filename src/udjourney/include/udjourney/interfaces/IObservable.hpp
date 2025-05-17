// Copyright 2025 Quentin Cartier
#pragma once

#include <string>

#include "udjourney/interfaces/IObserver.hpp"
class IObservable {
 public:
    virtual void add_observer(IObserver* observer) = 0;
    virtual void remove_observer(IObserver* observer) = 0;
    virtual void notify(const std::string& event) = 0;
    virtual ~IObservable() = default;
};
