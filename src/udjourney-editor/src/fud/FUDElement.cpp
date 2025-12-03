// Copyright 2025 Quentin Cartier
#include "udjourney-editor/fud/FUDElement.hpp"
#include <string>

std::string fud_anchor_to_string(FUDAnchor anchor) {
    switch (anchor) {
        case FUDAnchor::TopLeft:
            return "TopLeft";
        case FUDAnchor::TopCenter:
            return "TopCenter";
        case FUDAnchor::TopRight:
            return "TopRight";
        case FUDAnchor::MiddleLeft:
            return "MiddleLeft";
        case FUDAnchor::MiddleCenter:
            return "MiddleCenter";
        case FUDAnchor::MiddleRight:
            return "MiddleRight";
        case FUDAnchor::BottomLeft:
            return "BottomLeft";
        case FUDAnchor::BottomCenter:
            return "BottomCenter";
        case FUDAnchor::BottomRight:
            return "BottomRight";
        default:
            return "TopLeft";
    }
}

FUDAnchor fud_anchor_from_string(const std::string& str) {
    if (str == "TopCenter") return FUDAnchor::TopCenter;
    if (str == "TopRight") return FUDAnchor::TopRight;
    if (str == "MiddleLeft") return FUDAnchor::MiddleLeft;
    if (str == "MiddleCenter") return FUDAnchor::MiddleCenter;
    if (str == "MiddleRight") return FUDAnchor::MiddleRight;
    if (str == "BottomLeft") return FUDAnchor::BottomLeft;
    if (str == "BottomCenter") return FUDAnchor::BottomCenter;
    if (str == "BottomRight") return FUDAnchor::BottomRight;
    return FUDAnchor::TopLeft;
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

std::string fud_image_render_mode_to_string(FUDImageRenderMode mode) {
    switch (mode) {
        case FUDImageRenderMode::Stretch:
            return "Stretch";
        case FUDImageRenderMode::Tile:
            return "Tile";
        case FUDImageRenderMode::Center:
            return "Center";
        default:
            return "Stretch";
    }
}

FUDImageRenderMode fud_image_render_mode_from_string(const std::string& str) {
    if (str == "Tile") return FUDImageRenderMode::Tile;
    if (str == "Center") return FUDImageRenderMode::Center;
    return FUDImageRenderMode::Stretch;
}

void to_json(nlohmann::json& j, const FUDElement& fud) {
    j = nlohmann::json{{"name", fud.name},
                       {"type_id", fud.type_id},
                       {"anchor", fud_anchor_to_string(fud.anchor)},
                       {"offset", {{"x", fud.offset.x}, {"y", fud.offset.y}}},
                       {"size", {{"x", fud.size.x}, {"y", fud.size.y}}},
                       {"visible", fud.visible},
                       {"properties", fud.properties}};

    // Add sprite sheet fields if set
    if (!fud.background_sheet.empty()) {
        j["background_sheet"] = fud.background_sheet;
        j["background_tile_size"] = fud.background_tile_size;
        j["background_tile_row"] = fud.background_tile_row;
        j["background_tile_col"] = fud.background_tile_col;
        j["background_tile_width"] = fud.background_tile_width;
        j["background_tile_height"] = fud.background_tile_height;
        j["background_render_mode"] =
            fud_image_render_mode_to_string(fud.background_render_mode);
    }
    if (!fud.foreground_sheet.empty()) {
        j["foreground_sheet"] = fud.foreground_sheet;
        j["foreground_tile_size"] = fud.foreground_tile_size;
        j["foreground_tile_row"] = fud.foreground_tile_row;
        j["foreground_tile_col"] = fud.foreground_tile_col;
        j["foreground_tile_width"] = fud.foreground_tile_width;
        j["foreground_tile_height"] = fud.foreground_tile_height;
        j["foreground_render_mode"] =
            fud_image_render_mode_to_string(fud.foreground_render_mode);
    }
    if (fud.image_scale != 1.0f) {
        j["image_scale"] = fud.image_scale;
    }
}

void from_json(const nlohmann::json& j, FUDElement& fud) {
    fud.name = j.at("name").get<std::string>();
    fud.type_id = j.at("type_id").get<std::string>();
    fud.anchor = fud_anchor_from_string(j.at("anchor").get<std::string>());

    // Handle both nested offset object and flat offset_x/offset_y
    if (j.contains("offset") && j["offset"].is_object()) {
        fud.offset.x = j["offset"].at("x").get<float>();
        fud.offset.y = j["offset"].at("y").get<float>();
    } else {
        fud.offset.x = j.value("offset_x", 0.0f);
        fud.offset.y = j.value("offset_y", 0.0f);
    }

    // Handle both nested size object and flat size_x/size_y
    if (j.contains("size") && j["size"].is_object()) {
        fud.size.x = j["size"].at("x").get<float>();
        fud.size.y = j["size"].at("y").get<float>();
    } else {
        fud.size.x = j.value("size_x", 100.0f);
        fud.size.y = j.value("size_y", 30.0f);
    }

    fud.visible = j.at("visible").get<bool>();
    fud.properties = j.at("properties");

    // Load sprite sheet fields if present
    if (j.contains("background_sheet")) {
        fud.background_sheet = j["background_sheet"].get<std::string>();
        fud.background_tile_size = j.value("background_tile_size", 32);
        fud.background_tile_row = j.value("background_tile_row", 0);
        fud.background_tile_col = j.value("background_tile_col", 0);
        fud.background_tile_width = j.value("background_tile_width", 1);
        fud.background_tile_height = j.value("background_tile_height", 1);
        fud.background_render_mode = fud_image_render_mode_from_string(
            j.value("background_render_mode", "Stretch"));
    }
    if (j.contains("foreground_sheet")) {
        fud.foreground_sheet = j["foreground_sheet"].get<std::string>();
        fud.foreground_tile_size = j.value("foreground_tile_size", 32);
        fud.foreground_tile_row = j.value("foreground_tile_row", 0);
        fud.foreground_tile_col = j.value("foreground_tile_col", 0);
        fud.foreground_tile_width = j.value("foreground_tile_width", 1);
        fud.foreground_tile_height = j.value("foreground_tile_height", 1);
        fud.foreground_render_mode = fud_image_render_mode_from_string(
            j.value("foreground_render_mode", "Stretch"));
    }
    fud.image_scale = j.value("image_scale", 1.0f);
}
