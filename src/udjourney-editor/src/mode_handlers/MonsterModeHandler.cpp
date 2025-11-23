// Copyright 2025 Quentin Cartier
#include "udjourney-editor/mode_handlers/MonsterModeHandler.hpp"

#include <imgui.h>
#include <raylib/raylib.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <functional>
#include <unordered_map>

// Texture cache for monster preview thumbnails
static std::unordered_map<std::string, Texture2D> monster_texture_cache;

// Cache for sprite info to avoid reloading JSON files
struct MonsterSpriteInfo {
    std::string sprite_filename;
    int sprite_width = 64;
    int sprite_height = 64;
    int idle_row = 0;
    int idle_col = 0;
    bool is_valid = false;
};
static std::unordered_map<std::string, MonsterSpriteInfo>
    monster_sprite_info_cache;

static Texture2D load_monster_texture_cached(const std::string& sprite_file) {
    auto it = monster_texture_cache.find(sprite_file);
    if (it != monster_texture_cache.end()) {
        return it->second;
    }

    std::string full_path = "assets/" + sprite_file;
    Texture2D texture = LoadTexture(full_path.c_str());
    if (texture.id != 0) {
        monster_texture_cache[sprite_file] = texture;
    }
    return texture;
}

static MonsterSpriteInfo load_monster_sprite_info(
    const std::string& preset_name) {
    // Check cache first
    auto cache_it = monster_sprite_info_cache.find(preset_name);
    if (cache_it != monster_sprite_info_cache.end()) {
        return cache_it->second;
    }

    MonsterSpriteInfo info;

    try {
        std::string monster_preset_path =
            "assets/monsters/" + preset_name + ".json";

        if (!std::filesystem::exists(monster_preset_path)) {
            return info;
        }

        std::ifstream file(monster_preset_path);
        if (!file.is_open()) {
            return info;
        }

        nlohmann::json preset_json;
        file >> preset_json;

        if (!preset_json.contains("animation_preset")) {
            return info;
        }

        std::string anim_config_file =
            preset_json["animation_preset"].get<std::string>();
        std::string anim_config_path = "assets/animations/" + anim_config_file;

        if (!std::filesystem::exists(anim_config_path)) {
            return info;
        }

        std::ifstream anim_file(anim_config_path);
        if (!anim_file.is_open()) {
            return info;
        }

        nlohmann::json anim_json;
        anim_file >> anim_json;

        if (anim_json.contains("animations")) {
            for (const auto& anim : anim_json["animations"]) {
                if (anim.contains("state_id") &&
                    anim["state_id"].get<int>() == 10) {  // ANIM_IDLE
                    if (anim.contains("sprite_config")) {
                        const auto& sprite_config = anim["sprite_config"];

                        if (sprite_config.contains("filename")) {
                            info.sprite_filename =
                                sprite_config["filename"].get<std::string>();
                        }
                        if (sprite_config.contains("sprite_width")) {
                            info.sprite_width =
                                sprite_config["sprite_width"].get<int>();
                        }
                        if (sprite_config.contains("sprite_height")) {
                            info.sprite_height =
                                sprite_config["sprite_height"].get<int>();
                        }

                        if (sprite_config.contains("frames") &&
                            sprite_config["frames"].is_array()) {
                            const auto& frames = sprite_config["frames"];
                            if (!frames.empty()) {
                                const auto& first_frame = frames[0];
                                if (first_frame.contains("row")) {
                                    info.idle_row =
                                        first_frame["row"].get<int>();
                                }
                                if (first_frame.contains("col")) {
                                    info.idle_col =
                                        first_frame["col"].get<int>();
                                }
                            }
                        }

                        info.is_valid = !info.sprite_filename.empty();
                    }
                    break;
                }
            }
        }
    } catch (const std::exception&) {
        // Return invalid info on any error
        info.is_valid = false;
    }

    // Cache the result
    monster_sprite_info_cache[preset_name] = info;
    return info;
}

MonsterModeHandler::MonsterModeHandler() {
    // Load presets on construction
    monster_preset_manager_.load_available_presets();
}

