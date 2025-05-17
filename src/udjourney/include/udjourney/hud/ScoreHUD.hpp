// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>

#include <string>

#include "udjourney/core/events/EventDispatcher.hpp"
#include "udjourney/hud/HUDComponent.hpp"
#include "udjourney/interfaces/IObservable.hpp"
#include "udjourney/interfaces/IObserver.hpp"

class ScoreHUD : public HUDComponent {
 public:
    explicit ScoreHUD(
        Vector2 position,
        udjourney::core::events::EventDispatcher& ioEventDispatcher);

    void update(float deltaTime) override {
        // Optional animation/logic
    }

    void draw() const override {
        DrawText(TextFormat("Score: %d", m_score),
                 static_cast<int>(m_position.x),
                 static_cast<int>(m_position.y),
                 20,
                 WHITE);
    }

    void setScore(int newScore) { m_score = newScore; }

 private:
    void update_score_display(int value);
    int m_score = 0;
    Vector2 m_position;
};
