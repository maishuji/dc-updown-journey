#include "udjourney/factories/ActorFactory.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/AnimSpriteController.hpp"
#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/interfaces/IComponent.hpp"
#include "udjourney/Monster.hpp"
#include "udjourney/Player.hpp"
#include "udjourney/managers/TextureManager.hpp"
#include "udjourney/SpriteAnim.hpp"

MonsterFactory::MonsterFactory(const IGame &game, Rectangle rect) :
    m_game(game), m_rect(rect) {}

std::unique_ptr<IActor> MonsterFactory::create_actor() {
    // Create a Monster actor instance
    // Using the player sprite for now - you can create custom monster sprites
    // later
    auto monster = std::make_unique<Monster>(m_game, m_rect, "char1-Sheet.png");

    // Set patrol range relative to spawn position
    monster->set_patrol_range(m_rect.x - 100.0f, m_rect.x + 100.0f);
    monster->set_chase_range(200.0f);
    monster->set_attack_range(50.0f);

    return monster;
}

PlayerFactory::PlayerFactory(const IGame &game, Rectangle rect) :
    m_game(game), m_rect(rect) {}

std::unique_ptr<IActor> PlayerFactory::create_actor() {
    // Animation constants
    static constexpr int SPRITE_WIDTH = 64;
    static constexpr int SPRITE_HEIGHT = 64;
    static constexpr int FRAMES_PER_ANIMATION = 8;
    static constexpr float FRAME_DURATION = 0.3f;

    // Get texture manager instance
    auto &texture_manager = TextureManager::get_instance();

    // Load sprite sheets
    Texture2D idle_sheet = texture_manager.get_texture("char1-Sheet.png");
    Texture2D run_sheet = texture_manager.get_texture("char1-run-Sheet.png");

    // Create animation controller with all player animations
    AnimSpriteController anim_controller;

    anim_controller.add_animation(PlayerState::IDLE,
                                  "idle",
                                  SpriteAnim(idle_sheet,
                                             SPRITE_WIDTH,
                                             SPRITE_HEIGHT,
                                             FRAME_DURATION,
                                             FRAMES_PER_ANIMATION,
                                             true));

    anim_controller.add_animation(PlayerState::RUNNING,
                                  "running",
                                  SpriteAnim(run_sheet,
                                             SPRITE_WIDTH,
                                             SPRITE_HEIGHT,
                                             FRAME_DURATION / 6.0F,
                                             FRAMES_PER_ANIMATION,
                                             true));

    anim_controller.add_animation(PlayerState::DASHING,
                                  "dashing",
                                  SpriteAnim(run_sheet,
                                             SPRITE_WIDTH,
                                             SPRITE_HEIGHT,
                                             FRAME_DURATION / 6.0F,
                                             FRAMES_PER_ANIMATION,
                                             true));

    anim_controller.add_animation(PlayerState::FALLING,
                                  "falling",
                                  SpriteAnim(run_sheet,
                                             SPRITE_WIDTH,
                                             SPRITE_HEIGHT,
                                             FRAME_DURATION / 6.0F,
                                             FRAMES_PER_ANIMATION,
                                             true));

    anim_controller.add_animation(PlayerState::JUMPING,
                                  "jumping",
                                  SpriteAnim(run_sheet,
                                             SPRITE_WIDTH,
                                             SPRITE_HEIGHT,
                                             FRAME_DURATION / 6.0F,
                                             FRAMES_PER_ANIMATION,
                                             true));

    // Create Player with animation controller
    auto player = std::make_unique<Player>(m_game,
                                           m_rect,
                                           m_game.get_event_dispatcher(),
                                           std::move(anim_controller));

    return player;
}