void MonsterModeHandler::render() {
    // Ensure we have valid monster presets loaded and selected
    initialize_presets();

    ImGui::Text("Monster Spawns");
    ImGui::Separator();

    // Check if we have any monster presets loaded
    if (!monster_preset_manager_.has_presets()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                           "No monster presets found!");
        ImGui::Text("Make sure assets/monsters/ contains");
        ImGui::Text("valid monster preset JSON files.");

        if (ImGui::Button("Reload Presets")) {
            monster_preset_manager_.load_available_presets();
        }
        return;
    }

    // Dynamic monster preset selection
    ImGui::Text("Select Monster Type:");

    const auto& presets = monster_preset_manager_.get_presets();

    // Display presets as a grid of 64x64 preview tiles
    const float tile_size = 64.0f;
    const float padding = 4.0f;
    const float available_width = ImGui::GetContentRegionAvail().x;
    const int tiles_per_row =
        static_cast<int>((available_width + padding) / (tile_size + padding));

    if (tiles_per_row > 0) {
        for (size_t i = 0; i < presets.size(); ++i) {
            const auto& preset = presets[i];
            ImGui::PushID(static_cast<int>(i + 2000));

            bool is_selected = (selected_monster_preset_ == preset.name);

            // Load sprite info and texture
            MonsterSpriteInfo sprite_info =
                load_monster_sprite_info(preset.name);

            if (sprite_info.is_valid) {
                Texture2D texture =
                    load_monster_texture_cached(sprite_info.sprite_filename);

                if (texture.id != 0) {
                    // Calculate UV coordinates for the idle frame
                    float u0 =
                        (sprite_info.idle_col * sprite_info.sprite_width) /
                        static_cast<float>(texture.width);
                    float v0 =
                        (sprite_info.idle_row * sprite_info.sprite_height) /
                        static_cast<float>(texture.height);
                    float u1 = u0 + (sprite_info.sprite_width /
                                     static_cast<float>(texture.width));
                    float v1 = v0 + (sprite_info.sprite_height /
                                     static_cast<float>(texture.height));

                    // Highlight selected preset
                    ImVec4 tint_color = is_selected
                                            ? ImVec4(0.5f, 1.0f, 0.5f, 1.0f)
                                            : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

                    ImVec4 border_color = is_selected
                                              ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f)
                                              : ImVec4(0.4f, 0.4f, 0.4f, 0.5f);

                    // Draw tile as ImageButton
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                          ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                          ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_Border, border_color);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);

                    char button_id[64];
                    std::snprintf(
                        button_id, sizeof(button_id), "##monster_%zu", i);

                    if (ImGui::ImageButton(button_id,
                                           static_cast<ImTextureID>(texture.id),
                                           ImVec2(tile_size, tile_size),
                                           ImVec2(u0, v0),
                                           ImVec2(u1, v1),
                                           ImVec4(0, 0, 0, 0),  // background
                                           tint_color)) {
                        selected_monster_preset_ = preset.name;
                    }

                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(4);

                    // Show preset information on hover
                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::Text("%s", preset.display_name.c_str());
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                                           "Health: %d",
                                           preset.health);
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                                           "Speed: %d",
                                           preset.speed);
                        if (!preset.description.empty()) {
                            ImGui::Separator();
                            ImGui::TextWrapped("%s",
                                               preset.description.c_str());
                        }
                        ImGui::EndTooltip();
                    }
                } else {
                    // Fallback to text button if texture fails to load
                    if (ImGui::Button(preset.display_name.c_str(),
                                      ImVec2(tile_size, tile_size))) {
                        selected_monster_preset_ = preset.name;
                    }

                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::Text("%s", preset.display_name.c_str());
                        ImGui::Text("Health: %d", preset.health);
                        ImGui::Text("Speed: %d", preset.speed);
                        if (!preset.description.empty()) {
                            ImGui::Separator();
                            ImGui::TextWrapped("%s",
                                               preset.description.c_str());
                        }
                        ImGui::EndTooltip();
                    }
                }
            } else {
                // Fallback to text button if sprite info invalid
                if (ImGui::Button(preset.display_name.c_str(),
                                  ImVec2(tile_size, tile_size))) {
                    selected_monster_preset_ = preset.name;
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s", preset.display_name.c_str());
                    ImGui::Text("Health: %d", preset.health);
                    ImGui::Text("Speed: %d", preset.speed);
                    if (!preset.description.empty()) {
                        ImGui::Separator();
                        ImGui::TextWrapped("%s", preset.description.c_str());
                    }
                    ImGui::EndTooltip();
                }
            }

            // Layout grid: add separator or new line
            if ((i + 1) % tiles_per_row != 0 && i < presets.size() - 1) {
                ImGui::SameLine(0.0f, padding);
            }

            ImGui::PopID();
        }
    }

    ImGui::Separator();
    ImGui::Text("Left click: Place %s", selected_monster_preset_.c_str());
    ImGui::Text("Right click: Remove monster");
    ImGui::Text("Click existing monster to edit");

    // Show reload button for development
    if (ImGui::Button("Reload Presets")) {
        monster_preset_manager_.load_available_presets();
        // Validate current selection still exists
        if (monster_preset_manager_.get_preset(selected_monster_preset_) ==
            nullptr) {
            auto preset_names = monster_preset_manager_.get_preset_names();
            if (!preset_names.empty()) {
                selected_monster_preset_ = preset_names[0];
            }
        }
    }

    // Monster editor for selected monster
    if (selected_monster_) {
        render_monster_editor();
    }
}

