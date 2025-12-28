// Copyright 2025 Quentin Cartier
#pragma once

namespace udjourney {

class Game;

/**
 * @brief Interface for state-specific rendering logic.
 *
 * Each GameState (TITLE, PLAY, PAUSE, etc.) can have its own renderer
 * that implements this interface, encapsulating the drawing logic for that
 * state.
 */
class IStateRenderer {
 public:
    virtual ~IStateRenderer() = default;

    /**
     * @brief Render the game world for this state.
     *
     * Called between PushMatrix/PopMatrix with the viewport transform already
     * applied.
     *
     * @param game Reference to the game object for accessing state and actors.
     */
    virtual void render(const Game& game) const = 0;
};

}  // namespace udjourney
