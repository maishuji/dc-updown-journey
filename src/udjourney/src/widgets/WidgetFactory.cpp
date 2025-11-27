// Copyright 2025 Quentin Cartier
#include "udjourney/widgets/WidgetFactory.hpp"
#include "udjourney/widgets/ButtonWidget.hpp"
#include <udj-core/Logger.hpp>

std::unique_ptr<IWidget> WidgetFactory::create_from_fud(
    const udjourney::scene::FUDData& fud,
    const IGame& game) {
    
    if (fud.type_id == "menu_button") {
        udjourney::Logger::debug("Creating ButtonWidget from FUD: %", fud.name);
        return std::make_unique<ButtonWidget>(game, fud);
    }
    
    // Add more widget types here as needed:
    // if (fud.type_id == "slider") {
    //     return std::make_unique<SliderWidget>(game, fud);
    // }
    // if (fud.type_id == "checkbox") {
    //     return std::make_unique<CheckboxWidget>(game, fud);
    // }
    
    udjourney::Logger::warning("Unknown widget type: %", fud.type_id);
    return nullptr;
}
