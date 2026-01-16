// Copyright 2025 Quentin Cartier
#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <chrono>

namespace udjourney {
namespace editor {

enum class ToastType { Success, Error, Info, Warning };

struct Toast {
    std::string message;
    ToastType type;
    std::chrono::steady_clock::time_point created_at;
    float duration;  // in seconds

    Toast(const std::string& msg, ToastType t, float dur = 3.0f) :
        message(msg),
        type(t),
        created_at(std::chrono::steady_clock::now()),
        duration(dur) {}

    bool is_expired() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                           now - created_at)
                           .count() /
                       1000.0f;
        return elapsed >= duration;
    }

    float get_alpha() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                           now - created_at)
                           .count() /
                       1000.0f;

        // Fade out in the last 0.5 seconds
        if (elapsed > duration - 0.5f) {
            return (duration - elapsed) / 0.5f;
        }
        return 1.0f;
    }
};

class ToastManager {
 public:
    ToastManager() = default;

    void add_toast(const std::string& message, ToastType type = ToastType::Info,
                   float duration = 3.0f) {
        toasts_.emplace_back(message, type, duration);
    }

    void draw() {
        // Remove expired toasts
        toasts_.erase(
            std::remove_if(toasts_.begin(),
                           toasts_.end(),
                           [](const Toast& t) { return t.is_expired(); }),
            toasts_.end());

        if (toasts_.empty()) return;

        // Get window dimensions
        ImGuiIO& io = ImGui::GetIO();
        float window_width = io.DisplaySize.x;
        float window_height = io.DisplaySize.y;

        // Toast dimensions
        const float toast_width = 400.0f;
        const float toast_padding = 15.0f;
        const float toast_spacing = 10.0f;

        // Position at bottom right
        float y_offset = window_height - toast_padding;

        for (auto it = toasts_.rbegin(); it != toasts_.rend(); ++it) {
            const Toast& toast = *it;
            float alpha = toast.get_alpha();

            // Calculate toast height based on text
            const float line_height = ImGui::GetTextLineHeightWithSpacing();
            const float toast_height = line_height * 2 + toast_padding * 2;

            y_offset -= toast_height;

            ImGui::SetNextWindowPos(
                ImVec2(window_width - toast_width - toast_padding, y_offset));
            ImGui::SetNextWindowSize(ImVec2(toast_width, toast_height));

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                                ImVec2(toast_padding, toast_padding));
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

            // Set background color based on type
            ImVec4 bg_color;
            switch (toast.type) {
                case ToastType::Success:
                    bg_color = ImVec4(0.2f, 0.7f, 0.3f, 0.9f);
                    break;
                case ToastType::Error:
                    bg_color = ImVec4(0.8f, 0.2f, 0.2f, 0.9f);
                    break;
                case ToastType::Warning:
                    bg_color = ImVec4(0.9f, 0.7f, 0.2f, 0.9f);
                    break;
                case ToastType::Info:
                default:
                    bg_color = ImVec4(0.2f, 0.4f, 0.8f, 0.9f);
                    break;
            }

            ImGui::PushStyleColor(ImGuiCol_WindowBg, bg_color);

            std::string window_name =
                "##Toast" + std::to_string(reinterpret_cast<uintptr_t>(&toast));

            if (ImGui::Begin(window_name.c_str(),
                             nullptr,
                             ImGuiWindowFlags_NoDecoration |
                                 ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoSavedSettings |
                                 ImGuiWindowFlags_NoFocusOnAppearing)) {
                ImGui::TextWrapped("%s", toast.message.c_str());
            }
            ImGui::End();

            ImGui::PopStyleColor();
            ImGui::PopStyleVar(3);

            y_offset -= toast_spacing;
        }
    }

 private:
    std::vector<Toast> toasts_;
};

}  // namespace editor
}  // namespace udjourney
