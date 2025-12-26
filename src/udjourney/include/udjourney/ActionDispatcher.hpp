// Copyright 2025 Quentin Cartier
#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace udjourney {

class IGame;

/**
 * @brief Handles action strings from UI elements and executes callbacks
 *
 * The ActionDispatcher provides a centralized way to handle menu actions
 * and scene transitions. Actions are registered with string names and
 * can be triggered from JSON-based UI elements.
 *
 * Example usage:
 * @code
 * ActionDispatcher::register_action("start_game", [](IGame* game, const auto&
 * params) { game->load_scene("level1.json");
 * });
 *
 * // Later, from a button click:
 * ActionDispatcher::execute("start_game", game);
 * @endcode
 */
class ActionDispatcher {
 public:
    using ActionCallback =
        std::function<void(udjourney::IGame*, const std::vector<std::string>&)>;

    /**
     * @brief Register an action handler
     * @param action_name Name of the action (e.g., "start_game")
     * @param callback Function to execute when action is triggered
     */
    static void register_action(const std::string& action_name,
                                ActionCallback callback);

    /**
     * @brief Execute an action
     * @param action_string Full action string (e.g., "load_level:level2")
     * @param game Pointer to game instance
     */
    static void execute(const std::string& action_string,
                        udjourney::IGame* game);

    /**
     * @brief Clear all registered actions
     */
    static void clear_actions();

 private:
    static std::unordered_map<std::string, ActionCallback> actions_;

    /**
     * @brief Split action string into name and parameters
     * @param action_string String like "action:param1:param2"
     * @return Vector of parts [action, param1, param2]
     */
    static std::vector<std::string> split_action(
        const std::string& action_string);
};
}  // namespace udjourney
