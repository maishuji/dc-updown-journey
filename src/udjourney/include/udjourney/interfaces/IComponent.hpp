#pragma once

class IGame;
class IActor;

class IComponent {
 public:
    virtual ~IComponent() = default;
    virtual void update(float delta) = 0;
    virtual void on_attach(IActor& actor) = 0;
    virtual void on_detach(IActor& actor) = 0;
};
