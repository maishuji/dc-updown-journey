// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include <any>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/platform/behavior_strategies/PlatformBehaviorStrategy.hpp"
#include "udjourney/platform/features/PlatformFeatureBase.hpp"
#include "udjourney/platform/reuse_strategies/PlatformReuseStrategy.hpp"
namespace udjourney {

class Platform : public IActor {
 public:
    Platform(const IGame& iGame, Rectangle iRect, Color iColor = BLUE,
             bool iIsRepeatedY = false,
             std::unique_ptr<PlatformReuseStrategy> reuseStrategy = nullptr);

    // Explicitly delete copy/move to avoid alignment issues
    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
    Platform(Platform&&) = delete;
    Platform& operator=(Platform&&) = delete;

    void draw() const override;
    void update(float delta) override;
    [[nodiscard]] Rectangle get_drawing_rect() const;

    [[nodiscard]] float get_dx() const noexcept { return m_delta_x; }
    void process_input() override;
    void set_rectangle(Rectangle iRect) override { this->m_rect = iRect; }
    [[nodiscard]] Rectangle get_rectangle() const override { return m_rect; }
    [[nodiscard]] bool check_collision(
        const IActor& iOtherActor) const override {
        return CheckCollisionRecs(m_rect, iOtherActor.get_rectangle());
    }
    [[nodiscard]] inline constexpr uint8_t get_group_id() const override {
        return 1;
    }
    [[nodiscard]] inline constexpr auto is_y_repeated() const noexcept -> bool {
        return m_repeated_y;
    }
    void reuse() noexcept {
        if (m_reuse_strategy) {
            m_reuse_strategy->reuse(*this);
        } else {
            // Default behavior: no reuse (level-based platform)
            set_state(ActorState::CONSUMED);
        }
    }

    void set_reuse_strategy(std::unique_ptr<PlatformReuseStrategy> strategy) {
        m_reuse_strategy = std::move(strategy);
    }

    [[nodiscard]] auto has_reuse_strategy() const noexcept -> bool {
        return m_reuse_strategy != nullptr;
    }

    void set_behavior(std::unique_ptr<PlatformBehaviorStrategy> ioBehavior) {
        m_behavior = std::move(ioBehavior);
    }

    [[nodiscard]] inline auto get_behavior() const
        -> const PlatformBehaviorStrategy* {
        return m_behavior.get();
    }

    void reset_behavior() {
        if (m_behavior) {
            m_behavior->reset();
        }
    }

    void move(float iValX, float iValY) noexcept;
    void resize(float iNewWidth, float iNewHeight) noexcept;

    void add_feature(std::unique_ptr<PlatformFeatureBase> feature);

    void set_texture_file(std::string texture_file) {
        m_texture_file = std::move(texture_file);
    }
    void set_texture_tiled(bool tiled) noexcept { m_texture_tiled = tiled; }
    [[nodiscard]] bool is_texture_tiled() const noexcept {
        return m_texture_tiled;
    }
    void set_use_atlas(bool use_atlas) noexcept { m_use_atlas = use_atlas; }
    void set_source_rect(Rectangle rect) noexcept { m_source_rect = rect; }
    [[nodiscard]] bool is_using_atlas() const noexcept { return m_use_atlas; }
    [[nodiscard]] Rectangle get_source_rect() const noexcept {
        return m_source_rect;
    }
    void clear_texture() { m_texture_file.clear(); }
    [[nodiscard]] bool has_texture() const noexcept {
        return !m_texture_file.empty();
    }

    [[nodiscard]] auto get_features() const
        -> const std::vector<std::unique_ptr<PlatformFeatureBase>>& {
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
    std::unique_ptr<PlatformReuseStrategy> m_reuse_strategy;
    Rectangle m_rect;
    Color m_color = BLUE;
    std::string m_texture_file;
    bool m_texture_tiled = false;
    bool m_use_atlas = false;
    Rectangle m_source_rect = {0, 0, 0, 0};
    bool m_repeated_y = false;
    std::unique_ptr<PlatformBehaviorStrategy> m_behavior;
    // Align vector to 8 bytes for SH4 safety
    alignas(8) std::vector<std::unique_ptr<PlatformFeatureBase>> m_features;
};
}  // namespace udjourney
