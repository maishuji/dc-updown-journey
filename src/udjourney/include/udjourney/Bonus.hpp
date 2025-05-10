// Copyright 2025 Quentin Cartier

#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_BONUS_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_BONUS_HPP_

#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include "udjourney/IActor.hpp"
#include "udjourney/IGame.hpp"

class Bonus : public IActor {
 public:
    Bonus(const IGame &iGame, Rectangle iRect);
    void draw() const override;
    void update(float iDelta) override;
    void process_input() override;
    void set_rectangle(Rectangle iRect) override { this->m_rect = iRect; }
    [[nodiscard]] Rectangle get_rectangle() const override { return m_rect; }
    [[nodiscard]] bool check_collision(const IActor &other) const override {
        return CheckCollisionRecs(m_rect, other.get_rectangle());
    }
    [[nodiscard]] inline constexpr uint8_t get_group_id() const override {
        return 2;
    }

 private:
    Rectangle m_rect;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_BONUS_HPP_
