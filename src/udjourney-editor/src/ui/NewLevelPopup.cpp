// Copyright 2025 Quentin Cartier
// NewLevelPopup.cpp
#include "udjourney-editor/ui/NewLevelPopup.hpp"

#include <imgui.h>

#include <iostream>
#include <memory>

#include "udjourney-editor/strategies/level/BlankLevelStrategy.hpp"
#include "udjourney-editor/strategies/level/StaircaseLevelStrategy.hpp"

void NewLevelPopup::render() {
    if (show) {
        ImGui::OpenPopup("New Level Options");
        show = false;
    }

    if (ImGui::BeginPopupModal(
            "New Level Options", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Create New Level");
        ImGui::Separator();

        ImGui::Text("Dimensions:");
        ImGui::InputInt("Columns", &cols);
        ImGui::InputInt("Rows", &rows);

        ImGui::Separator();
        ImGui::Text("Template:");

        if (ImGui::RadioButton("Blank",
                               selected_template == LevelTemplate::Blank)) {
            selected_template = LevelTemplate::Blank;
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Empty level");

        if (ImGui::RadioButton("Staircase",
                               selected_template == LevelTemplate::Staircase)) {
            selected_template = LevelTemplate::Staircase;
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                           "Descending zigzag platforms");

        ImGui::Separator();

        if (ImGui::Button("Create", ImVec2(120, 0))) {
            if (level) {
                std::cout << "Creating new level with dimensions: " << cols
                          << "x" << rows << " using template: ";

                std::unique_ptr<LevelCreationStrategy> chosen_strategy;

                switch (selected_template) {
                    case LevelTemplate::Blank:
                        std::cout << "Blank" << std::endl;
                        chosen_strategy =
                            std::make_unique<BlankLevelStrategy>();
                        break;
                    case LevelTemplate::Staircase:
                        std::cout << "Staircase" << std::endl;
                        chosen_strategy =
                            std::make_unique<StaircaseLevelStrategy>(3, 1);
                        break;
                }

                if (chosen_strategy) {
                    chosen_strategy->create(*level, rows, cols);
                }
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
