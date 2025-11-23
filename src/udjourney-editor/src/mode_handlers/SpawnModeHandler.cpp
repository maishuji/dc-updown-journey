// Copyright 2025 Quentin Cartier
#include "udjourney-editor/mode_handlers/SpawnModeHandler.hpp"

#include <imgui.h>

SpawnModeHandler::SpawnModeHandler() {}

void SpawnModeHandler::render() {
    ImGui::Text("Player Spawn");
    ImGui::Separator();
    ImGui::Text("Click on the grid to set");
    ImGui::Text("the player spawn position.");
}
