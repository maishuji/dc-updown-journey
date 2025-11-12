// Copyright 2025 Quentin Cartier

#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_IACTOR_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_IACTOR_HPP_

#include <concepts>
#include <memory>
#include <utility>
#include <vector>

#include "udjourney/interfaces/IComponent.hpp"

// Forward declarations
class IGame;

enum class ActorState {
    ONGOING,
    CONSUMED  // Need to be removed from the game
};

class IActor {
 public:
    explicit IActor(const IGame &game) : m_game(&game) {}
    virtual ~IActor() = default;
    virtual void draw() const = 0;
    virtual void update(float delta) = 0;
    virtual void process_input() = 0;
    virtual void set_rectangle(struct Rectangle iRect) = 0;
    [[nodiscard]] virtual struct Rectangle get_rectangle() const = 0;
    [[nodiscard]] virtual bool check_collision(const IActor &other) const = 0;
    [[nodiscard]] virtual constexpr uint8_t get_group_id() const = 0;
    [[nodiscard]] virtual const IGame &get_game() const { return *m_game; }

    void set_state(ActorState iState) noexcept { state = iState; }
    [[nodiscard]] ActorState get_state() const noexcept { return state; }

    void add_component(std::unique_ptr<IComponent> component) {
        m_components.push_back(std::move(component));
    }
    const std::vector<std::unique_ptr<IComponent>> &get_components() const {
        return m_components;
    }

 private:
    const IGame *m_game = nullptr;
    ActorState state = ActorState::ONGOING;
    std::vector<std::unique_ptr<IComponent>> m_components;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_IACTOR_HPP_
