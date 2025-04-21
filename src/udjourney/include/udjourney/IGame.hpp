// Copyright 2025 Quentin Cartier

#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_IGAME_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_IGAME_HPP_

#include <kos.h>  // maple_device_t, cont_state_t
#include <raylib/raylib.h>  // Rectangle

#include <memory>  // unique_ptr

class IActor;

class IGame {
 public:
    virtual void run() = 0;
    virtual void update() = 0;
    virtual void process_input(cont_state_t *cont) = 0;
    virtual void add_actor(std::unique_ptr<IActor> actor) = 0;
    virtual void remove_actor(IActor *actor) = 0;
    virtual Rectangle get_rectangle() const = 0;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_IGAME_HPP_
