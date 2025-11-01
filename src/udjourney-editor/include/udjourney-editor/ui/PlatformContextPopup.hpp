// Copyright 2025 Quentin Cartier
#pragma once

#include <imgui.h>
#include "../Level.hpp"
#include <algorithm>

struct PlatformContextPopup {
    bool show = false;
    EditorPlatform* editing_platform = nullptr;
    
    // Temporary storage for editing
    float platform_width = 1.0f;
    float platform_height = 1.0f;
    PlatformBehaviorType selected_behavior = PlatformBehaviorType::Static;
    bool has_spikes = false;
    bool has_checkpoint = false;
    bool should_delete = false;

    void open(EditorPlatform* platform) {
        if (!platform) return;
        
        editing_platform = platform;
        platform_width = platform->width_tiles;
        platform_height = platform->height_tiles;
        selected_behavior = platform->behavior_type;
        
        // Check current features
        has_spikes = std::find(platform->features.begin(), platform->features.end(), 
                              PlatformFeatureType::Spikes) != platform->features.end();
        has_checkpoint = std::find(platform->features.begin(), platform->features.end(), 
                                   PlatformFeatureType::Checkpoint) != platform->features.end();
        
        should_delete = false;
        show = true;
    }

    void render() {
        if (!show || !editing_platform) {
            return;
        }

        ImGui::OpenPopup("Platform Properties");
        
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(350, 450), ImGuiCond_Appearing);

        if (ImGui::BeginPopupModal("Platform Properties", &show, ImGuiWindowFlags_NoResize)) {
            // Platform Position (read-only, informational)
            ImGui::SeparatorText("Position");
            ImGui::Text("Tile X: %d", editing_platform->tile_x);
            ImGui::Text("Tile Y: %d", editing_platform->tile_y);
            ImGui::Text("World X: %.1f", editing_platform->tile_x * 32.0f);
            ImGui::Text("World Y: %.1f", editing_platform->tile_y * 32.0f);
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Platform Behavior
            ImGui::SeparatorText("Behavior Type");
            
            const char* behavior_names[] = {
                "Static",
                "Horizontal",
                "Eight Turn",
                "Oscillating Size"
            };
            
            int current_behavior = static_cast<int>(selected_behavior);
            if (ImGui::Combo("##Behavior", &current_behavior, behavior_names, IM_ARRAYSIZE(behavior_names))) {
                selected_behavior = static_cast<PlatformBehaviorType>(current_behavior);
                editing_platform->behavior_type = selected_behavior;
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Platform Size
            ImGui::SeparatorText("Size (in tiles)");
            
            if (ImGui::SliderFloat("Width", &platform_width, 0.5f, 5.0f, "%.1f")) {
                editing_platform->width_tiles = platform_width;
            }
            
            if (ImGui::SliderFloat("Height", &platform_height, 0.5f, 3.0f, "%.1f")) {
                editing_platform->height_tiles = platform_height;
            }
            
            // Show world size
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), 
                "World size: %.0fx%.0f pixels", 
                platform_width * 32.0f, 
                platform_height * 32.0f);
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Platform Features
            ImGui::SeparatorText("Features");
            
            bool features_changed = false;
            
            if (ImGui::Checkbox("Spikes", &has_spikes)) {
                features_changed = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Damages the player on contact");
            }
            
            if (ImGui::Checkbox("Checkpoint", &has_checkpoint)) {
                features_changed = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Sets respawn point when touched");
            }
            
            // Update features if changed
            if (features_changed) {
                editing_platform->features.clear();
                if (has_spikes) {
                    editing_platform->features.push_back(PlatformFeatureType::Spikes);
                }
                if (has_checkpoint) {
                    editing_platform->features.push_back(PlatformFeatureType::Checkpoint);
                }
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Action Buttons
            ImGui::BeginGroup();
            
            // Delete button (red, full width)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
            
            if (ImGui::Button("Delete Platform", ImVec2(-1, 40))) {
                should_delete = true;
                show = false;
            }
            
            ImGui::PopStyleColor(3);
            
            ImGui::Spacing();
            
            // Close button (green, full width)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.2f, 1.0f));
            
            if (ImGui::Button("Done", ImVec2(-1, 40))) {
                show = false;
            }
            
            ImGui::PopStyleColor(3);
            
            ImGui::EndGroup();

            ImGui::EndPopup();
        }
    }
    
    void close() {
        show = false;
        editing_platform = nullptr;
        should_delete = false;
    }
    
    bool wants_to_delete_platform() const {
        return should_delete;
    }
    
    EditorPlatform* get_platform_to_delete() const {
        return should_delete ? editing_platform : nullptr;
    }
};