// Copyright 2025 Quentin Cartier
#include "udjourney/managers/HUDManager.hpp"

#include <string>
#include <utility>

void HUDManager::add_background_hud(
    std::unique_ptr<HUDComponent> ioHudComponent) {
    m_background_components.push_back(std::move(ioHudComponent));
}

void HUDManager::push_foreground_hud(
    std::unique_ptr<HUDComponent> ioHudComponent) {
    m_focus_stacks.push_back(std::move(ioHudComponent));
}

void HUDManager::pop_foreground_hud() {
    if (!m_focus_stacks.empty()) {
        m_focus_stacks.pop_back();
    }
}

void HUDManager::update(float deltaTime) {
    for (auto& component : m_background_components) {
        component->update(deltaTime);
    }

    for (auto& component : m_focus_stacks) {
        component->update(deltaTime);
    }
}

void HUDManager::draw() const {
    for (const auto& component : m_background_components) {
        component->draw();
    }

    for (const auto& component : m_focus_stacks) {
        component->draw();
    }
}

HUDComponent* HUDManager::get_component_by_type(const std::string& type_str) {
    for (auto& component : m_background_components) {
        if (component->get_type() == type_str) {
            return component.get();
        }
    }
    return nullptr;
}
