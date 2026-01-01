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
#include "udjourney/scene/LevelPhysicsConfig.hpp"

// Forward declarations
namespace udjourney {
class ProjectilePresetLoader;
struct ProjectilePreset;

class Player : public IActor, public IObservable {
 public:
    Player(const IGame &iGame, Rectangle iRect,
           udjourney::core::events::EventDispatcher &ioDispatcher,
           AnimSpriteController anim_controller,
           const scene::LevelPhysicsConfig &physics_config);
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

    // Attack system for testing
    void attack_nearby_monsters();

    // Projectile shooting system
    void load_projectile_presets(const std::string &config_file);
    void set_current_projectile(const std::string &preset_name);
    void cycle_projectile_type();
    bool can_shoot() const { return shoot_cooldown_ <= 0.0f; }
    const udjourney::ProjectilePreset *get_current_projectile_preset() const;
    const std::string &get_current_projectile_name() const {
        return current_projectile_preset_;
    }
    void reset_shoot_cooldown();
    Vector2 get_shoot_position() const;
    Vector2 get_shoot_direction() const;

 private:
    float m_invincibility_timer = 0.0F;

    // Physics configuration
    scene::LevelPhysicsConfig m_physics_config;

    // Projectile system
    std::unique_ptr<udjourney::ProjectilePresetLoader> projectile_loader_;
    std::string current_projectile_preset_ = "bullet";
    float shoot_cooldown_ = 0.0f;
    static constexpr float SHOOT_COOLDOWN_DURATION = 0.3f;

    void _reset_jump() noexcept;
    Rectangle r;
    std::vector<IObserver *> observers;
    struct PImpl;
    std::unique_ptr<struct PImpl> m_pimpl;
    Texture2D m_texture = {};
    AnimSpriteController anim_controller_;
    udjourney::core::events::EventDispatcher &m_dispatcher;

    // Animation state tracking
    bool m_facing_right = true;
    bool m_is_running = false;

    // Animation constants
    static constexpr int SPRITE_WIDTH = 64;
    static constexpr int SPRITE_HEIGHT = 64;
    static constexpr int FRAMES_PER_ANIMATION = 8;
    static constexpr float FRAME_DURATION = 0.3f;  // Seconds per frame

    // Animation helper methods
    void update_animation_state();
};
}  // namespace udjourney
