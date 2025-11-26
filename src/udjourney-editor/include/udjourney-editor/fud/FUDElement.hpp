// Copyright 2025 Quentin Cartier
#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <imgui.h>

// FUD anchor position on screen
enum class FUDAnchor {
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    MiddleCenter,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

// Convert FUDAnchor to/from string for JSON serialization
std::string fud_anchor_to_string(FUDAnchor anchor);
FUDAnchor fud_anchor_from_string(const std::string& str);

// FUD category for editor organization
enum class FUDCategory {
    StatusDisplay,  // Health, mana, stamina
    ScoreCounter,   // Score, coins, items
    Timer,          // Countdown, elapsed time
    Gauge,          // Any bar/meter display
    Text,           // Simple text display
    Custom          // User-defined
};

std::string fud_category_to_string(FUDCategory category);
FUDCategory fud_category_from_string(const std::string& str);

// FUD preset loaded from JSON
struct FUDPreset {
    std::string type_id;       // e.g., "healthbar", "score_display"
    std::string display_name;  // Human-readable name
    FUDCategory category;
    ImVec2 default_size;               // Default size in pixels
    FUDAnchor default_anchor;          // Default anchor position
    nlohmann::json properties_schema;  // Property definitions
    std::string preview_image;         // Path to preview image
    std::string description;           // Optional description
};

// FUD element placed in a level
struct FUDElement {
    std::string name;     // Instance name (e.g., "Player Health")
    std::string type_id;  // Reference to preset type
    FUDAnchor anchor;
    ImVec2 offset;  // Offset from anchor in pixels
    ImVec2 size;    // Size in pixels
    bool visible;
    nlohmann::json properties;  // Instance-specific properties

    FUDElement() :
        anchor(FUDAnchor::TopLeft),
        offset(0, 0),
        size(100, 30),
        visible(true) {}

    FUDElement(const std::string& name, const std::string& type_id,
               FUDAnchor anchor, ImVec2 offset, ImVec2 size) :
        name(name),
        type_id(type_id),
        anchor(anchor),
        offset(offset),
        size(size),
        visible(true) {}
};

// JSON serialization
void to_json(nlohmann::json& j, const FUDElement& fud);
void from_json(const nlohmann::json& j, FUDElement& fud);
