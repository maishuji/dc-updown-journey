// Copyright 2025 Quentin Cartier

#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_IACTOR_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_IACTOR_HPP_

#include <kos.h>  // maple_device_t, cont_state_t

#include <concepts>
#include <memory>

class IGame;

class IActor {
 public:
    explicit IActor(const IGame &game) : m_game(&game) {}
    virtual void draw() const = 0;
    virtual void update(float delta) = 0;
    virtual void process_input(cont_state_t *t) = 0;
    virtual Rectangle get_rectangle() const = 0;
    virtual bool check_collision(const IActor &other) const = 0;
    virtual constexpr uint8_t get_group_id() const = 0;

 private:
    const IGame *m_game = nullptr;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_IACTOR_HPP_
