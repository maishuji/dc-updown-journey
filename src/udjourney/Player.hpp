// Copyright 2025 Quentin Cartier

#ifndef SRC_UDJOURNEY_PLAYER_HPP_
#define SRC_UDJOURNEY_PLAYER_HPP_

#include <kos.h>  // maple_device_t, cont_state_t
#include <raylib/raylib.h>

#include <memory>
#include <string>
#include <vector>

#include "IActor.hpp"
#include "IGame.hpp"
#include "IObserver.hpp"

class Player : public IActor {
 public:
    Player(const IGame &game, Rectangle r);
    void draw() const override;
    void update(float delta) override;
    void process_input(cont_state_t *cont) override;
    void resolve_collision(const IActor &platform) noexcept;
    void handle_collision(
        const std::vector<std::unique_ptr<IActor>> &platforms) noexcept;
    Rectangle get_rectangle() const override { return r; }
    bool check_collision(const IActor &other) const override {
        return CheckCollisionRecs(r, other.get_rectangle());
    }

    // Observable
    void add_observer(IObserver *observer);
    void remove_observer(IObserver *observer);
    void notify(const std::string &event);

    inline constexpr uint8_t get_group_id() const override { return 0; }

 private:
    Rectangle r;
};

#endif  // SRC_UDJOURNEY_PLAYER_HPP_
