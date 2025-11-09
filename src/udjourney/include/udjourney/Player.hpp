// Copyright 2025 Quentin Cartier
#pragma once
#include <raylib/raylib.h>

#include <memory>
#include <string>
#include <vector>

#include "udjourney/core/events/EventDispatcher.hpp"
#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/interfaces/IObservable.hpp"
#include "udjourney/interfaces/IObserver.hpp"
#include "udjourney/AnimSpriteController.hpp"

class Player : public IActor, public IObservable {
 public:
    Player(const IGame &iGame, Rectangle iRect,
           udjourney::core::events::EventDispatcher &ioDispatcher);
    ~Player() override;
    void draw() const override;
    void update(float iDelta) override;
    void process_input() override;
    void resolve_collision(const IActor &iActor) noexcept;
    void handle_collision(

        const std::vector<std::unique_ptr<IActor>> &iPlatforms) noexcept;
    void set_rectangle(Rectangle iRect) override { this->r = iRect; }
    [[nodiscard]] Rectangle get_rectangle() const override { return r; }
    [[nodiscard]] bool check_collision(
        const IActor &iOtherActor) const override {
        return CheckCollisionRecs(r, iOtherActor.get_rectangle());
    }

    // Observable
    void add_observer(IObserver *ioObserver) override;
    void remove_observer(IObserver *ioObserver) override;
    void notify(const std::string &iEvent) override;

    [[nodiscard]] inline constexpr uint8_t get_group_id() const override {
        return 0;
    }

    void set_invicibility(float iDuration) noexcept {
        m_invincibility_timer = iDuration;
    }
    [[nodiscard]] inline constexpr bool is_invincible() const noexcept {
        return m_invincibility_timer > 0.0F;
    }

 private:
    float m_invincibility_timer = 0.0F;

    void _reset_jump() noexcept;
    Rectangle r;
    std::vector<IObserver *> observers;
    struct PImpl;
    std::unique_ptr<struct PImpl> m_pimpl;
    Texture2D m_texture = {};
    Texture2D m_sprite_sheet = {};
    udjourney::core::events::EventDispatcher &m_dispatcher;

    // Animation state
    int m_current_frame = 0;
    float m_frame_timer = 0.0f;
    bool m_facing_right = true;

    // Animation constants
    static constexpr int SPRITE_WIDTH = 64;
    static constexpr int SPRITE_HEIGHT = 64;
    static constexpr int FRAMES_PER_ANIMATION = 8;
    static constexpr float FRAME_DURATION = 0.3f;  // Seconds per frame

    // Animation helper methods
    void update_animation(float delta_time);
    Rectangle get_sprite_frame_rect() const;
};
