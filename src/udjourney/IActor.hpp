#ifndef IACTOR_HPP
#define IACTOR_HPP

#include <memory>
#include <concepts>
#include <kos.h> // maple_device_t, cont_state_t

class IActor
{
public:
    virtual void draw() const = 0;
    virtual void update(float delta) = 0;
    virtual void process_input(cont_state_t *t) = 0;
};

#endif // ACTOR_HPP