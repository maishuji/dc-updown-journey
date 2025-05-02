// Copyright 2025 Quentin Cartier

#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_PLATFORM_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_PLATFORM_HPP_

#include <kos.h>  // maple_device_t, cont_state_t
#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include "udjourney/IActor.hpp"
#include "udjourney/IGame.hpp"
#include "udjourney/platform/reuse_strategies/PlatformReuseStrategy.hpp"

class Platform : public IActor {
 public:
    Platform(const IGame &game, Rectangle r, Color c = BLUE,
             bool y_repeated = false);
    void draw() const override;
    void update(float delta) override;
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

 private:
    Rectangle r;
    Color color = BLUE;
    bool y_repeated = false;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_PLATFORM_HPP_
