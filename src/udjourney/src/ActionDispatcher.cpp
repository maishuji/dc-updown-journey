// Copyright 2025 Quentin Cartier
#include "udjourney/ActionDispatcher.hpp"
#include <sstream>
#include <iostream>
#include <udj-core/Logger.hpp>

std::unordered_map<std::string, ActionDispatcher::ActionCallback>
    ActionDispatcher::actions_;

void ActionDispatcher::register_action(const std::string& action_name,
                                       ActionCallback callback) {
    actions_[action_name] = callback;
    udjourney::Logger::debug("Registered action: %", action_name);
}

void ActionDispatcher::execute(const std::string& action_string, IGame* game) {
    if (action_string.empty()) {
        udjourney::Logger::error("Empty action string!");
        return;
    }

    auto parts = split_action(action_string);
    std::string action_name = parts[0];
    std::vector<std::string> params(parts.begin() + 1, parts.end());

    auto it = actions_.find(action_name);
    if (it != actions_.end()) {
        udjourney::Logger::info("Executing action: %", action_name);
        it->second(game, params);
    } else {
        udjourney::Logger::error("Unknown action: %", action_name);
    }
}

void ActionDispatcher::clear_actions() { actions_.clear(); }

std::vector<std::string> ActionDispatcher::split_action(
    const std::string& action_string) {
    std::vector<std::string> parts;
    std::stringstream ss(action_string);
    std::string part;

    while (std::getline(ss, part, ':')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }

    return parts;
}
