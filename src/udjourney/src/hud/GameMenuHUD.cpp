// Copyright 2025 Quentin Cartier

#include "udjourney/hud/GameMenuHUD.hpp"

#include <string>
#include <utility>
#include <vector>

#include <udj-core/Logger.hpp>

#include "udjourney/ActionDispatcher.hpp"

namespace udjourney {

GameMenuHUD::GameMenuHUD(IGame& game, const Rectangle& rect,
                         const std::string& title) :
    m_game(game), m_rect(rect), m_title(title) {
    m_is_focusable = true;
}

void GameMenuHUD::draw() const {
    // Draw semi-transparent background
    DrawRectangle(0,
                  0,
                  static_cast<int>(m_rect.width + m_rect.x * 2),
                  static_cast<int>(m_rect.height + m_rect.y * 2),
                  ColorAlpha(BLACK, 0.7f));

    // Draw menu background
    DrawRectangleRec(m_rect, ColorAlpha(DARKGRAY, 0.9f));
    DrawRectangleLinesEx(m_rect, 2, YELLOW);

    // Draw menu title
    int title_width = MeasureText(m_title.c_str(), 30);
    DrawText(m_title.c_str(),
             static_cast<int>(m_rect.x + m_rect.width / 2 - title_width / 2),
             static_cast<int>(m_rect.y + 20),
             30,
             YELLOW);

    // Draw menu items
    float item_y = m_rect.y + 80;
    const float item_height = 40;

    for (size_t i = 0; i < m_items.size(); ++i) {
        Color text_color =
            (static_cast<int>(i) == m_selected_index) ? YELLOW : WHITE;
        Color bg_color = (static_cast<int>(i) == m_selected_index)
                             ? ColorAlpha(YELLOW, 0.3f)
                             : BLANK;

        // Draw selection background
        if (static_cast<int>(i) == m_selected_index) {
            DrawRectangle(static_cast<int>(m_rect.x + 20),
                          static_cast<int>(item_y - 5),
                          static_cast<int>(m_rect.width - 40),
                          static_cast<int>(item_height),
                          bg_color);
        }

        // Draw item text
        DrawText(m_items[i].label.c_str(),
                 static_cast<int>(m_rect.x + 40),
                 static_cast<int>(item_y),
                 20,
                 text_color);

        item_y += item_height + 10;
    }

    // Draw controls hint
    const char* hint = "UP/DOWN: Navigate  |  ENTER: Select  |  ESC: Close";
    int hint_width = MeasureText(hint, 15);
    DrawText(hint,
             static_cast<int>(m_rect.x + m_rect.width / 2 - hint_width / 2),
             static_cast<int>(m_rect.y + m_rect.height - 30),
             15,
             LIGHTGRAY);
}

void GameMenuHUD::update(float delta) {
    // Nothing to update for now
}

void GameMenuHUD::handle_input() {
    if (m_items.empty()) return;

#ifdef PLATFORM_DREAMCAST
    // Dreamcast D-pad support
    bool up_pressed = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP);
    bool down_pressed =
        IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
    bool select_pressed =
        IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    bool close_pressed = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_LEFT);
#else
    bool up_pressed = IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_Z);
    bool down_pressed = IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S);
    bool select_pressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    bool close_pressed = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P);
#endif

    // Navigation
    if (up_pressed) {
        m_selected_index =
            (m_selected_index - 1 + static_cast<int>(m_items.size())) %
            static_cast<int>(m_items.size());
    }

    if (down_pressed) {
        m_selected_index =
            (m_selected_index + 1) % static_cast<int>(m_items.size());
    }

    // Selection
    if (select_pressed) {
        execute_action(m_items[m_selected_index]);
    }

    // Close menu
    if (close_pressed) {
        if (m_on_menu_closed) {
            m_on_menu_closed();
        }
    }
}

void GameMenuHUD::load_menu_items(const std::vector<MenuItem>& items) {
    m_items = items;
    m_selected_index = 0;
}

void GameMenuHUD::set_on_menu_closed(std::function<void()> callback) {
    m_on_menu_closed = std::move(callback);
}

void GameMenuHUD::execute_action(const MenuItem& item) {
    udj::core::Logger::info("Executing menu action: %", item.action);

    // Build action string from item (action:param1:param2...)
    std::string action_string = item.action;
    for (const auto& param : item.params) {
        action_string += ":" + param;
    }

    ActionDispatcher::execute(action_string, &m_game);

    // Close menu after action execution
    // Don't close for actions that show another menu on top (level select)
    // or handle closing themselves (resume)
    if (m_on_menu_closed && item.action != "resume_game" &&
        item.action != "show_level_select2") {
        m_on_menu_closed();
    }
}

}  // namespace udjourney
