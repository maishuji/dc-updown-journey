// Copyright 2025 Quentin Cartier
#include "udjourney/managers/BonusManager.hpp"

#include <chrono>
#include <iostream>
#include <algorithm>
#include <random>

#include "udjourney/interfaces/IObserver.hpp"

const float kMinBonusInterval = 2.0F;  // Minimum time between bonus spawns

void BonusManager::update(float delta) {
    // Update the bonus spawner
    timeSinceLastBonus += delta;
    if (timeSinceLastBonus >= kMinBonusInterval) {
        timeSinceLastBonus = 0.0F;
        static auto seed =
            std::chrono::steady_clock::now().time_since_epoch().count();
        static std::mt19937 gen(static_cast<unsigned int>(seed));
        static std::uniform_int_distribution<> dist(0, 99);
        if (dist(gen) < 50) {
            // Notify observers to spawn a bonus

            auto pos_x = dist(gen);
            auto pos_y = dist(gen);

            for (auto *listener : observers) {
                listener->on_notify("2;" + std::to_string(pos_x) + "+" +
                                    std::to_string(pos_y));
            }
        }
    }
}

void BonusManager::add_observer(IObserver *observer) {
    observers.push_back(observer);
}

void BonusManager::remove_observer(IObserver *observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer),
                    observers.end());
}
