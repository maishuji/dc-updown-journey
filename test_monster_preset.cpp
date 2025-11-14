// Test script to validate monster preset system without graphics
#include <iostream>
#include <memory>
#include "udjourney/loaders/MonsterPresetLoader.hpp"

int main() {
    try {
        std::cout << "=== Testing Monster Preset System ===" << std::endl;
        
        // Test loading goblin preset
        std::string preset_path = "src/udjourney/romdisk/monsters/goblin.json";
        std::cout << "Loading preset from: " << preset_path << std::endl;
        
        auto preset = udjourney::MonsterPresetLoader::load_preset(preset_path);
        
        std::cout << "SUCCESS: Preset loaded!" << std::endl;
        std::cout << "Name: " << preset->name << std::endl;
        std::cout << "Display Name: " << preset->display_name << std::endl;
        std::cout << "Max Health: " << preset->stats.max_health << std::endl;
        std::cout << "Movement Speed: " << preset->stats.movement_speed << std::endl;
        std::cout << "Initial State: " << preset->state_config.initial_state << std::endl;
        std::cout << "Available States: ";
        for (const auto& state : preset->state_config.available_states) {
            std::cout << state << " ";
        }
        std::cout << std::endl;
        std::cout << "Number of Transitions: " << preset->state_config.transitions.size() << std::endl;
        
        std::cout << "=== Monster Preset System Test PASSED ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        std::cout << "=== Monster Preset System Test FAILED ===" << std::endl;
        return 1;
    }
}
