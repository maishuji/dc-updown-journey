// Copyright 2025 Quentin Cartier
#pragma once
#include <memory>
#include "udjourney/widgets/IWidget.hpp"
#include "udjourney/scene/Scene.hpp"
namespace udjourney {
class IGame;

/**
 * @brief Factory for creating widgets from FUD data
 *
 * Converts FUD elements into concrete widget instances based on their type_id.
 * Supports extensibility for adding new widget types.
 */
class WidgetFactory {
 public:
    /**
     * @brief Create a widget from FUD element
     * @param fud FUD element containing widget properties
     * @param game Reference to game instance
     * @return Unique pointer to created widget, or nullptr if type not
     * recognized
     */
    static std::unique_ptr<IWidget> create_from_fud(
        const udjourney::scene::FUDData& fud, const IGame& game);
};
}  // namespace udjourney
