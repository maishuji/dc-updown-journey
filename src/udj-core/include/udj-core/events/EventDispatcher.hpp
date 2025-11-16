// Copyright 2025 Quentin Cartier
#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "udj-core/events/IEvent.hpp"

namespace udjourney::core::events {

class EventDispatcher {
 public:
    using EventHandler = std::function<void(const IEvent&)>;

    /**
     * @brief Registers a new handler for a given event type.
     *
     * Adds the provided callable \p handler to the internal list of subscribers
     * for events identified by \p event_type. When an event of this type is
     * dispatched, all registered handlers will be invoked in the order they
     * were added.
     *
     * @param event_type  The string identifier of the event to subscribe to.
     * @param handler     A callable conforming to `void(const IEvent&)` that
     *                    will be invoked whenever an event of type
     *                    \p event_type is dispatched.
     */
    void register_handler(const std::string& event_type, EventHandler handler) {
        handlers[event_type].push_back(std::move(handler));
    }

    /**
     * @brief Dispatches an event to all handlers registered for its type.
     *
     * Looks up the handlers associated with the event’s runtime type (as
     * returned by `event.type()`) and invokes each handler in registration
     * order, passing the event as a constant reference. Handlers that have been
     * registered via `register_handler` for this event type will all be called.
     *
     * @param event  The event instance to deliver; must derive from IEvent and
     *               implement the `type()` method to identify its event
     * category.
     */
    void dispatch(const IEvent& event) const {
        auto iter = handlers.find(event.type());
        if (iter != handlers.end()) {
            for (const auto& handler : iter->second) {
                handler(event);
            }
        }
    }

 private:
    std::unordered_map<std::string, std::vector<EventHandler>> handlers;
};

}  // namespace udjourney::core::events
