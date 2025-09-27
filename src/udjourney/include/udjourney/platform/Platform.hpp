// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include <any>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/platform/behavior_strategies/PlatformBehaviorStrategy.hpp"
#include "udjourney/platform/features/PlatformFeatureBase.hpp"
#include "udjourney/platform/reuse_strategies/PlatformReuseStrategy.hpp"

class Platform : public IActor {
 public:
    Platform(const IGame &iGame, Rectangle iRect, Color iColor = BLUE,
             bool iIsRepeatedY = false);
    void draw() const override;
    void update(float delta) override;
    Rectangle get_drawing_rect() const;

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

    void add_feature(std::unique_ptr<PlatformFeatureBase> feature);

    auto get_features() const
        -> const std::vector<std::unique_ptr<PlatformFeatureBase>> & {
        return m_features;
    }

    auto set_collidable(bool iCollidable) noexcept -> void {
        m_collidable = iCollidable;
    }
    [[nodiscard]] auto is_collidable() const noexcept -> bool {
        return m_collidable;
    }

 private:
    bool m_collidable = true;
    float m_delta_x = 0.0F;
    Rectangle m_rect;
    Color m_color = BLUE;
    bool m_repeated_y = false;
    std::unique_ptr<PlatformBehaviorStrategy> m_behavior;
    std::vector<std::unique_ptr<PlatformFeatureBase>> m_features;
};
