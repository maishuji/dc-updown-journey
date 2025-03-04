#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <memory>

#include <raylib/raylib.h>
#include <kos.h> // maple_device_t, cont_state_t

#include "IGame.hpp"
#include "IActor.hpp"

class Player : public IActor
{
public:
    Player(IGame &game, Rectangle r);
    void draw() const override;
    void update(float delta) override;
    void process_input(cont_state_t *cont) override;
    void resolve_collision(const IActor &platform) noexcept;
    void handle_collision(const std::vector<std::unique_ptr<IActor>> &platforms) noexcept;
    Rectangle get_rectangle() const override { return r; }
    bool check_collision(const IActor &other) const override { return CheckCollisionRecs(r, other.get_rectangle()); }

private:
    Rectangle r;
};

#endif // PLAYER_HPP