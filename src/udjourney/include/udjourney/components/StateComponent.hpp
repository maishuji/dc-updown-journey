// Copyright 2025 Quentin Cartier
#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include "udjourney/interfaces/IComponent.hpp"

class StateComponent : public IComponent {
 public:
    StateComponent() = default;
    ~StateComponent() override = default;

    void update(float delta) override;

    void on_attach(IActor& actor) override;
    void on_detach(IActor& actor) override;

    void register_state(std::unique_ptr<class IActorState> ioState);
    void pop_state();
    void change_state(const std::string& iNewState);
    [[nodiscard]] inline std::vector<class State*>& get_stack() noexcept {
        return m_state_stack;
    }

 private:
    float elapsed_time_ = 0.0F;

    std::unordered_map<std::string, std::unique_ptr<class IActorState>>
        m_state_map;
    class IActorState* m_cur_state = nullptr;
    std::vector<class IActorState*> m_state_stack;

    std::vector<class IActorState*> m_pending_states;
    int m_remove_state_trigger;
};
