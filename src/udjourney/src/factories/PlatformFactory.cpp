// Copyright 2025 Quentin Cartier
#include "udjourney/factories/PlatformFactory.hpp"

#include "udjourney/platform/behavior_strategies/EightTurnHorizontalBehaviorStrategy.hpp"
#include "udjourney/platform/behavior_strategies/HorizontalBehaviorStrategy.hpp"
#include "udjourney/platform/behavior_strategies/OscillatingSizeBehaviorStrategy.hpp"
#include "udjourney/platform/features/CheckpointFeature.hpp"
#include "udjourney/platform/features/SpikeFeature.hpp"

namespace udjourney {

std::unique_ptr<Platform> PlatformFactory::create(
    const IGame& game, const Rectangle& world_rect,
    const udjourney::scene::PlatformData& platform_data) {
    Color platform_color = BLUE;

    switch (platform_data.behavior_type) {
        case udjourney::scene::PlatformBehaviorType::Horizontal:
            platform_color = GREEN;
            break;
        case udjourney::scene::PlatformBehaviorType::EightTurnHorizontal:
            platform_color = ORANGE;
            break;
        case udjourney::scene::PlatformBehaviorType::OscillatingSize:
            platform_color = PURPLE;
            break;
        case udjourney::scene::PlatformBehaviorType::Static:
        default:
            platform_color = BLUE;
            break;
    }

    // Add spikes color indication
    bool has_spikes = false;
    for (auto feature : platform_data.features) {
        if (feature == udjourney::scene::PlatformFeatureType::Spikes) {
            platform_color = RED;
            has_spikes = true;
            break;
        }
    }

    // Scene-based platforms should not be reused to avoid cluttering the
    // screen Default constructor uses nullptr reuse strategy (no reuse)
    auto platform = std::make_unique<Platform>(game,
                                               world_rect,
                                               platform_color,
                                               false);  // not Y-repeated

    // Set behavior based on platform data
    switch (platform_data.behavior_type) {
        case udjourney::scene::PlatformBehaviorType::Horizontal: {
            float speed = 50.0f;          // Default speed
            float range = 100.0f;         // Default range
            float initial_offset = 0.0f;  // Default starting offset

            if (platform_data.behavior_params.count("speed")) {
                speed = platform_data.behavior_params.at("speed") *
                        10.0f;  // Scale for pixels
            }
            if (platform_data.behavior_params.count("range")) {
                range = platform_data.behavior_params.at("range") *
                        32.0f;  // Convert tiles to pixels
            }
            if (platform_data.behavior_params.count("initial_offset")) {
                initial_offset =
                    platform_data.behavior_params.at("initial_offset") *
                    32.0f;  // Convert tiles to pixels
            }

            platform->set_behavior(std::make_unique<HorizontalBehaviorStrategy>(
                speed, range, initial_offset));
            break;
        }
        case udjourney::scene::PlatformBehaviorType::EightTurnHorizontal: {
            float speed = 1.0f;        // Default speed
            float amplitude = 100.0f;  // Default amplitude

            if (platform_data.behavior_params.count("speed")) {
                speed = platform_data.behavior_params.at("speed");
            }
            if (platform_data.behavior_params.count("amplitude")) {
                amplitude = platform_data.behavior_params.at("amplitude") *
                            32.0f;  // Convert tiles to pixels
            }

            platform->set_behavior(
                std::make_unique<EightTurnHorizontalBehaviorStrategy>(
                    speed, amplitude));
            break;
        }
        case udjourney::scene::PlatformBehaviorType::OscillatingSize: {
            float speed = 2.0f;        // Default speed
            float min_scale = -50.0f;  // Default min scale
            float max_scale = 50.0f;   // Default max scale

            if (platform_data.behavior_params.count("speed")) {
                speed = platform_data.behavior_params.at("speed");
            }
            if (platform_data.behavior_params.count("min_scale")) {
                min_scale =
                    (platform_data.behavior_params.at("min_scale") - 1.0f) *
                    world_rect.width;
            }
            if (platform_data.behavior_params.count("max_scale")) {
                max_scale =
                    (platform_data.behavior_params.at("max_scale") - 1.0f) *
                    world_rect.width;
            }

            platform->set_behavior(
                std::make_unique<OscillatingSizeBehaviorStrategy>(
                    speed, min_scale, max_scale));
            break;
        }
        case udjourney::scene::PlatformBehaviorType::Static:
        default:
            // No behavior needed for static platforms
            break;
    }

    // Add features
    udjourney::Logger::info("Processing platform at (%, %) with % features",
                            platform_data.tile_x,
                            platform_data.tile_y,
                            platform_data.features.size());
    for (auto feature : platform_data.features) {
        if (feature == udjourney::scene::PlatformFeatureType::Spikes) {
            platform->add_feature(std::make_unique<SpikeFeature>());
            udjourney::Logger::info(
                "Added spikes feature to platform at (%, %)",
                platform_data.tile_x,
                platform_data.tile_y);
        } else if (feature ==
                   udjourney::scene::PlatformFeatureType::Checkpoint) {
            udjourney::Logger::info(
                "Creating checkpoint platform at tile_x=%, tile_y=%",
                platform_data.tile_x,
                platform_data.tile_y);
            platform->add_feature(std::make_unique<CheckpointFeature>());
            // Set checkpoint platform color to distinguish it
            platform_color = ORANGE;
        }
    }
    return platform;
}

}  // namespace udjourney
