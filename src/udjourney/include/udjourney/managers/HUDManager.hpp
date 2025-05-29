// Copyright 2025 Quentin Cartier
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "raylib/raylib.h"
#include "udjourney/hud/HUDComponent.hpp"

class HUDManager {
 public:
    void add_background_hud(std::unique_ptr<HUDComponent> ioHudComponent);
    void push_foreground_hud(std::unique_ptr<HUDComponent> ioHudComponent);
    void pop_foreground_hud();
    void update(float deltaTime);
    void handle_input() {
        if (!m_focus_stacks.empty()) {
            m_focus_stacks.back()->handle_input();
        }
    }

    [[nodiscard]] inline bool has_focus() const noexcept {
        return !m_focus_stacks.empty();
    }

    [[nodiscard]] inline HUDComponent* get_top_focus() const {
        return m_focus_stacks.empty() ? nullptr : m_focus_stacks.back().get();
    }
    void draw() const;

    HUDComponent* get_component_by_type(const std::string& type_str);

 private:
    std::vector<std::unique_ptr<HUDComponent>> m_background_components;
    std::vector<std::unique_ptr<HUDComponent>> m_focus_stacks;
};
