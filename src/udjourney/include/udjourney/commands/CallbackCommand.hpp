// Copyright 2025 Quentin Cartier
#pragma once

#include <functional>
#include <utility>

#include <udj-core/ICommand.hpp>

namespace udjourney::commands {

class CallbackCommand : public udj::core::ICommand {
 public:
    using Callback = std::function<void()>;

    explicit CallbackCommand(Callback callback) :
        m_callback(std::move(callback)) {}

    void execute() override { m_callback(); }

 private:
    Callback m_callback;
};

}  // namespace udjourney::commands
