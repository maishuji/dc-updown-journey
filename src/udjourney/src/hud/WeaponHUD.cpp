// Copyright 2025 Quentin Cartier
#include "udjourney/hud/WeaponHUD.hpp"

#include <raylib/raylib.h>

#include <string>
#include <utility>

#include "udjourney/core/events/IEvent.hpp"
#include "udjourney/core/events/WeaponSelectedEvent.hpp"

WeaponHUD::WeaponHUD(
    Vector2 iPosition,
    udjourney::core::events::EventDispatcher& ioEventDispatcher) :
    m_position(iPosition) {
    ioEventDispatcher.register_handler(
        udjourney::core::events::WeaponSelectedEvent::TYPE,
        [this](const udjourney::core::events::IEvent& evt) {
            const auto& weapon_ev = static_cast<
                const udjourney::core::events::WeaponSelectedEvent&>(evt);
            set_weapon_name(weapon_ev.weapon_name);
        });
}

void WeaponHUD::draw() const {
    const char* weapon_cstr =
        m_weapon_name.empty() ? "(none)" : m_weapon_name.c_str();

    DrawText(TextFormat("Weapon: %s", weapon_cstr),
             static_cast<int>(m_position.x),
             static_cast<int>(m_position.y),
             20,
             WHITE);
}

void WeaponHUD::set_weapon_name(const std::string& weapon_name) {
    m_weapon_name = weapon_name;
}
