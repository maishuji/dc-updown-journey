// Copyright 2025 Quentin Cartier
#include "udjourney/hud/DialogBoxHUD.hpp"

#include <raylib/raylib.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace internal {

struct TextOptions {
    int font_size;
    int max_width;
    int padding;
};

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

struct DialogBoxHUD::PImpl {
    internal::TextOptions text_options;  // Text options for wrapping
    Rectangle button_rect;
    size_t page_cnt = 0;
    size_t current_page = 0;
    int_fast8_t line_per_page = 3;
};

size_t compute_page_count(size_t wrapped_lines_count, size_t lines_per_page) {
    if (lines_per_page == 0) {
        return 0;  // Avoid division by zero
    }
    return (wrapped_lines_count + lines_per_page - 1) / lines_per_page;
}

DialogBoxHUD::DialogBoxHUD(Rectangle iRect) :
    m_rect(iRect), m_pimpl(std::make_unique<DialogBoxHUD::PImpl>()) {
    // Initialize the dialog box with a default message
    m_text =
        "Hello, this is a dialog box!\n Testing severallinesof texts "
        "to see how it wraps and displays.\nPress Enter to continue or "
        "Escape to close.";
    m_is_focusable = true;  // Set focusable to true
    m_pimpl->text_options.font_size = 20;
    m_pimpl->text_options.padding = 10;
    m_pimpl->text_options.max_width =
        static_cast<int>(m_rect.width) -
        2 * static_cast<int>(m_pimpl->text_options.padding);
    internal::wrap_text(m_text, m_pimpl->text_options, m_wrapped_lines);
    m_pimpl->page_cnt =
        compute_page_count(m_wrapped_lines.size(), m_pimpl->line_per_page);

    m_pimpl->button_rect = Rectangle{
        m_rect.x + m_rect.width - 40,
        m_rect.y + m_rect.height - 40,
        30,
        30};  // Position the button at the bottom right of the dialog box
}

DialogBoxHUD::~DialogBoxHUD() = default;

void DialogBoxHUD::next_page() {
    if (m_pimpl->current_page < m_pimpl->page_cnt - 1) {
        m_pimpl->current_page++;
        m_on_next_callback();  // Add custom behavior for next page
    } else {
        m_on_finished_callback();
    }
}

void DialogBoxHUD::set_text(const std::string& text) {
    m_text = text;
    internal::wrap_text(m_text, m_pimpl->text_options, m_wrapped_lines);
    m_pimpl->page_cnt =
        compute_page_count(m_wrapped_lines.size(), m_pimpl->line_per_page);
}
void DialogBoxHUD::set_text(std::string&& text) {
    m_text = std::move(text);
    internal::wrap_text(m_text, m_pimpl->text_options, m_wrapped_lines);
    m_pimpl->page_cnt =
        compute_page_count(m_wrapped_lines.size(), m_pimpl->line_per_page);
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

    auto start_index = m_pimpl->current_page * m_pimpl->line_per_page;
    auto end_index =
        std::min(start_index + m_pimpl->line_per_page, m_wrapped_lines.size());
    for (size_t i = start_index; i < end_index; ++i) {
        if (i >= m_wrapped_lines.size()) {
            break;  // Prevent out-of-bounds access
        }
        const std::string& line = m_wrapped_lines[i];
        DrawText(line.c_str(),
                 static_cast<int>(m_rect.x + padding),
                 static_cast<int>(pos_y),
                 fontSize,
                 WHITE);
        pos_y += fontSize + 4;  // Add some spacing between lines
    }

    DrawRectangleRec(m_pimpl->button_rect, LIGHTGRAY);
}

void DialogBoxHUD::set_on_finished_callback(std::function<void()> callback) {
    m_on_finished_callback = std::move(callback);
}

void DialogBoxHUD::set_on_next_callback(std::function<void()> callback) {
    m_on_next_callback = std::move(callback);
}
