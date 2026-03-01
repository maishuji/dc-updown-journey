// Copyright 2025 Quentin Cartier
#include "udjourney/factories/UiFactory.hpp"

#include <memory>
#include <utility>

#include <udj-core/Logger.hpp>

#include "udjourney/Game.hpp"
#include "udjourney/hud/scene/HeartHealthHUD.hpp"
#include "udjourney/hud/scene/ScoreDisplayHUD.hpp"
#include "udjourney/hud/scene/WeaponHUD.hpp"
#include "udjourney/widgets/WidgetFactory.hpp"

namespace udjourney {

UiFactory::Result UiFactory::create(
    const udjourney::scene::Scene& scene, udjourney::Game& game,
    udjourney::core::events::EventDispatcher& event_dispatcher) {
    Result result;

    const auto& ui_list = scene.get_huds();

    for (const auto& ui : ui_list) {
        // 1) First, try to create a scene HUD overlay (non-actor).
        std::unique_ptr<udjourney::hud::scene::IHUD> hud = nullptr;

        if (ui.type_id == "score_display") {
            hud = std::make_unique<udjourney::hud::scene::ScoreDisplayHUD>(
                ui, &game);
        } else if (ui.type_id == "heart_health") {
            hud = std::make_unique<udjourney::hud::scene::HeartHealthHUD>(
                ui, event_dispatcher);
        } else if (ui.type_id == "weapon_display") {
            auto weapon_hud =
                std::make_unique<udjourney::hud::scene::WeaponHUD>(
                    ui, event_dispatcher);
            weapon_hud->load_projectile_presets("projectiles.json");
            hud = std::move(weapon_hud);
        }

        if (hud) {
            result.scene_huds.push_back(std::move(hud));
            continue;
        }

        // 2) Otherwise, try to create a widget actor (interactive UI).
        if (auto widget = WidgetFactory::create_from_fud(ui, game)) {
            result.widget_actors.push_back(std::move(widget));
            continue;
        }

        // 3) Unknown UI element type.
        udjourney::Logger::warning(
            "Unknown UI element type_id: % (name: %)", ui.type_id, ui.name);
    }

    return result;
}

}  // namespace udjourney
