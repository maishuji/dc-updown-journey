// Copyright 2025 Quentin Cartier
#pragma once

#include <string>
#include <memory>

#include "udjourney-editor/mode_handlers/IModeHandler.hpp"
#include "udjourney-editor/Level.hpp"
#include "udjourney-editor/hud/HUDPresetManager.hpp"
#include "udjourney-editor/hud/UIAtlasPresetManager.hpp"

/**
 * @brief Handler for HUD (Fixed UI Display) edit mode
 */
class HUDModeHandler : public IModeHandler {
 public:
    HUDModeHandler();

    void render() override;
    void set_scale(float scale) override { scale_ = scale; }

    // FUD-specific API
    const std::string& get_selected_fud_preset() const {
        return selected_fud_preset_;
    }

    HUDElement* get_selected_fud() const { return selected_fud_; }

    void set_selected_fud(HUDElement* hud) { selected_fud_ = hud; }

    bool should_delete_selected_fud() const { return delete_selected_fud_; }

    void clear_delete_flag() {
        delete_selected_fud_ = false;
        selected_fud_ = nullptr;
    }

    bool should_add_fud() const { return add_fud_requested_; }
    void clear_add_flag() { add_fud_requested_ = false; }

    // Create a new HUD element from the selected preset
    HUDElement create_fud_from_preset() const;

    void initialize_presets();

    const HUDPresetManager& get_fud_preset_manager() const {
        return fud_preset_manager_;
    }

 private:
    float scale_ = 1.0f;
    std::string selected_fud_preset_;
    HUDElement* selected_fud_ = nullptr;
    bool delete_selected_fud_ = false;
    bool add_fud_requested_ = false;

    HUDPresetManager fud_preset_manager_;
    UIAtlasPresetManager ui_atlas_manager_;

    int selected_background_sprite_ = -1;
    int selected_foreground_sprite_ = -1;

    void render_fud_editor();
    void render_fud_preset_selector();
    void render_fud_properties();
    void render_sprite_selector(const char* label, bool is_background);
    void render_property_editor();
};
