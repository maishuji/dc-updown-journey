#include "udjourney/factories/ActorFactory.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/AnimSpriteController.hpp"
#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/interfaces/IComponent.hpp"
#include "udjourney/Monster.hpp"

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
