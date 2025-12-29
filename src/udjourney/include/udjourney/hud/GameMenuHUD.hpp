// Copyright 2025 Quentin Cartier

#pragma once

#include "udjourney/hud/HUDComponent.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include <raylib/raylib.h>
#include <vector>
#include <string>
#include <functional>

namespace udjourney {

struct MenuItem {
    std::string label;
    std::string action;
    std::vector<std::string> params;
};

class GameMenuHUD : public HUDComponent {
 public:
    GameMenuHUD(IGame& game, const Rectangle& rect,
                const std::string& title = "GAME MENU");
    ~GameMenuHUD() override = default;

    void draw() const override;
    void update(float delta) override;
    void handle_input() override;
    [[nodiscard]] std::string get_type() const override {
        return "GameMenuHUD";
    }

    // Load menu items from scene data
    void load_menu_items(const std::vector<MenuItem>& items);

    // Set callbacks for menu actions
    void set_on_menu_closed(std::function<void()> callback);

 private:
    IGame& m_game;
    Rectangle m_rect;
    std::string m_title;
    std::vector<MenuItem> m_items;
    int m_selected_index = 0;
    std::function<void()> m_on_menu_closed;

    void execute_action(const MenuItem& item);
};

}  // namespace udjourney
