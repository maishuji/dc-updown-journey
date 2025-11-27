// Copyright 2025 Quentin Cartier
#pragma once
#include <raylib/raylib.h>
#include <memory>
#include <string>
#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/components/HealthComponent.hpp"
#include "udjourney/components/MovementComponent.hpp"
#include "udjourney/components/SpriteComponent.hpp"
#include "udjourney/MonsterPreset.hpp"
#include "udjourney/loaders/MonsterPresetLoader.hpp"
#include "udjourney/AnimSpriteController.hpp"

// Forward declarations
class Player;
class Monster;
class IGame;
struct EditorMonster;

/**
 * @brief Factory for creating actors with components
 *
 * Centralizes actor creation logic and simplifies the process of
 * composing actors from components based on presets or editor data.
 */
class ActorFactory {
 public:
    /**
     * @brief Create a monster from editor data
     * @param game Reference to the game
     * @param editor_monster Monster placement data from editor
     * @param preset_loader Loader for monster presets
     * @param dispatcher Event dispatcher reference
     * @return Unique pointer to the created monster actor
     */
    static std::unique_ptr<Monster> create_monster_from_editor(
        const IGame& game, const EditorMonster& editor_monster,
        udjourney::loaders::MonsterPresetLoader& preset_loader,
        udjourney::core::events::EventDispatcher& dispatcher);

    /**
     * @brief Create a monster from preset name
     * @param game Reference to the game
     * @param preset_name Name of the monster preset
     * @param position World position to spawn the monster
     * @param preset_loader Loader for monster presets
     * @param dispatcher Event dispatcher reference
     * @return Unique pointer to the created monster actor
     */
    static std::unique_ptr<Monster> create_monster_from_preset(
        const IGame& game, const std::string& preset_name, Vector2 position,
        udjourney::loaders::MonsterPresetLoader& preset_loader,
        udjourney::core::events::EventDispatcher& dispatcher);

    /**
     * @brief Add standard components to a monster
     * @param monster The monster actor
     * @param preset Monster preset with stats
     * @param anim_controller Animation controller
     */
    static void add_monster_components(
        Monster& monster, const udjourney::MonsterPreset& preset,
        const AnimSpriteController& anim_controller);

    /**
     * @brief Add standard components to the player
     * @param player The player actor
     * @param max_health Player's maximum health
     * @param speed Player's movement speed
     * @param anim_controller Animation controller
     */
    static void add_player_components(
        Player& player, int max_health, float speed,
        const AnimSpriteController& anim_controller);
};
