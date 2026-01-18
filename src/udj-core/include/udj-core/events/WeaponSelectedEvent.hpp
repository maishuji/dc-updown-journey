// Copyright 2025 Quentin Cartier
#pragma once

#include <string>
#include <utility>

#include "udj-core/events/EventDispatcher.hpp"
#include "udj-core/events/EventPlugin.hpp"

namespace udjourney::core::events {

class WeaponSelectedEvent : public IEvent {
 public:
    static constexpr char const* TYPE = "WeaponSelectedEvent";

    WeaponSelectedEvent(std::string weaponId, std::string displayName,
                        int ammoInMag = -1, int ammoReserve = -1) :
        weaponId(std::move(weaponId)),
        displayName(std::move(displayName)),
        ammoInMag(ammoInMag),
        ammoReserve(ammoReserve) {}

    [[nodiscard]] std::string type() const override { return TYPE; }

    std::string weaponId;     // stable id: "pistol", "shotgun", ...
    std::string displayName;  // text for HUD
    int ammoInMag;            // optional
    int ammoReserve;          // optional
};

}  // namespace udjourney::core::events
