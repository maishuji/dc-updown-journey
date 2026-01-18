// Copyright 2025 Quentin Cartier

#include "udjourney/managers/MenuManager.hpp"

#include <fstream>
#include <memory>

#include <nlohmann/json.hpp>

#include "udj-core/CoreUtils.hpp"
#include "udj-core/Logger.hpp"
#include "udjourney/Game.hpp"
#include "udjourney/hud/HUDComponent.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/scene/Scene.hpp"

using json = nlohmann::json;

namespace udjourney {
namespace managers {

MenuManager::MenuManager(IGame &game) : m_game(game) {
    // Set hardcoded fallback defaults in case JSON loading fails
    m_default_game_menu.title = "GAME MENU";
    m_default_game_menu.rect = Rectangle{160, 100, 320, 280};
    m_default_game_menu.items = {{"Resume", "resume_game", {}},
                                 {"Restart Level", "restart_level", {}},
                                 {"Level Select", "show_level_select", {}},
                                 {"Return to Title", "return_to_title", {}},
                                 {"Quit", "quit_game", {}}};
}

bool MenuManager::load_config(const std::string &config_path) {
    try {
        std::string full_path =
            udjourney::coreutils::get_assets_path(config_path);
        std::ifstream file(full_path);
        if (!file.is_open()) {
            udj::core::Logger::warning(
                "Failed to open menu config file: %, using defaults",
                full_path);
            return false;
        }

        json j;
        file >> j;

        if (!j.contains("menus") || !j["menus"].contains("game_menu")) {
            udj::core::Logger::warning(
                "Menu config missing 'menus.game_menu', using defaults");
            return false;
        }

        const auto &game_menu_json = j["menus"]["game_menu"];

        m_default_game_menu.title = game_menu_json.value("title", "GAME MENU");

        if (game_menu_json.contains("rect")) {
            const auto &rect_json = game_menu_json["rect"];
            m_default_game_menu.rect =
                Rectangle{rect_json.value("x", 160.0f),
                          rect_json.value("y", 100.0f),
                          rect_json.value("width", 320.0f),
                          rect_json.value("height", 280.0f)};
        }

        m_default_game_menu.items.clear();
        if (game_menu_json.contains("items") &&
            game_menu_json["items"].is_array()) {
            for (const auto &item_json : game_menu_json["items"]) {
                MenuItem item;
                item.label = item_json.value("label", "");
                item.action = item_json.value("command", "");

                if (item_json.contains("params") &&
                    item_json["params"].is_array()) {
                    for (const auto &param : item_json["params"]) {
                        if (param.is_string()) {
                            item.params.push_back(param.get<std::string>());
                        }
                    }
                }

                m_default_game_menu.items.push_back(item);
            }
        }

        m_config_loaded = true;
        udj::core::Logger::info("Loaded menu config from % with % items",
                                config_path,
                                m_default_game_menu.items.size());
        return true;
    } catch (const std::exception &e) {
        udj::core::Logger::error("Error loading menu config: %, using defaults",
                                 e.what());
        return false;
    }
}

MenuConfig MenuManager::build_menu_config_(
    const udjourney::scene::Scene *scene) const {
    // Priority 1: Scene-defined menu
    if (scene && scene->has_game_menu()) {
        const auto &menu_data = scene->get_game_menu();

        MenuConfig config;
        config.title = menu_data.title;
        config.rect = menu_data.rect;

        config.items.reserve(menu_data.items.size());
        for (const auto &item_data : menu_data.items) {
            config.items.push_back(
                {item_data.label, item_data.action, item_data.params});
        }

        udj::core::Logger::info("Using scene-defined game menu with % items",
                                config.items.size());
        return config;
    }

    // Priority 2: Loaded config from JSON
    if (m_config_loaded) {
        udj::core::Logger::info("Using loaded menu config with % items",
                                m_default_game_menu.items.size());
        return m_default_game_menu;
    }

    // Priority 3: Hardcoded fallback
    udj::core::Logger::info("Using hardcoded default game menu");
    return m_default_game_menu;
}

void MenuManager::show_game_menu(const udjourney::scene::Scene *scene,
                                 OnClosedCallback on_closed) {
    if (m_showing) return;

    m_showing = true;

    MenuConfig config = build_menu_config_(scene);

    auto game_menu =
        std::make_unique<GameMenuHUD>(m_game, config.rect, config.title);
    game_menu->load_menu_items(config.items);

    // Let Game decide what happens on close (resume/play vs keep UI state).
    game_menu->set_on_menu_closed([on_closed]() {
        if (on_closed) on_closed();
    });

    // Access HUD manager through Game cast (same pattern as LevelSelectManager)
    Game &game = static_cast<Game &>(m_game);
    game.get_hud_manager().push_foreground_hud(std::unique_ptr<HUDComponent>(
        static_cast<HUDComponent *>(game_menu.release())));
}

void MenuManager::hide_game_menu() {
    if (!m_showing) return;

    m_showing = false;

    Game &game = static_cast<Game &>(m_game);
    game.get_hud_manager().pop_foreground_hud();
}

}  // namespace managers
}  // namespace udjourney
