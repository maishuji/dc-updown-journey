// Copyright 2025 Quentin Cartier

#include "udjourney/managers/LevelSelectManager.hpp"

#include <memory>
#include <string>

#include <udj-core/CoreUtils.hpp>

#include "udjourney/hud/HUDComponent.hpp"
#include "udjourney/hud/LevelSelectHUD.hpp"
#include "udjourney/Game.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/managers/HUDManager.hpp"

namespace udjourney {
namespace managers {

LevelSelectManager::LevelSelectManager(IGame& game) :
    m_game(game),
    m_levels_dir(udjourney::coreutils::get_assets_path("levels")) {}

void LevelSelectManager::show(const Rectangle& menu_rect,
                              OnLevelSelectedCallback on_selected,
                              OnCancelledCallback on_cancelled) {
    if (m_showing) return;  // Already showing

    m_showing = true;

    auto level_select_hud =
        std::make_unique<LevelSelectHUD>(menu_rect, m_levels_dir);

    // Set callbacks directly (Game will handle hiding)
    level_select_hud->set_on_level_selected_callback(
        [on_selected](const std::string& level_path) {
            if (on_selected) on_selected(level_path);
        });

    level_select_hud->set_on_cancelled_callback([on_cancelled]() {
        if (on_cancelled) on_cancelled();
    });

    // Access HUD manager through Game cast
    Game& game = static_cast<Game&>(m_game);
    game.get_hud_manager().push_foreground_hud(std::unique_ptr<HUDComponent>(
        static_cast<HUDComponent*>(level_select_hud.release())));
}

void LevelSelectManager::show(OnLevelSelectedCallback on_selected,
                              OnCancelledCallback on_cancelled) {
    Rectangle game_rect = m_game.get_rectangle();
    Rectangle menu_rect = {game_rect.width * 0.2f,
                           game_rect.height * 0.15f,
                           game_rect.width * 0.6f,
                           game_rect.height * 0.7f};
    show(menu_rect, on_selected, on_cancelled);
}

void LevelSelectManager::hide() {
    if (!m_showing) return;

    m_showing = false;

    // Access HUD manager through Game cast
    Game& game = static_cast<Game&>(m_game);
    game.get_hud_manager().pop_foreground_hud();
}

void LevelSelectManager::set_levels_directory(const std::string& dir) {
    m_levels_dir = dir;
}

}  // namespace managers
}  // namespace udjourney
