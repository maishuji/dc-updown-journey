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
    void set_scale(float scale) override { scale_ = scale; }

    // Platform-specific API
    PlatformBehaviorType get_platform_behavior() const {
        return platform_behavior_;
    }
    ImVec2 get_platform_size() const { return platform_size_; }
    std::vector<PlatformFeatureType> get_selected_features() const;

    EditorPlatform* get_selected_platform() const { return selected_platform_; }
    void set_selected_platform(EditorPlatform* platform) {
        selected_platform_ = platform;
    }

 private:
    float scale_ = 1.0f;
    ImVec2 platform_size_ = ImVec2(1.0f, 1.0f);
    PlatformBehaviorType platform_behavior_ = PlatformBehaviorType::Static;

    // Feature selection for new platforms
    bool feature_spikes_ = false;
    bool feature_checkpoint_ = false;

    // Currently selected platform for editing
    EditorPlatform* selected_platform_ = nullptr;

    void render_creator();
    void render_editor();
};
