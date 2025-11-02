// Copyright 2025 Quentin Cartier
#pragma once

#include <imgui.h>

namespace udjourney {

/**
 * Helper class to manage docking layout.
 * Isolates the use of imgui_internal.h to avoid exposing internal APIs.
 */
class DockingHelper {
public:
    /**
     * Set up the default docking layout with Editor Panel on left and Scene View on right.
     * 
     * @param dockspace_id The ID of the dockspace to configure
     * @param size The size of the dockspace
     */
    static void SetupDefaultLayout(ImGuiID dockspace_id, const ImVec2& size);
};

} // namespace udjourney
