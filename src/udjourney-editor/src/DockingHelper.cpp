// Copyright 2025 Quentin Cartier
#include "udjourney-editor/DockingHelper.hpp"

#include <imgui_internal.h>  // Only include internal header here

namespace udjourney {

void DockingHelper::SetupDefaultLayout(ImGuiID dockspace_id, const ImVec2& size) {
    // Remove any existing layout
    ImGui::DockBuilderRemoveNode(dockspace_id);
    
    // Create a new dockspace node
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, size);
    
    // Split the dockspace: left side for Editor Panel (30% width), right for Scene View
    ImGuiID dock_left_id = 0;
    ImGuiID dock_right_id = 0;
    ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.30f, &dock_left_id, &dock_right_id);
    
    // Dock the Editor Panel to the left
    ImGui::DockBuilderDockWindow("Editor Panel", dock_left_id);
    
    // Dock the Scene View to the right (center)
    ImGui::DockBuilderDockWindow("Scene View", dock_right_id);
    
    // Finalize the layout
    ImGui::DockBuilderFinish(dockspace_id);
}

} // namespace udjourney
