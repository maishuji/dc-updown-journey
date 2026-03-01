// Copyright 2025 Quentin Cartier
#pragma once

#include "udjourney/render/IStateRenderer.hpp"

namespace udjourney {

/**
 * @brief Renderer for UI screens (TITLE, WIN, GAMEOVER).
 *
 * Draws backgrounds + scene HUDs + widgets.
 */
class UiScreenRenderer : public IStateRenderer {
 public:
    void render(const Game& game) const override;
};

/**
 * @brief Renderer for active gameplay (PLAY state).
 *
 * Draws backgrounds + actors + player + finish line + dash HUD.
 */
class PlayStateRenderer : public IStateRenderer {
 public:
    void render(const Game& game) const override;
};

/**
 * @brief Renderer for pause overlay (PAUSE state).
 *
 * Draws gameplay scene + pause overlay text.
 */
class PauseStateRenderer : public IStateRenderer {
 public:
    void render(const Game& game) const override;
};

}  // namespace udjourney
