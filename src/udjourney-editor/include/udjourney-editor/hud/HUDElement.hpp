// Copyright 2025 Quentin Cartier
#pragma once

#include <imgui.h>
#include <string>
#include <nlohmann/json.hpp>

// HUD anchor position on screen
enum class HUDAnchor {
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

// Convert HUDAnchor to/from string for JSON serialization
std::string fud_anchor_to_string(HUDAnchor anchor);
HUDAnchor fud_anchor_from_string(const std::string& str);

// HUD category for editor organization
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

// Image render mode for background/foreground
enum class HUDImageRenderMode {
    Stretch,  // Scale to fit HUD size
    Tile,     // Repeat/tile the image
    Center    // Center without scaling
};

std::string fud_image_render_mode_to_string(HUDImageRenderMode mode);
HUDImageRenderMode fud_image_render_mode_from_string(const std::string& str);

// HUD preset loaded from JSON
struct FUDPreset {
    std::string type_id;       // e.g., "healthbar", "score_display"
    std::string display_name;  // Human-readable name
    FUDCategory category;
    ImVec2 default_size;               // Default size in pixels
    HUDAnchor default_anchor;          // Default anchor position
    nlohmann::json properties_schema;  // Property definitions
    std::string preview_image;         // Path to preview image
    std::string description;           // Optional description
};

// HUD element placed in a level
struct HUDElement {
    std::string name;     // Instance name (e.g., "Player Health")
    std::string type_id;  // Reference to preset type
    HUDAnchor anchor;
    ImVec2 offset;  // Offset from anchor in pixels
    ImVec2 size;    // Size in pixels
    bool visible;
    nlohmann::json properties;  // Instance-specific properties

    // Background sprite configuration
    std::string background_sheet;
    int background_tile_size = 32;
    int background_tile_row = 0;
    int background_tile_col = 0;
    int background_tile_width = 1;
    int background_tile_height = 1;
    HUDImageRenderMode background_render_mode = HUDImageRenderMode::Stretch;

    // Foreground sprite configuration
    std::string foreground_sheet;
    int foreground_tile_size = 32;
    int foreground_tile_row = 0;
    int foreground_tile_col = 0;
    int foreground_tile_width = 1;
    int foreground_tile_height = 1;
    HUDImageRenderMode foreground_render_mode = HUDImageRenderMode::Stretch;

    float image_scale = 1.0f;

    HUDElement() :
        anchor(HUDAnchor::TopLeft),
        offset(0, 0),
        size(100, 30),
        visible(true) {}

    HUDElement(const std::string& name, const std::string& type_id,
               HUDAnchor anchor, ImVec2 offset, ImVec2 size) :
        name(name),
        type_id(type_id),
        anchor(anchor),
        offset(offset),
        size(size),
        visible(true) {}
};

// JSON serialization
void to_json(nlohmann::json& j, const HUDElement& hud);
void from_json(const nlohmann::json& j, HUDElement& hud);
