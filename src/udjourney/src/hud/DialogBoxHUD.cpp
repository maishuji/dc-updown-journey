#include "udjourney/hud/DialogBoxHUD.hpp"

#include <raylib/raylib.h>

#include <string>
#include <utility>

DialogBoxHUD::DialogBoxHUD(Rectangle iRect) : m_rect(iRect) {
    // Initialize the dialog box with a default message
    m_text = "Hello, this is a dialog box!";
    m_is_focusable = true;  // Set focusable to true
}

void DialogBoxHUD::update(float deltaTime) {
    // Update logic for the dialog box
    // For example, you could animate the dialog box or change its text
}

void DialogBoxHUD::draw() const {
    // Draw the dialog box
    DrawRectangleRec(m_rect, DARKGRAY);
    DrawText(m_text.c_str(),
             static_cast<int>(m_rect.x + 10),
             static_cast<int>(m_rect.y + 10),
             20,
             WHITE);
}