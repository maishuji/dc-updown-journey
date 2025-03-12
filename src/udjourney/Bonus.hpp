#ifndef BONUS_HPP
#define BONUS_HPP

#include <kos.h> // maple_device_t, cont_state_t
#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include "IActor.hpp"
#include "IGame.hpp"

class Bonus : public IActor
{
public:
    Bonus(IGame &game, Rectangle r);
    void draw() const override;
    void update(float delta) override;
    void process_input(cont_state_t *t) override;
    Rectangle get_rectangle() const override { return r; }
    bool check_collision(const IActor &other) const override { return CheckCollisionRecs(r, other.get_rectangle()); }
    inline constexpr uint8_t get_group_id() const override { return 2; }

private:
    Rectangle r;
};

#endif // BONUS_HPP