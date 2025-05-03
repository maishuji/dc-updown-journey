// Copyright 2025 Quentin Cartier
#include "udjourney/platform/reuse_strategies/RandomizePositionStrategy.hpp"

#include <chrono>
#include <random>

#include "udjourney/IGame.hpp"
#include "udjourney/platform/Platform.hpp"

void RandomizePositionStrategy::reuse(Platform& platform) {
    // Randomize the X position of the platform
    // Y position is set to the bottom of the screen
    const auto game_rect = platform.get_game().get_rectangle();

    auto x_pos_range =
        static_cast<int>(game_rect.width - platform.get_rectangle().width);

    static auto seed =
        std::chrono::steady_clock::now().time_since_epoch().count();
    static std::mt19937 gen(static_cast<unsigned int>(seed));
    std::uniform_int_distribution<> distrib(0, x_pos_range);
    auto random_x = distrib(gen);
    const auto origin_rect = platform.get_rectangle();

    bool is_y_repeated = platform.is_y_repeated();
    if (is_y_repeated) {
        // DOTO(QCA) : To move out of this strategy later as it is no random
        // behavior In this case, y is repeated like a circular buffer Used for
        // borders
        platform.set_rectangle(
            Rectangle{origin_rect.x,
                      origin_rect.y + game_rect.height + origin_rect.height,
                      origin_rect.width,
                      origin_rect.height});
    } else {
        // platform
        platform.set_rectangle(Rectangle{static_cast<float>(random_x),
                                         game_rect.y + game_rect.height,
                                         origin_rect.width,
                                         origin_rect.height});
    }
    platform.set_state(ActorState::ONGOING);
}
