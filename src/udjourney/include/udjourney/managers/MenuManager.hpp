// Copyright 2025 Quentin Cartier

#pragma once

#include <functional>
#include <string>
#include <vector>

#include <raylib/raylib.h>

#include "udjourney/hud/GameMenuHUD.hpp"  // MenuItem

namespace udjourney {

class IGame;

namespace scene {
class Scene;
}

namespace managers {

struct MenuConfig {
    std::string title;
    Rectangle rect;
    std::vector<MenuItem> items;
};

class MenuManager {
 public:
    using OnClosedCallback = std::function<void()>;

    explicit MenuManager(IGame &game);
    ~MenuManager() = default;

    // Load menu configuration from JSON file
    bool load_config(const std::string &config_path);

    // Show the pause/game menu.
    // If the current scene defines a game menu, it is used; otherwise uses
    // loaded config.
    void show_game_menu(const udjourney::scene::Scene *scene,
                        OnClosedCallback on_closed);

    void hide_game_menu();

    [[nodiscard]] bool is_showing() const { return m_showing; }

 private:
    IGame &m_game;
    bool m_showing = false;
    MenuConfig m_default_game_menu;
    bool m_config_loaded = false;

    MenuConfig build_menu_config_(const udjourney::scene::Scene *scene) const;
};

}  // namespace managers
}  // namespace udjourney
