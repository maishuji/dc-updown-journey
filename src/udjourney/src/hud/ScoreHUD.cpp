// Copyright 2025 Quentin Cartier
#include "udjourney/hud/ScoreHUD.hpp"

#include <raylib/raylib.h>

#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

#include "udjourney/core/events/EventDispatcher.hpp"
#include "udjourney/core/events/IEvent.hpp"
#include "udjourney/core/events/ScoreEvent.hpp"
#include "udjourney/interfaces/IActor.hpp"


ScoreHUD::ScoreHUD(
    Vector2 iPosition,
    udjourney::core::events::EventDispatcher& ioEventDispatcher) :
    m_position(iPosition) {
    ioEventDispatcher.register_handler(
        udjourney::core::events::ScoreEvent::TYPE,
        [this](const udjourney::core::events::IEvent& evt) {
            // Safe downcast after matching type
            const auto& score_ev =
                static_cast<const udjourney::core::events::ScoreEvent&>(evt);
            update_score_display(score_ev.value);
        });
}

void ScoreHUD::update_score_display(int value) { m_score += value; }
