// Copyright 2025 Quentin Cartier

#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_HPP_

#include <kos.h>  // maple_device_t, cont_state_t
#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include "udjourney/IActor.hpp"
#include "udjourney/IGame.hpp"

class Platform : public IActor {
 public:
    Platform(const IGame &game, Rectangle r);
    void draw() const override;
    void update(float delta) override;
    void process_input(cont_state_t *t) override;
    void set_rectangle(Rectangle r) override { this->r = r; }
    Rectangle get_rectangle() const override { return r; }
    bool check_collision(const IActor &other) const override {
        return CheckCollisionRecs(r, other.get_rectangle());
    }
    inline constexpr uint8_t get_group_id() const override { return 1; }

 private:
    Rectangle r;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_HPP_
