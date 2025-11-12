// Copyright 2025 Quentin Cartier
#include "udjourney/components/StateComponent.hpp"

#include "udjourney/interfaces/IActorState.hpp"

void StateComponent::update(float delta) {
    elapsed_time_ += delta;
    // Additional state update logic can be added here

    // Apply removing
    while (m_remove_state_trigger > 0) {
        assert(!m_state_stack.empty());

        m_state_stack.at(m_state_stack.size() - 1)->on_exit();
        m_state_stack.pop_back();
        m_state_stack.back()->on_resume();
        m_remove_state_trigger -= 1;
    }

    // Add pending states
    for (auto& pending : m_pending_states) {
        Logger::log(LogLevel::debug,
                    "Pushing pending state state for % ",
                    pending->get_name());
        m_state_stack.push_back(pending);
        m_state_stack.back()->on_enter();
    }
    while (!m_params.empty()) {
        m_params.pop();
    }
    m_pending_states.clear();
    m_cur_state = m_state_stack.back();

    if (m_cur_state != nullptr && !m_state_stack.empty()) {
        m_cur_state->update(iDelta);
    }
}

void StateComponent::register_state(std::unique_ptr<IActorState> ioState) {
    m_state_map.emplace(ioState->get_name(), std::move(ioState));
}

void StateComponent::change_state(const std::string& iNewState) {
    auto it = m_state_map.find(iNewState);
    Logger::log(LogLevel::debug, "changing state for % ", iNewState);
    if (it != m_state_map.end()) {
        if (m_cur_state != nullptr) {
            m_pending_states.push_back(it->second.get());
        } else {
            m_state_stack.push_back(it->second.get());
            m_state_stack.back()->on_enter();
        }
    }
}

void StateComponent::on_attach(IActor& actor) {
    // Logic to execute when the component is attached to an actor
    elapsed_time_ = 0.0F;  // Reset elapsed time on attach
}
void StateComponent::on_detach(IActor& actor) {
    // Logic to execute when the component is detached from an actor
}
