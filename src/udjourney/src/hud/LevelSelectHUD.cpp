// Copyright 2025 Quentin Cartier
#include "udjourney/hud/LevelSelectHUD.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

#include "raylib/raylib.h"

#ifdef PLATFORM_DREAMCAST
namespace fs = std::experimental::filesystem;
#else
namespace fs = std::filesystem;
#endif

LevelSelectHUD::LevelSelectHUD(Rectangle rect, const std::string& levels_dir) :
    m_rect(rect), m_levels_dir(levels_dir) {
    m_is_focusable = true;
    scan_levels_directory();
}

void LevelSelectHUD::scan_levels_directory() {
    m_level_files.clear();

    try {
        if (fs::exists(m_levels_dir) && fs::is_directory(m_levels_dir)) {
            for (const auto& entry : fs::directory_iterator(m_levels_dir)) {
                if (entry.is_regular_file() &&
                    entry.path().extension() == ".json") {
                    m_level_files.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning levels directory: " << e.what()
                  << std::endl;
    }

    // Sort the level files for consistent ordering
    std::sort(m_level_files.begin(), m_level_files.end());

    // Ensure we have at least one level
    if (m_level_files.empty()) {
        std::cerr << "Warning: No level files found in " << m_levels_dir
                  << std::endl;
        m_level_files.push_back("No levels found");
    }

    // Reset selection to first item
    m_selected_index = 0;
}

void LevelSelectHUD::update(float deltaTime) {
    // Nothing to update for now
    (void)deltaTime;  // Suppress unused parameter warning
}

void LevelSelectHUD::draw() const { draw_menu(); }

void LevelSelectHUD::draw_menu() const {
    // Draw semi-transparent background
    DrawRectangle(static_cast<int>(m_rect.x),
                  static_cast<int>(m_rect.y),
                  static_cast<int>(m_rect.width),
                  static_cast<int>(m_rect.height),
                  Fade(BLACK, 0.8f));

    // Draw border
    DrawRectangleLinesEx(m_rect, 2.0f, WHITE);

    // Title
    const char* title = "SELECT LEVEL";
    int title_width = MeasureText(title, 24);
    DrawText(title,
             static_cast<int>(m_rect.x + (m_rect.width - title_width) / 2),
             static_cast<int>(m_rect.y + 20),
             24,
             WHITE);

    // Instructions
    const char* instructions = "UP/DOWN: Navigate  ENTER: Select  ESC: Cancel";
    int instr_width = MeasureText(instructions, 16);
    DrawText(instructions,
             static_cast<int>(m_rect.x + (m_rect.width - instr_width) / 2),
             static_cast<int>(m_rect.y + m_rect.height - 40),
             16,
             GRAY);

    // Level list
    const int start_y = static_cast<int>(m_rect.y + 70);
    const int line_height = 30;
    const int max_visible_items =
        static_cast<int>((m_rect.height - 130) / line_height);

    // Calculate visible range (simple scrolling)
    int start_index = 0;
    int end_index =
        std::min(static_cast<int>(m_level_files.size()), max_visible_items);

    if (m_selected_index >= max_visible_items) {
        start_index = m_selected_index - max_visible_items + 1;
        end_index = m_selected_index + 1;
    }

    for (int i = start_index; i < end_index; ++i) {
        if (i >= static_cast<int>(m_level_files.size())) break;

        int draw_y = start_y + (i - start_index) * line_height;
        Color text_color = (i == m_selected_index) ? YELLOW : WHITE;
        Color bg_color =
            (i == m_selected_index) ? Fade(YELLOW, 0.2f) : Color{0, 0, 0, 0};

        // Draw selection background
        if (i == m_selected_index) {
            DrawRectangle(static_cast<int>(m_rect.x + 10),
                          draw_y - 5,
                          static_cast<int>(m_rect.width - 20),
                          line_height,
                          bg_color);
        }

        // Draw level name (remove .json extension for display)
        std::string display_name = m_level_files[i];
        if (display_name.length() >= 5 &&
            display_name.substr(display_name.length() - 5) == ".json") {
            display_name = display_name.substr(0, display_name.length() - 5);
        }

        // Add selection indicator
        std::string text =
            (i == m_selected_index) ? "> " + display_name : "  " + display_name;

        DrawText(text.c_str(),
                 static_cast<int>(m_rect.x + 20),
                 draw_y,
                 20,
                 text_color);
    }

    // Show scroll indicators if needed
    if (m_level_files.size() > static_cast<size_t>(max_visible_items)) {
        if (start_index > 0) {
            DrawText("↑",
                     static_cast<int>(m_rect.x + m_rect.width - 30),
                     start_y - 20,
                     20,
                     WHITE);
        }
        if (end_index < static_cast<int>(m_level_files.size())) {
            DrawText("↓",
                     static_cast<int>(m_rect.x + m_rect.width - 30),
                     start_y + (max_visible_items - 1) * line_height + 20,
                     20,
                     WHITE);
        }
    }
}

void LevelSelectHUD::handle_input() {
    // Handle navigation input with proper key state tracking to avoid repeats
    bool up_pressed = false;
    bool down_pressed = false;
    bool enter_pressed = false;
    bool escape_pressed = false;

#ifdef PLATFORM_DREAMCAST
    up_pressed = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP);
    down_pressed = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
    enter_pressed = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    escape_pressed = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
#else
    up_pressed = IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
    down_pressed = IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S);
    enter_pressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    escape_pressed = IsKeyPressed(KEY_ESCAPE);
#endif

    // Navigate up
    if (up_pressed && !m_up_pressed_last_frame) {
        if (m_selected_index > 0) {
            m_selected_index--;
        }
    }

    // Navigate down
    if (down_pressed && !m_down_pressed_last_frame) {
        if (m_selected_index < static_cast<int>(m_level_files.size()) - 1) {
            m_selected_index++;
        }
    }

    // Select level
    if (enter_pressed && !m_enter_pressed_last_frame) {
        if (m_selected_index >= 0 &&
            m_selected_index < static_cast<int>(m_level_files.size())) {
            const std::string& selected_file = m_level_files[m_selected_index];
            if (selected_file != "No levels found" && m_on_level_selected) {
                // Construct full path
                std::string full_path = m_levels_dir + "/" + selected_file;
                m_on_level_selected(full_path);
            }
        }
    }

    // Cancel
    if (escape_pressed && !m_escape_pressed_last_frame) {
        if (m_on_cancelled) {
            m_on_cancelled();
        }
    }

    // Update last frame state
    m_up_pressed_last_frame = up_pressed;
    m_down_pressed_last_frame = down_pressed;
    m_enter_pressed_last_frame = enter_pressed;
    m_escape_pressed_last_frame = escape_pressed;
}

void LevelSelectHUD::set_on_level_selected_callback(
    std::function<void(const std::string&)> callback) {
    m_on_level_selected = std::move(callback);
}

void LevelSelectHUD::set_on_cancelled_callback(std::function<void()> callback) {
    m_on_cancelled = std::move(callback);
}
