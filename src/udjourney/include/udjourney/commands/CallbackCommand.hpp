#pragma once

#include <udj-core/ICommand.hpp>
#include <functional>

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
