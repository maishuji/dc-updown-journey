// Copyright 2025 Quentin Cartier
#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_BONUSMANAGER_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_BONUSMANAGER_HPP_


#include <vector>
#include "udjourney/IObserver.hpp"

class BonusManager {
 public:
    BonusManager() = default;

    void update(float delta);
    void add_observer(IObserver *observer);
    void remove_observer(IObserver *observer);

 private:
    float timeSinceLastBonus = 0.0f;
    std::vector<IObserver *> observers;
};
#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_BONUSMANAGER_HPP_
