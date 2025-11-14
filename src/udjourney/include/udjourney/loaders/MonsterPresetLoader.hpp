// Copyright 2025 Quentin Cartier
#pragma once
#include <string>
#include <memory>

#include "udjourney/MonsterPreset.hpp"

#ifdef PLATFORM_DREAMCAST
#include "udjourney/dreamcast_json_compat.h"
#endif
#include <nlohmann/json.hpp>

namespace udjourney {

class MonsterPresetLoader {
 public:
    static std::unique_ptr<MonsterPreset> load_preset(
        const std::string& preset_file);

 private:
    static MonsterStats parse_stats(const nlohmann::json& json);
    static MonsterBehavior parse_behavior(const nlohmann::json& json);
    static MonsterStateConfig parse_state_config(const nlohmann::json& json);
};

}  // namespace udjourney
