// Copyright 2025 Quentin Cartier
#pragma once

#include <vector>
#include "udjourney/interfaces/IObserver.hpp"

class BonusManager {
 public:
    BonusManager() = default;

    void update(float delta);
    void add_observer(IObserver *observer);
    void remove_observer(IObserver *observer);

 private:
    float timeSinceLastBonus = 0.0F;
    std::vector<IObserver *> observers;
};
