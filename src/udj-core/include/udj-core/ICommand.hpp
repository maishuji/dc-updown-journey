// Copyright 2025 Quentin Cartier
#pragma once

namespace udj::core {
class ICommand {
 public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
};

}  // namespace udj::core
