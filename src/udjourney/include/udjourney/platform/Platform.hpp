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
    Platform(const IGame &iGame, Rectangle iRect, Color iColor = BLUE,
             bool iIsRepeatedY = false);
    void draw() const override;
    void update(float delta) override;

    [[nodiscard]] float get_dx() const noexcept { return m_delta_x; }
    void process_input() override;
    void set_rectangle(Rectangle iRect) override { this->m_rect = iRect; }
    [[nodiscard]] Rectangle get_rectangle() const override { return m_rect; }
    [[nodiscard]] bool check_collision(
        const IActor &iOtherActor) const override {
        return CheckCollisionRecs(m_rect, iOtherActor.get_rectangle());
    }
    [[nodiscard]] inline constexpr uint8_t get_group_id() const override {
        return 1;
    }
    [[nodiscard]] inline constexpr auto is_y_repeated() const noexcept -> bool {
        return m_repeated_y;
    }
    inline void reuse(PlatformReuseStrategy &ioStrategy) noexcept {
        ioStrategy.reuse(*this);
    }

    void set_behavior(std::unique_ptr<PlatformBehaviorStrategy> ioBehavior) {
        m_behavior = std::move(ioBehavior);
    }

    [[nodiscard]] inline auto get_behavior()
        -> const PlatformBehaviorStrategy * {
        return m_behavior.get();
    }

    void move(float iValX, float iValY) noexcept;
    void resize(float iNewWidth, float iNewHeight) noexcept;

 private:
    float m_delta_x = 0.0F;
    Rectangle m_rect;
    Color m_color = BLUE;
    bool m_repeated_y = false;
    std::unique_ptr<PlatformBehaviorStrategy> m_behavior;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_PLATFORM_HPP_
