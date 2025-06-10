// Copyright 2025 Quentin Cartier
#include "udjourney-editor/Editor.hpp"

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <cmath>
#include <memory>
#include <utility>
#include <vector>

#include "raylib/raylib.h"

struct Cell {
    ImU32 color = IM_COL32(2550, 255, 255, 255);  // Default color for the cell
};

struct Editor::PImpl {
    bool running = true;
    bool selecting = false;
    bool selection_done = false;
    ImVec2 selection_start;
    ImVec2 selection_end;
    size_t row_cnt = 20;
    size_t col_cnt = 20;
    std::vector<Cell> tiles;
};

Editor::Editor() : pimpl(std::make_unique<PImpl>()) { pimpl->running = true; }

Editor::~Editor() {
    // Cleanup resources if needed
    shutdown();
}

void Editor::init() {
    InitWindow(800, 600, "UDJourney Editor");
    SetTargetFPS(60);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    // Initialize OpenGL3 backend
    ImGui_ImplOpenGL3_Init("#version 330");

    pimpl->tiles.reserve(pimpl->row_cnt * pimpl->col_cnt);
    for (size_t i = 0; i < pimpl->row_cnt * pimpl->col_cnt; ++i) {
        pimpl->tiles.push_back(Cell());
    }
}

void Editor::run() {
    while (!WindowShouldClose()) {
        ImGuiIO &io = ImGui::GetIO();

        // Set display size each frame (fixes your crash)
        io.DisplaySize = ImVec2(static_cast<float>(GetScreenWidth()),
                                static_cast<float>(GetScreenHeight()));

        // Update ImGui inputs from raylib
        update_imgui_input();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        draw_tiles_panel();

        // Set up the main scene view
        ImGui::SetNextWindowPos(ImVec2(150, 0),
                                ImGuiCond_Always);  // adjust offset
        ImGui::SetNextWindowSize(
            ImVec2(io.DisplaySize.x - 150, io.DisplaySize.y), ImGuiCond_Always);

        ImGui::Begin("Scene View",
                     nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoCollapse);

        // Get draw list inside the ImGui scene view window
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 origin = ImGui::GetCursorScreenPos();

        // Example canvas grid (draw with ImGui)
        const float tile_size = 32.0f;

        for (int y = 0; y < pimpl->row_cnt; ++y) {
            for (int x = 0; x < pimpl->col_cnt; ++x) {
                ImVec2 top_left =
                    ImVec2(origin.x + x * tile_size, origin.y + y * tile_size);
                ImVec2 bottom_right =
                    ImVec2(top_left.x + tile_size, top_left.y + tile_size);
                // draw_list->AddRect(
                //     top_left, bottom_right, IM_COL32(200, 200, 200, 255));

                draw_list->AddRectFilled(
                    top_left,
                    bottom_right,
                    pimpl->tiles[y * pimpl->col_cnt + x].color);
                draw_list->AddRect(
                    top_left, bottom_right, IM_COL32(200, 200, 200, 255));
            }
        }

        ImVec2 mouse_pos = ImGui::GetMousePos();
        bool hovered = ImGui::IsWindowHovered();
        bool clicked = ImGui::IsMouseClicked(0);
        bool released = ImGui::IsMouseReleased(0);

        // --- Selection logic ---
        if (hovered && clicked) {
            pimpl->selecting = true;
            pimpl->selection_done = false;
            pimpl->selection_start = mouse_pos;
            pimpl->selection_end = mouse_pos;
        }
        if (pimpl->selecting) {
            pimpl->selection_end = mouse_pos;
            if (released) {
                pimpl->selecting = false;
                pimpl->selection_done = true;
            }
        }

        // --- Draw selection rectangle ---
        if (pimpl->selecting) {
            ImU32 col = IM_COL32(0, 120, 255, 100);     // semi-transparent fill
            ImU32 border = IM_COL32(0, 120, 255, 255);  // solid border

            ImVec2 p_min =
                ImVec2(fminf(pimpl->selection_start.x, pimpl->selection_end.x),
                       fminf(pimpl->selection_start.y, pimpl->selection_end.y));
            ImVec2 p_max =
                ImVec2(fmaxf(pimpl->selection_start.x, pimpl->selection_end.x),
                       fmaxf(pimpl->selection_start.y, pimpl->selection_end.y));

            draw_list->AddRectFilled(p_min, p_max, col);
            draw_list->AddRect(p_min, p_max, border, 0.0f, 0, 2.0f);
        }

        if (pimpl->selection_done) {
            ImVec2 p_min =
                ImVec2(fminf(pimpl->selection_start.x, pimpl->selection_end.x),
                       fminf(pimpl->selection_start.y, pimpl->selection_end.y));
            ImVec2 p_max =
                ImVec2(fmaxf(pimpl->selection_start.x, pimpl->selection_end.x),
                       fmaxf(pimpl->selection_start.y, pimpl->selection_end.y));

            for (int y = 0; y < pimpl->row_cnt; ++y) {
                for (int x = 0; x < pimpl->col_cnt; ++x) {
                    ImVec2 tile_top_left = ImVec2(origin.x + x * tile_size,
                                                  origin.y + y * tile_size);
                    ImVec2 tile_bottom_right =
                        ImVec2(tile_top_left.x + tile_size,
                               tile_top_left.y + tile_size);

                    // Check if tile is fully inside the selection rectangle
                    if (tile_top_left.x >= p_min.x &&
                        tile_bottom_right.x <= p_max.x &&
                        tile_top_left.y >= p_min.y &&
                        tile_bottom_right.y <= p_max.y) {
                        pimpl->tiles[y * pimpl->col_cnt + x].color =
                            IM_COL32(0, 0, 255, 255);  // Highlight color

                        // Selected tile rectangle
                        draw_list->AddRect(tile_top_left,
                                                 tile_bottom_right,
                                                 IM_COL32(255, 0, 0, 100));
                    }
                }
            }
        }

        // Reserve space so ImGui knows where we've "drawn"
        ImGui::Dummy(
            ImVec2(pimpl->col_cnt * tile_size, pimpl->row_cnt * tile_size));

        ImGui::End();

        ImGui::Render();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Render ImGui draw data
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        EndDrawing();
    }
}
void Editor::shutdown() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    CloseWindow();
}

