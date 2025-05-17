// Copyright 2025 Quentin Cartier
#pragma once
#include <string>

class IObserver {
 public:
    virtual ~IObserver() = default;
    virtual void on_notify(const std::string &event) = 0;
};
