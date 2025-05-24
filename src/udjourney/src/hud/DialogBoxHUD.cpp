// Copyright 2025 Quentin Cartier
#include "udjourney/hud/DialogBoxHUD.hpp"

#include <raylib/raylib.h>

#include <sstream>
#include <string>
#include <utility>

DialogBoxHUD::DialogBoxHUD(Rectangle iRect) : m_rect(iRect) {
    // Initialize the dialog box with a default message
    m_text = "Hello, this is a dialog box!";
    m_is_focusable = true;  // Set focusable to true
}

void DialogBoxHUD::update(float deltaTime) {
    const int font_size = 20;
    const int padding = 10;
    float max_width = m_rect.width - 2 * padding;

    m_wrapped_lines.clear();
    std::istringstream stream(m_text);
    std::string word;
    std::string line;

    while (stream >> word) {
        std::string testLine = line.empty() ? word : line + " " + word;
        int line_width = MeasureText(testLine.c_str(), font_size);

        if (line_width > static_cast<int>(max_width)) {
            m_wrapped_lines.push_back(line);
            line = word;
        } else {
            line = testLine;
        }
    }

    if (!line.empty()) {
        m_wrapped_lines.push_back(line);
    }
}

void DialogBoxHUD::draw() const {
    // Draw the dialog box
    const int fontSize = 20;
    const int padding = 10;

    DrawRectangleRec(m_rect, DARKGRAY);

    float pos_y = m_rect.y + padding;
    for (const std::string& line : m_wrapped_lines) {
        DrawText(line.c_str(),
                 static_cast<int>(m_rect.x + padding),
                 static_cast<int>(pos_y),
                 fontSize,
                 WHITE);
        pos_y += fontSize + 4;
    }
}