void Editor::update_imgui_input() {
    ImGuiIO &io = ImGui::GetIO();

    auto RaylibKeyToImGuiKey = [](int key) -> ImGuiKey {
        switch (key) {
            case KEY_SPACE:
                return ImGuiKey_Space;
            case KEY_ENTER:
                return ImGuiKey_Enter;
            case KEY_TAB:
                return ImGuiKey_Tab;
            case KEY_BACKSPACE:
                return ImGuiKey_Backspace;
            case KEY_ESCAPE:
                return ImGuiKey_Escape;
            case KEY_LEFT:
                return ImGuiKey_LeftArrow;
            case KEY_RIGHT:
                return ImGuiKey_RightArrow;
            case KEY_UP:
                return ImGuiKey_UpArrow;
            case KEY_DOWN:
                return ImGuiKey_DownArrow;

            // Modifier keys example:
            case KEY_LEFT_CONTROL:
            case KEY_RIGHT_CONTROL:
                return ImGuiKey_ModCtrl;
            case KEY_LEFT_SHIFT:
            case KEY_RIGHT_SHIFT:
                return ImGuiKey_ModShift;
            case KEY_LEFT_ALT:
            case KEY_RIGHT_ALT:
                return ImGuiKey_ModAlt;
            case KEY_LEFT_SUPER:
            case KEY_RIGHT_SUPER:
                return ImGuiKey_ModSuper;

            default:
                return ImGuiKey_None;  // Ignore unmapped keys
        }
    };

    // Mouse position
    io.MousePos = {static_cast<float>(GetMouseX()),
                   static_cast<float>(GetMouseY())};

    // Mouse buttons
    io.MouseDown[0] = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    io.MouseDown[1] = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
    io.MouseDown[2] = IsMouseButtonDown(MOUSE_MIDDLE_BUTTON);

    // Mouse wheel
    io.MouseWheel += GetMouseWheelMove();

    // Keyboard input (basic)
    io.KeyCtrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    io.KeyShift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    io.KeyAlt = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
    io.KeySuper = IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);

    // ImGui keys - update as needed, here is minimal example:

    for (int key = 0; key < 512; key++) {
        ImGuiKey imgui_key = RaylibKeyToImGuiKey(key);
        if (imgui_key != ImGuiKey_None) {
            bool pressed = IsKeyDown(key);
            io.AddKeyEvent(imgui_key, pressed);
        }
    }
}

void Editor::draw_tiles_panel() {
    ImGui::Begin("Tiles Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Tile Picker");
    ImGui::Separator();

    // Example tiles
    if (ImGui::Button("Brick")) { /* select brick */
    }
    ImGui::SameLine();
    if (ImGui::Button("Grass")) { /* select grass */
    }
    ImGui::SameLine();
    if (ImGui::Button("Water")) { /* select water */
    }
    if (ImGui::Button("Brick2")) { /* select brick */
    }
    ImGui::SameLine();
    if (ImGui::Button("Grass2")) { /* select grass */
    }
    ImGui::SameLine();
    if (ImGui::Button("Water2")) { /* select water */
    }

    ImGui::End();
}
