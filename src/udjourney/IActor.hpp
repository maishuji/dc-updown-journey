#ifndef IACTOR_HPP
#define IACTOR_HPP

#include <memory>
#include <concepts>
#include <kos.h> // maple_device_t, cont_state_t

class IGame;

class IActor
{
public:
    IActor(IGame &game) : m_game(&game) {};
    virtual void draw() const = 0;
    virtual void update(float delta) = 0;
    virtual void process_input(cont_state_t *t) = 0;
    virtual Rectangle get_rectangle() const = 0;
    virtual bool check_collision(const IActor &other) const = 0;
    virtual constexpr uint8_t get_group_id() const = 0;

private:
    IGame *m_game = nullptr;
};

#endif // ACTOR_HPP