// Copyright 2025 Quentin Cartier

#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLAYER_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLAYER_HPP_

#include <raylib/raylib.h>

#include <memory>
#include <string>
#include <vector>

#include "udjourney/IActor.hpp"
#include "udjourney/IGame.hpp"
#include "udjourney/IObserver.hpp"

class Player : public IActor {
 public:
    Player(const IGame &iGame, Rectangle iRect);
    ~Player();
    void draw() const override;
    void update(float iDelta) override;
    void process_input() override;
    void resolve_collision(const IActor &iActor) noexcept;
    void handle_collision(
        const std::vector<std::unique_ptr<IActor>> &iPlatforms) noexcept;
    void set_rectangle(Rectangle iRect) override { this->r = iRect; }
    [[nodiscard]] Rectangle get_rectangle() const override { return r; }
    [[nodiscard]] bool check_collision(const IActor &iOtherActor) const override {
        return CheckCollisionRecs(r, iOtherActor.get_rectangle());
    }

    // Observable
    void add_observer(IObserver *ioObserver);
    void remove_observer(IObserver *ioObserver);
    void notify(const std::string &iEvent);

    [[nodiscard]] inline constexpr uint8_t get_group_id() const override { return 0; }

 private:
    void _reset_jump() noexcept;
    Rectangle r;
    std::vector<IObserver *> observers;
    struct PImpl;
    std::unique_ptr<struct PImpl> m_pimpl;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLAYER_HPP_
