// Copyright 2025 Quentin Cartier
#include "udjourney/widgets/WidgetFactory.hpp"

#include <memory>

#include "udjourney/widgets/ButtonWidget.hpp"
#include "udjourney/widgets/ScrollableListWidget.hpp"
#include <udj-core/Logger.hpp>
namespace udjourney {
std::unique_ptr<IWidget> WidgetFactory::create_from_fud(
    const udjourney::scene::HUDData& hud, const IGame& game) {
    // All button types use ButtonWidget
    if (hud.type_id == "menu_button" || hud.type_id == "icon_button" ||
        hud.type_id == "small_button" || hud.type_id == "large_button" ||
        hud.type_id == "textured_button") {
        udjourney::Logger::debug("Creating ButtonWidget from FUD: %", hud.name);
        return std::make_unique<ButtonWidget>(game, hud);
    }

    // Scrollable list widget for levels, settings, etc.
    if (hud.type_id == "scrollable_list") {
        udjourney::Logger::debug("Creating ScrollableListWidget from FUD: %",
                                 hud.name);
        return std::make_unique<ScrollableListWidget>(game, hud);
    }

    // Add more widget types here as needed:
    // if (hud.type_id == "slider") {
    //     return std::make_unique<SliderWidget>(game, hud);
    // }
    // if (hud.type_id == "checkbox") {
    //     return std::make_unique<CheckboxWidget>(game, hud);
    // }

    udjourney::Logger::warning("Unknown widget type: %", hud.type_id);
    return nullptr;
}
}  // namespace udjourney
