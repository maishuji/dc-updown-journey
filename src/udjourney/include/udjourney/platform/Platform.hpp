// Copyright 2025 Quentin Cartier

#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_PLATFORM_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_PLATFORM_HPP_

#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include <memory>
#include <utility>

#include "udjourney/IActor.hpp"
#include "udjourney/IGame.hpp"
#include "udjourney/platform/behavior_strategies/PlatformBehaviorStrategy.hpp"
#include "udjourney/platform/reuse_strategies/PlatformReuseStrategy.hpp"

class Platform : public IActor {
 public:
    Platform(const IGame &game, Rectangle r, Color c = BLUE,
             bool y_repeated = false);
    void draw() const override;
    void update(float delta) override;

    [[nodiscard]] float get_dx() const noexcept { return dx; }
    void process_input() override;
    void set_rectangle(Rectangle r) override { this->r = r; }
    Rectangle get_rectangle() const override { return r; }
    bool check_collision(const IActor &other) const override {
        return CheckCollisionRecs(r, other.get_rectangle());
    }
    inline constexpr uint8_t get_group_id() const override { return 1; }
    bool constexpr is_y_repeated() { return y_repeated; }
    inline void reuse(PlatformReuseStrategy &strategy) noexcept {
        strategy.reuse(*this);
    }

    void set_behavior(std::unique_ptr<PlatformBehaviorStrategy> b) {
        behavior = std::move(b);
    }

    [[nodiscard]] inline const PlatformBehaviorStrategy *get_behavior() {
        return behavior.get();
    }

    void move(float x, float y) noexcept;

 private:
    float dx;
    Rectangle r;
    Color color = BLUE;
    bool y_repeated = false;
    std::unique_ptr<PlatformBehaviorStrategy> behavior;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_PLATFORM_HPP_
