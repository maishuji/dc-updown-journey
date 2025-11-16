// Copyright 2025 Quentin Cartier
#include "udjourney/factories/ActorFactory.hpp"

#include <string>
#include <memory>
#include <utility>

#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/AnimSpriteController.hpp"
#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/interfaces/IComponent.hpp"
#include "udjourney/Monster.hpp"
#include "udjourney/Player.hpp"
#include "udjourney/loaders/AnimationConfigLoader.hpp"
#include <udj-core/CoreUtils.hpp>
#include "udjourney/core/events/EventDispatcher.hpp"

MonsterFactory::MonsterFactory(
    const IGame &game, Rectangle rect,
    udjourney::core::events::EventDispatcher &event_dispatcher) :
    m_game(game), m_rect(rect), m_event_dispatcher(event_dispatcher) {}

std::unique_ptr<IActor> MonsterFactory::create_actor() {
    // Load monster animation configuration from JSON file
    std::string config_path =
        std::string(ASSETS_BASE_PATH) + "animations/monster_animations.json";
    AnimSpriteController anim_controller =
        udjourney::loaders::AnimationConfigLoader::load_and_create(config_path);

    // Create a Monster actor instance with animation controller loaded from
    // JSON
    auto monster = std::make_unique<Monster>(
        m_game, m_rect, std::move(anim_controller), m_event_dispatcher);

    // Set patrol range relative to spawn position
    monster->set_patrol_range(m_rect.x - 100.0f, m_rect.x + 100.0f);
    monster->set_chase_range(200.0f);
    monster->set_attack_range(50.0f);

    return monster;
}

PlayerFactory::PlayerFactory(
    const IGame &game, Rectangle rect,
    udjourney::core::events::EventDispatcher &event_dispatcher) :
    m_game(game), m_rect(rect), m_event_dispatcher(event_dispatcher) {}

std::unique_ptr<IActor> PlayerFactory::create_actor() {
    // Load animation configuration from JSON file
    std::string config_path =
        std::string(ASSETS_BASE_PATH) + "animations/player_animations.json";
    AnimSpriteController anim_controller =
        udjourney::loaders::AnimationConfigLoader::load_and_create(config_path);

    // Create Player with animation controller loaded from JSON
    auto player = std::make_unique<Player>(
        m_game, m_rect, m_event_dispatcher, std::move(anim_controller));

    return player;
}
