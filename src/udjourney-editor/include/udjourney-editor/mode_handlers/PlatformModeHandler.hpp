// Copyright 2025 Quentin Cartier
#pragma once

#include <imgui.h>
#include <vector>

#include "udjourney-editor/mode_handlers/IModeHandler.hpp"
#include "udjourney-editor/Level.hpp"

/**
 * @brief Handler for Platform edit mode
 */
class PlatformModeHandler : public IModeHandler {
 public:
    PlatformModeHandler();

    void render() override;

    // Must be called outside any ImGui window
    void render_file_dialogs();
    void set_scale(float scale) override { scale_ = scale; }

    // Platform-specific API
    PlatformBehaviorType get_platform_behavior() const {
        return platform_behavior_;
    }
    ImVec2 get_platform_size() const { return platform_size_; }
    std::vector<PlatformFeatureType> get_selected_features() const;

    const std::string& get_new_platform_texture_file() const {
        return new_platform_texture_file_;
    }

    bool get_new_platform_texture_tiled() const noexcept {
        return new_platform_texture_tiled_;
    }

    EditorPlatform* get_selected_platform() const { return selected_platform_; }
    void set_selected_platform(EditorPlatform* platform) {
        selected_platform_ = platform;
    }

    // Get behavior parameters for new platforms
    std::map<std::string, float> get_behavior_params() const;

 private:
    float scale_ = 1.0f;
    ImVec2 platform_size_ = ImVec2(1.0f, 1.0f);
    PlatformBehaviorType platform_behavior_ = PlatformBehaviorType::Static;

    // Feature selection for new platforms
    bool feature_spikes_ = false;
    bool feature_checkpoint_ = false;

    // Behavior parameters for new platforms
    // Horizontal behavior
    float horizontal_speed_ = 2.0f;
    float horizontal_range_ = 5.0f;
    float horizontal_initial_offset_ = 0.0f;

    // Eight Turn behavior
    float eight_turn_speed_ = 1.0f;
    float eight_turn_amplitude_ = 4.0f;

    // Oscillating Size behavior
    float oscillating_speed_ = 2.0f;
    float oscillating_min_scale_ = 0.5f;
    float oscillating_max_scale_ = 1.5f;

    // Currently selected platform for editing
    EditorPlatform* selected_platform_ = nullptr;

    // Buffer for editing texture path (ImGui InputText needs stable storage)
    EditorPlatform* texture_platform_ = nullptr;
    char texture_file_buf_[256] = {0};

    // Default texture for newly created platforms
    std::string new_platform_texture_file_;
    char new_texture_file_buf_[256] = {0};
    bool new_platform_texture_tiled_ = false;

    enum class TextureDialogTarget {
        SelectedPlatform,
        NewPlatforms,
    };
    TextureDialogTarget texture_dialog_target_ =
        TextureDialogTarget::SelectedPlatform;

    void render_creator();
    void render_editor();
};
