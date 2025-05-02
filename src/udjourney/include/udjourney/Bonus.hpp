// Copyright 2025 Quentin Cartier

#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_BONUS_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_BONUS_HPP_

#include <kos.h>  // maple_device_t, cont_state_t
#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include "udjourney/IActor.hpp"
#include "udjourney/IGame.hpp"

class Bonus : public IActor {
 public:
    Bonus(const IGame &game, Rectangle r);
    void draw() const override;
    void update(float delta) override;
    void process_input() override;
    void set_rectangle(Rectangle r) override { this->r = r; }
    Rectangle get_rectangle() const override { return r; }
    bool check_collision(const IActor &other) const override {
        return CheckCollisionRecs(r, other.get_rectangle());
    }
    inline constexpr uint8_t get_group_id() const override { return 2; }

 private:
    Rectangle r;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_BONUS_HPP_
