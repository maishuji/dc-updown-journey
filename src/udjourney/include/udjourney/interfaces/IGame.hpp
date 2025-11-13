// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>  // Rectangle

#include <memory>  // unique_ptr
#include <vector>  // vector

class IActor;
class Player;

class IGame {
 public:
    virtual void run() = 0;
    virtual void update() = 0;
    virtual void process_input() = 0;
    virtual void add_actor(std::unique_ptr<IActor> actor) = 0;
    virtual void remove_actor(IActor* actor) = 0;
    [[nodiscard]] virtual Rectangle get_rectangle() const = 0;
    virtual void on_checkpoint_reached(float x, float y) const = 0;
    [[nodiscard]] virtual Player* get_player() const = 0;
};
