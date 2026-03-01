// Copyright 2025 Quentin Cartier

#pragma once

#include <raylib/raylib.h>

#include <functional>
#include <memory>
#include <string>

namespace udjourney {

class IGame;

namespace managers {

class LevelSelectManager {
 public:
    using OnLevelSelectedCallback = std::function<void(const std::string&)>;
    using OnCancelledCallback = std::function<void()>;

    explicit LevelSelectManager(IGame& game);
    ~LevelSelectManager() = default;

    // Show the level select menu with custom callbacks
    void show(const Rectangle& menu_rect, OnLevelSelectedCallback on_selected,
              OnCancelledCallback on_cancelled);

    // Show with default game rect (20% margins)
    void show(OnLevelSelectedCallback on_selected,
              OnCancelledCallback on_cancelled);

    // Hide the menu
    void hide();

    // Check if menu is currently shown
    [[nodiscard]] bool is_showing() const { return m_showing; }

    // Set custom levels directory (default: "assets/levels")
    void set_levels_directory(const std::string& dir);

 private:
    IGame& m_game;
    bool m_showing = false;
    std::string m_levels_dir;
};

}  // namespace managers
}  // namespace udjourney
