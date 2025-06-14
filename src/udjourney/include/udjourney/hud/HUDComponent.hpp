// Copyright 2025 Quentin Cartier
#pragma once
#include <string>

class HUDComponent {
 public:
    [[nodiscard]] virtual std::string get_type() const = 0;
    virtual void update(float deltaTime) = 0;
    [[nodiscard]] inline bool is_focusable() const noexcept {
        return m_is_focusable;
    }
    virtual void draw() const = 0;
    virtual void handle_input() {}
    virtual ~HUDComponent() = default;

 protected:
    bool m_is_focusable = false;
};
