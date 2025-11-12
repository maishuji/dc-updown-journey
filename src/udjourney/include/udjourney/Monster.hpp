// Copyright 2025 Quentin Cartier
#pragma once
#include <raylib/raylib.h>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "udjourney/AnimSpriteController.hpp"
#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/interfaces/IActorState.hpp"
#include "udjourney/interfaces/IGame.hpp"

// Forward declarations
class Player;

class Monster : public IActor {
 public:
    Monster(const IGame &game, Rectangle rect, const std::string &sprite_sheet);
    ~Monster() override = default;

    void draw() const override;
    void update(float delta) override;
    void process_input() override;
    void handle_collision(
        const std::vector<std::unique_ptr<IActor>> &actors) noexcept;

    void set_rectangle(Rectangle rect) override { rect_ = rect; }
    [[nodiscard]] Rectangle get_rectangle() const override { return rect_; }
    [[nodiscard]] bool check_collision(const IActor &other) const override {
        return CheckCollisionRecs(rect_, other.get_rectangle());
    }

    [[nodiscard]] inline constexpr uint8_t get_group_id() const override {
        return 2;  // Monster group ID
    }

    // Monster-specific methods
    void take_damage(float damage);
    void set_patrol_range(float min_x, float max_x);
    void set_chase_range(float range) { chase_range_ = range; }
    void set_attack_range(float range) { attack_range_ = range; }

    [[nodiscard]] bool is_alive() const { return health_ > 0.0f; }
    [[nodiscard]] bool is_attacking() const;

    // State management
    void change_state(const std::string &new_state);

    // Accessors for states to use
    [[nodiscard]] float get_patrol_speed() const { return speed_; }
    [[nodiscard]] float get_chase_speed() const { return speed_ * 1.5f; }
    [[nodiscard]] float get_chase_range() const { return chase_range_; }
    [[nodiscard]] float get_attack_range() const { return attack_range_; }
    [[nodiscard]] float get_patrol_left_bound() const { return patrol_min_x_; }
    [[nodiscard]] float get_patrol_right_bound() const { return patrol_max_x_; }
    [[nodiscard]] bool is_facing_right() const { return facing_right_; }

    void set_velocity_x(float vx) { velocity_x_ = vx; }
    void set_facing_right(bool right) { facing_right_ = right; }
    void reverse_direction();

    // Find player in game actors
    Player *find_player() const;

 private:
    const IGame &game_;
    Rectangle rect_;
    AnimSpriteController anim_controller_;

    // State pattern - using IActorState
    std::unordered_map<std::string, std::unique_ptr<IActorState>> states_;
    IActorState *current_state_ = nullptr;

    // Monster stats
    float health_ = 100.0f;
    float max_health_ = 100.0f;
    float damage_ = 10.0f;

    // Movement
    float speed_ = 50.0f;
    float velocity_x_ = 0.0f;
    float velocity_y_ = 0.0f;
    bool facing_right_ = true;
    bool grounded_ = false;

    // Physics
    static constexpr float GRAVITY = 1.0f;
    static constexpr float MAX_FALL_SPEED = 10.0f;

    // Patrol behavior
    float patrol_min_x_ = 0.0f;
    float patrol_max_x_ = 200.0f;
    bool patrol_direction_right_ = true;

    // AI ranges
    float chase_range_ = 200.0f;
    float attack_range_ = 50.0f;

    // Timers
    float attack_cooldown_ = 0.0f;

    // Animation constants
    static constexpr int SPRITE_WIDTH = 64;
    static constexpr int SPRITE_HEIGHT = 64;
    static constexpr int FRAMES_PER_ANIMATION = 8;
    static constexpr float FRAME_DURATION = 0.15f;
    static constexpr float ATTACK_COOLDOWN_TIME = 1.5f;

    // Helper methods
    void update_animations();
    void apply_gravity(float delta);
    void handle_border_collisions();
};
