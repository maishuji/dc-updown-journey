// NewLevelPopup.cpp
#include "udjourney-editor/ui/NewLevelPopup.hpp"

#include "imgui.h"

void NewLevelPopup::render() {
    if (show) {
        ImGui::OpenPopup("New Level Options");
        show = false;
    }

    if (ImGui::BeginPopupModal(
            "New Level Options", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter dimensions for the new level:");
        ImGui::InputInt("Columns", &cols);
        ImGui::InputInt("Rows", &rows);

        if (ImGui::Button("Create")) {
            if (strategy) {
                std::cout << "Creating new level with dimensions: " << cols
                          << "x" << rows << std::endl;
                strategy->create(*level, rows, cols);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
