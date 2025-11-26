// Copyright 2025 Quentin Cartier
#include "udjourney-editor/fud/FUDElement.hpp"

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

void to_json(nlohmann::json& j, const FUDElement& fud) {
    j = nlohmann::json{{"name", fud.name},
                       {"type_id", fud.type_id},
                       {"anchor", fud_anchor_to_string(fud.anchor)},
                       {"offset", {{"x", fud.offset.x}, {"y", fud.offset.y}}},
                       {"size", {{"x", fud.size.x}, {"y", fud.size.y}}},
                       {"visible", fud.visible},
                       {"properties", fud.properties}};
}

void from_json(const nlohmann::json& j, FUDElement& fud) {
    fud.name = j.at("name").get<std::string>();
    fud.type_id = j.at("type_id").get<std::string>();
    fud.anchor = fud_anchor_from_string(j.at("anchor").get<std::string>());
    fud.offset.x = j.at("offset").at("x").get<float>();
    fud.offset.y = j.at("offset").at("y").get<float>();
    fud.size.x = j.at("size").at("x").get<float>();
    fud.size.y = j.at("size").at("y").get<float>();
    fud.visible = j.at("visible").get<bool>();
    fud.properties = j.at("properties");
}
