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
#include "udjourney/scene/Scene.hpp"
#include "udj-core/Logger.hpp"

namespace udjourney {

MonsterFactory::MonsterFactory(
    const IGame &game,
    udjourney::core::events::EventDispatcher &event_dispatcher) :
    m_game(game), m_event_dispatcher(event_dispatcher) {}

std::unique_ptr<IActor> MonsterFactory::create_actor() {
    // Default rectangle for when no rect is specified
    return create_actor(Rectangle{0, 0, 64, 64});
}

std::unique_ptr<IActor> MonsterFactory::create_actor(Rectangle rect) {
    // Load monster animation configuration from JSON file
    std::string config_path =
        std::string(ASSETS_BASE_PATH) + "animations/monster_animations.json";
    AnimSpriteController anim_controller =
        udjourney::loaders::AnimationConfigLoader::load_and_create(config_path);

    // Create a Monster actor instance with animation controller loaded from
    // JSON
    auto monster = std::make_unique<Monster>(
        m_game, rect, std::move(anim_controller), m_event_dispatcher);

    // Set patrol range relative to spawn position
    monster->set_patrol_range(rect.x - 100.0f, rect.x + 100.0f);
    monster->set_chase_range(200.0f);
    monster->set_attack_range(50.0f);

    return monster;
}

std::unique_ptr<IActor> MonsterFactory::create_actor_from_monster_data(
    const scene::MonsterSpawnData &monster_data) {
    Vector2 world_pos = udjourney::scene::Scene::tile_to_world_pos(
        monster_data.tile_x, monster_data.tile_y);

    Rectangle monster_rect = {
        world_pos.x,
        world_pos.y,
        64.0f,  // Monster width
        64.0f   // Monster height
    };

    // First load the monster preset to get animation configuration
    std::unique_ptr<udjourney::MonsterPreset> preset;
    if (!monster_data.preset_name.empty()) {
        preset = udjourney::MonsterPresetLoader::load_preset(
            monster_data.preset_name + ".json");
    }

    // Get animation preset file from the monster preset
    std::string animation_file = "monster_animations.json";  // Default fallback
    if (preset && !preset->animation_preset_file.empty()) {
        animation_file = preset->animation_preset_file;
    }

    // Load monster animation configuration from the preset
    std::string anim_preset_path =
        std::string(ASSETS_BASE_PATH) + "animations/" + animation_file;
    if (!udjourney::coreutils::file_exists(anim_preset_path)) {
        throw std::runtime_error("Monster animation config file not found: " +
                                 anim_preset_path);
    }

    AnimSpriteController monster_anim_controller =
        udjourney::loaders::AnimationConfigLoader::load_and_create(
            anim_preset_path);

    udjourney::Logger::info("DEBUG: Creating monster...");

    // Create monster with EventDispatcher
    auto monster = std::make_unique<Monster>(m_game,
                                             monster_rect,
                                             std::move(monster_anim_controller),
                                             m_event_dispatcher);

    udjourney::Logger::debug("Monster created successfully!");

    // Load preset if specified (this will apply all preset data)
    if (!monster_data.preset_name.empty()) {
        monster->load_preset(monster_data.preset_name);
    }

    // Configure monster behavior ranges
    float patrol_min = world_pos.x - (monster_data.patrol_range / 2.0f);
    float patrol_max = world_pos.x + (monster_data.patrol_range / 2.0f);
    monster->set_patrol_range(patrol_min, patrol_max);
    monster->set_chase_range(monster_data.chase_range);
    monster->set_attack_range(monster_data.attack_range);

    udjourney::Logger::info("Spawned monster at tile (%, %), world pos (%, %)",
                            monster_data.tile_x,
                            monster_data.tile_y,
                            world_pos.x,
                            world_pos.y);

    return monster;
}

PlayerFactory::PlayerFactory(
    const IGame &game,
    udjourney::core::events::EventDispatcher &event_dispatcher) :
    m_game(game), m_event_dispatcher(event_dispatcher) {}

std::unique_ptr<IActor> PlayerFactory::create_actor() {
    // Default rectangle for when no rect is specified
    return create_actor(Rectangle{0, 0, 20, 20});
}

std::unique_ptr<IActor> PlayerFactory::create_actor(Rectangle rect) {
    // Load animation configuration from JSON file
    std::string config_path =
        std::string(ASSETS_BASE_PATH) + "animations/player_animations.json";
    AnimSpriteController anim_controller =
        udjourney::loaders::AnimationConfigLoader::load_and_create(config_path);

    // Create Player with animation controller loaded from JSON
    auto player = std::make_unique<Player>(
        m_game, rect, m_event_dispatcher, std::move(anim_controller));

    return player;
}
}  // namespace udjourney
