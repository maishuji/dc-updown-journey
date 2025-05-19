// Copyright 2025 Quentin Cartier
#include "udjourney/managers/HUDManager.hpp"

#include <string>
#include <utility>

void HUDManager::add(std::unique_ptr<HUDComponent> ioHudComponent) {
    m_components.push_back(std::move(ioHudComponent));
}

void HUDManager::update(float deltaTime) {
    for (auto& component : m_components) {
        component->update(deltaTime);
    }
}

void HUDManager::draw() const {
    for (const auto& component : m_components) {
        component->draw();
    }
}

HUDComponent* HUDManager::get_component_by_type(const std::string& type_str) {
    for (auto& component : m_components) {
        if (component->get_type() == type_str) {
            return component.get();
        }
    }
    return nullptr;
}
