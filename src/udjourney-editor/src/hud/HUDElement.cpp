// Copyright 2025 Quentin Cartier
#include "udjourney-editor/hud/HUDElement.hpp"
#include <string>

std::string fud_anchor_to_string(HUDAnchor anchor) {
    switch (anchor) {
        case HUDAnchor::TopLeft:
            return "TopLeft";
        case HUDAnchor::TopCenter:
            return "TopCenter";
        case HUDAnchor::TopRight:
            return "TopRight";
        case HUDAnchor::MiddleLeft:
            return "MiddleLeft";
        case HUDAnchor::MiddleCenter:
            return "MiddleCenter";
        case HUDAnchor::MiddleRight:
            return "MiddleRight";
        case HUDAnchor::BottomLeft:
            return "BottomLeft";
        case HUDAnchor::BottomCenter:
            return "BottomCenter";
        case HUDAnchor::BottomRight:
            return "BottomRight";
        default:
            return "TopLeft";
    }
}

HUDAnchor fud_anchor_from_string(const std::string& str) {
    if (str == "TopCenter") return HUDAnchor::TopCenter;
    if (str == "TopRight") return HUDAnchor::TopRight;
    if (str == "MiddleLeft") return HUDAnchor::MiddleLeft;
    if (str == "MiddleCenter") return HUDAnchor::MiddleCenter;
    if (str == "MiddleRight") return HUDAnchor::MiddleRight;
    if (str == "BottomLeft") return HUDAnchor::BottomLeft;
    if (str == "BottomCenter") return HUDAnchor::BottomCenter;
    if (str == "BottomRight") return HUDAnchor::BottomRight;
    return HUDAnchor::TopLeft;
}

std::string fud_category_to_string(FUDCategory category) {
    switch (category) {
        case FUDCategory::StatusDisplay:
            return "StatusDisplay";
        case FUDCategory::ScoreCounter:
            return "ScoreCounter";
        case FUDCategory::Timer:
            return "Timer";
        case FUDCategory::Gauge:
            return "Gauge";
        case FUDCategory::Text:
            return "Text";
        case FUDCategory::Custom:
            return "Custom";
        default:
            return "Custom";
    }
}

FUDCategory fud_category_from_string(const std::string& str) {
    if (str == "StatusDisplay") return FUDCategory::StatusDisplay;
    if (str == "ScoreCounter") return FUDCategory::ScoreCounter;
    if (str == "Timer") return FUDCategory::Timer;
    if (str == "Gauge") return FUDCategory::Gauge;
    if (str == "Text") return FUDCategory::Text;
    return FUDCategory::Custom;
}

std::string fud_image_render_mode_to_string(HUDImageRenderMode mode) {
    switch (mode) {
        case HUDImageRenderMode::Stretch:
            return "Stretch";
        case HUDImageRenderMode::Tile:
            return "Tile";
        case HUDImageRenderMode::Center:
            return "Center";
        default:
            return "Stretch";
    }
}

HUDImageRenderMode fud_image_render_mode_from_string(const std::string& str) {
    if (str == "Tile") return HUDImageRenderMode::Tile;
    if (str == "Center") return HUDImageRenderMode::Center;
    return HUDImageRenderMode::Stretch;
}

void to_json(nlohmann::json& j, const HUDElement& hud) {
    j = nlohmann::json{{"name", hud.name},
                       {"type_id", hud.type_id},
                       {"anchor", fud_anchor_to_string(hud.anchor)},
                       {"offset", {{"x", hud.offset.x}, {"y", hud.offset.y}}},
                       {"size", {{"x", hud.size.x}, {"y", hud.size.y}}},
                       {"visible", hud.visible},
                       {"properties", hud.properties}};

    // Add sprite sheet fields if set
    if (!hud.background_sheet.empty()) {
        j["background_sheet"] = hud.background_sheet;
        j["background_tile_size"] = hud.background_tile_size;
        j["background_tile_row"] = hud.background_tile_row;
        j["background_tile_col"] = hud.background_tile_col;
        j["background_tile_width"] = hud.background_tile_width;
        j["background_tile_height"] = hud.background_tile_height;
        j["background_render_mode"] =
            fud_image_render_mode_to_string(hud.background_render_mode);
    }
    if (!hud.foreground_sheet.empty()) {
        j["foreground_sheet"] = hud.foreground_sheet;
        j["foreground_tile_size"] = hud.foreground_tile_size;
        j["foreground_tile_row"] = hud.foreground_tile_row;
        j["foreground_tile_col"] = hud.foreground_tile_col;
        j["foreground_tile_width"] = hud.foreground_tile_width;
        j["foreground_tile_height"] = hud.foreground_tile_height;
        j["foreground_render_mode"] =
            fud_image_render_mode_to_string(hud.foreground_render_mode);
    }
    if (hud.image_scale != 1.0f) {
        j["image_scale"] = hud.image_scale;
    }
}

void from_json(const nlohmann::json& j, HUDElement& hud) {
    hud.name = j.at("name").get<std::string>();
    hud.type_id = j.at("type_id").get<std::string>();
    hud.anchor = fud_anchor_from_string(j.at("anchor").get<std::string>());

    // Handle both nested offset object and flat offset_x/offset_y
    if (j.contains("offset") && j["offset"].is_object()) {
        hud.offset.x = j["offset"].at("x").get<float>();
        hud.offset.y = j["offset"].at("y").get<float>();
    } else {
        hud.offset.x = j.value("offset_x", 0.0f);
        hud.offset.y = j.value("offset_y", 0.0f);
    }

    // Handle both nested size object and flat size_x/size_y
    if (j.contains("size") && j["size"].is_object()) {
        hud.size.x = j["size"].at("x").get<float>();
        hud.size.y = j["size"].at("y").get<float>();
    } else {
        hud.size.x = j.value("size_x", 100.0f);
        hud.size.y = j.value("size_y", 30.0f);
    }

    hud.visible = j.at("visible").get<bool>();
    hud.properties = j.at("properties");

    // Load sprite sheet fields if present
    if (j.contains("background_sheet")) {
        hud.background_sheet = j["background_sheet"].get<std::string>();
        hud.background_tile_size = j.value("background_tile_size", 32);
        hud.background_tile_row = j.value("background_tile_row", 0);
        hud.background_tile_col = j.value("background_tile_col", 0);
        hud.background_tile_width = j.value("background_tile_width", 1);
        hud.background_tile_height = j.value("background_tile_height", 1);
        hud.background_render_mode = fud_image_render_mode_from_string(
            j.value("background_render_mode", "Stretch"));
    }
    if (j.contains("foreground_sheet")) {
        hud.foreground_sheet = j["foreground_sheet"].get<std::string>();
        hud.foreground_tile_size = j.value("foreground_tile_size", 32);
        hud.foreground_tile_row = j.value("foreground_tile_row", 0);
        hud.foreground_tile_col = j.value("foreground_tile_col", 0);
        hud.foreground_tile_width = j.value("foreground_tile_width", 1);
        hud.foreground_tile_height = j.value("foreground_tile_height", 1);
        hud.foreground_render_mode = fud_image_render_mode_from_string(
            j.value("foreground_render_mode", "Stretch"));
    }
    hud.image_scale = j.value("image_scale", 1.0f);
}
