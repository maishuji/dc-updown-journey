#ifndef ACTOR_HPP
#define ACTOR_HPP

#include <memory>
#include <concepts>

class IActor
{
public:
    virtual void draw() const = 0;
    virtual void update(float delta) = 0;
};

#endif // ACTOR_HPP