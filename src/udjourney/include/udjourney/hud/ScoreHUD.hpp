// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>

#include <string>

#include "udjourney/hud/HUDComponent.hpp"
#include "udjourney/interfaces/IObservable.hpp"
#include "udjourney/interfaces/IObserver.hpp"

class ScoreHUD : public HUDComponent, public IObserver {
 public:
    explicit ScoreHUD(Vector2 position,
                      class IObservable *ioNullableObservable);
    ~ScoreHUD();

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

    void on_notify(const std::string &iEvent) override;

 private:
    int m_score = 0;
    Vector2 m_position;
    IObservable *m_observable_actor;
};
