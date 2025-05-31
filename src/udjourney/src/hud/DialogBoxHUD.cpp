// Copyright 2025 Quentin Cartier
#include "udjourney/hud/DialogBoxHUD.hpp"

#include <raylib/raylib.h>

#include <functional>
#include <sstream>
#include <string>
#include <utility>

namespace internal {

struct TextOptions {
    int font_size;
    int max_width;
    int padding;
} text_options;

void wrap_text(const std::string& iText, TextOptions iTextOptions,
               std::vector<std::string>& oWrappedLines) {
    oWrappedLines.clear();
    std::istringstream stream(iText);
    std::string word;
    std::string line;

    while (stream >> word) {
        std::string test_line = line.empty() ? word : line + " " + word;
        int line_width = MeasureText(test_line.c_str(), iTextOptions.font_size);

        if (line_width > static_cast<int>(iTextOptions.max_width)) {
            oWrappedLines.push_back(line);
            line = word;
        } else {
            line = test_line;
        }
    }

    if (!line.empty()) {
        oWrappedLines.push_back(line);
    }
}
}  // namespace internal

DialogBoxHUD::DialogBoxHUD(Rectangle iRect) : m_rect(iRect) {
    // Initialize the dialog box with a default message
    m_text = "Hello, this is a dialog box!";
    m_is_focusable = true;  // Set focusable to true
    internal::text_options.font_size = 20;
    internal::text_options.padding = 10;
    internal::text_options.max_width =
        static_cast<int>(m_rect.width - 2 * internal::text_options.padding);
    internal::wrap_text(m_text, internal::text_options, m_wrapped_lines);
}

void DialogBoxHUD::set_text(const std::string& text) {
    m_text = text;
    internal::wrap_text(m_text, internal::text_options, m_wrapped_lines);
}
void DialogBoxHUD::set_text(std::string&& text) {
    m_text = std::move(text);
    internal::wrap_text(m_text, internal::text_options, m_wrapped_lines);
}

void DialogBoxHUD::update(float deltaTime) {
    // To implement
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

void DialogBoxHUD::set_on_finished_callback(std::function<void()> callback) {
    m_on_finished_callback = std::move(callback);
}

void DialogBoxHUD::set_on_next_callback(std::function<void()> callback) {
    m_on_next_callback = std::move(callback);
}