void MonsterModeHandler::initialize_presets() {
    // Ensure we have a valid selected preset
    if (monster_preset_manager_.has_presets()) {
        auto preset_names = monster_preset_manager_.get_preset_names();
        if (!preset_names.empty()) {
            // Check if current selection is valid
            bool found = false;
            for (const auto& name : preset_names) {
                if (name == selected_monster_preset_) {
                    found = true;
                    break;
                }
            }
            // If current selection is invalid, pick the first available preset
            if (!found) {
                selected_monster_preset_ = preset_names[0];
            }
        }
    }
}

void MonsterModeHandler::render_monster_editor() {
    if (!selected_monster_) return;

    ImGui::Separator();
    ImGui::Text("Editing Monster at (%d, %d)",
                selected_monster_->tile_x,
                selected_monster_->tile_y);

    // Dynamic preset selection for existing monster
    ImGui::Text("Monster Type:");
    bool preset_changed = false;

    const auto& presets = monster_preset_manager_.get_presets();

    // Display presets as a grid of 64x64 preview tiles
    const float tile_size = 64.0f;
    const float padding = 4.0f;
    const float available_width = ImGui::GetContentRegionAvail().x;
    const int tiles_per_row =
        static_cast<int>((available_width + padding) / (tile_size + padding));

    if (tiles_per_row > 0) {
        for (size_t i = 0; i < presets.size(); ++i) {
            const auto& preset = presets[i];
            ImGui::PushID(static_cast<int>(i + 3000));

            bool is_selected = (selected_monster_->preset_name == preset.name);

            // Load sprite info and texture
            MonsterSpriteInfo sprite_info =
                load_monster_sprite_info(preset.name);

            if (sprite_info.is_valid) {
                Texture2D texture =
                    load_monster_texture_cached(sprite_info.sprite_filename);

                if (texture.id != 0) {
                    // Calculate UV coordinates for the idle frame
                    float u0 =
                        (sprite_info.idle_col * sprite_info.sprite_width) /
                        static_cast<float>(texture.width);
                    float v0 =
                        (sprite_info.idle_row * sprite_info.sprite_height) /
                        static_cast<float>(texture.height);
                    float u1 = u0 + (sprite_info.sprite_width /
                                     static_cast<float>(texture.width));
                    float v1 = v0 + (sprite_info.sprite_height /
                                     static_cast<float>(texture.height));

                    // Highlight selected preset
                    ImVec4 tint_color = is_selected
                                            ? ImVec4(0.5f, 1.0f, 0.5f, 1.0f)
                                            : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

                    ImVec4 border_color = is_selected
                                              ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f)
                                              : ImVec4(0.4f, 0.4f, 0.4f, 0.5f);

                    // Draw tile as ImageButton
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                          ImVec4(0.3f, 0.3f, 0.3f, 0.3f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                          ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_Border, border_color);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);

                    char button_id[64];
                    std::snprintf(
                        button_id, sizeof(button_id), "##monster_edit_%zu", i);

                    if (ImGui::ImageButton(button_id,
                                           static_cast<ImTextureID>(texture.id),
                                           ImVec2(tile_size, tile_size),
                                           ImVec2(u0, v0),
                                           ImVec2(u1, v1),
                                           ImVec4(0, 0, 0, 0),  // background
                                           tint_color)) {
                        selected_monster_->preset_name = preset.name;
                        preset_changed = true;
                    }

                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(4);

                    // Show preset information on hover
                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::Text("%s", preset.display_name.c_str());
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                                           "Health: %d",
                                           preset.health);
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                                           "Speed: %d",
                                           preset.speed);
                        if (!preset.description.empty()) {
                            ImGui::Separator();
                            ImGui::TextWrapped("%s",
                                               preset.description.c_str());
                        }
                        ImGui::EndTooltip();
                    }
                } else {
                    // Fallback to text button
                    if (ImGui::Button(preset.display_name.c_str(),
                                      ImVec2(tile_size, tile_size))) {
                        selected_monster_->preset_name = preset.name;
                        preset_changed = true;
                    }

                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::Text("%s", preset.display_name.c_str());
                        ImGui::Text("Health: %d", preset.health);
                        ImGui::Text("Speed: %d", preset.speed);
                        if (!preset.description.empty()) {
                            ImGui::Separator();
                            ImGui::TextWrapped("%s",
                                               preset.description.c_str());
                        }
                        ImGui::EndTooltip();
                    }
                }
            } else {
                // Fallback to text button
                if (ImGui::Button(preset.display_name.c_str(),
                                  ImVec2(tile_size, tile_size))) {
                    selected_monster_->preset_name = preset.name;
                    preset_changed = true;
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s", preset.display_name.c_str());
                    ImGui::Text("Health: %d", preset.health);
                    ImGui::Text("Speed: %d", preset.speed);
                    if (!preset.description.empty()) {
                        ImGui::Separator();
                        ImGui::TextWrapped("%s", preset.description.c_str());
                    }
                    ImGui::EndTooltip();
                }
            }

            // Layout grid: add separator or new line
            if ((i + 1) % tiles_per_row != 0 && i < presets.size() - 1) {
                ImGui::SameLine(0.0f, padding);
            }

            ImGui::PopID();
        }
    }

    // Update color based on preset (using a color mapping system)
    if (preset_changed) {
        // Generate a consistent color based on preset name hash
        std::hash<std::string> hasher;
        size_t hash = hasher(selected_monster_->preset_name);

        // Use hash to generate RGB values with good contrast
        ImU8 r = 128 + (hash & 0xFF) / 2;          // 128-255 range
        ImU8 g = 128 + ((hash >> 8) & 0xFF) / 2;   // 128-255 range
        ImU8 b = 128 + ((hash >> 16) & 0xFF) / 2;  // 128-255 range

        selected_monster_->color = IM_COL32(r, g, b, 255);
    }

    // Show current preset info and allow overrides
    const auto* current_preset =
        monster_preset_manager_.get_preset(selected_monster_->preset_name);
    if (current_preset) {
        ImGui::Separator();
        ImGui::Text("Stats Override:");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                           "(Leave at -1 to use preset defaults)");

        // Health override
        int health_value = selected_monster_->health_override;
        if (health_value == -1) {
            health_value = current_preset->health;
        }

        ImGui::Text("Health:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80.0f);
        int temp_health = selected_monster_->health_override;
        if (ImGui::InputInt("##health_override", &temp_health, 1, 10)) {
            selected_monster_->health_override = temp_health;
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                           "(Default: %d)",
                           current_preset->health);
        if (selected_monster_->health_override != -1) {
            ImGui::SameLine();
            if (ImGui::SmallButton("Reset##health")) {
                selected_monster_->health_override = -1;
            }
        }

        // Speed override
        int speed_value = selected_monster_->speed_override;
        if (speed_value == -1) {
            speed_value = current_preset->speed;
        }

        ImGui::Text("Speed:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80.0f);
        int temp_speed = selected_monster_->speed_override;
        if (ImGui::InputInt("##speed_override", &temp_speed, 1, 10)) {
            selected_monster_->speed_override = temp_speed;
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                           "(Default: %d)",
                           current_preset->speed);
        if (selected_monster_->speed_override != -1) {
            ImGui::SameLine();
            if (ImGui::SmallButton("Reset##speed")) {
                selected_monster_->speed_override = -1;
            }
        }

        // Show current effective values
        ImGui::Separator();
        ImGui::Text("Current Values:");
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f),
                           "Health: %d, Speed: %d",
                           health_value,
                           speed_value);

        if (!current_preset->description.empty()) {
            ImGui::Separator();
            ImGui::TextWrapped("Description: %s",
                               current_preset->description.c_str());
        }
    }

    // Position info (read-only for now)
    ImGui::Separator();
    ImGui::Text("Position: Tile (%d, %d)",
                selected_monster_->tile_x,
                selected_monster_->tile_y);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                       "(Right-click monster in scene to delete)");
}
