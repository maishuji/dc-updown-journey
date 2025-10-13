// Copyright 2025 Quentin Cartier
#include "udjourney-editor/Editor.hpp"

#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <utility>
#include <vector>

#include "ImGuiFileDialog.h"
#include "raylib/raylib.h"
#include "udjourney-editor/EditorScene.hpp"
#include "udjourney-editor/Level.hpp"
#include "udjourney-editor/TilePanel.hpp"
#include "udjourney-editor/strategies/level/LevelCreationStrategy.hpp"
#include "udjourney-editor/ui/NewLevelPopup.hpp"

struct Editor::PImpl {
    bool running = true;
    float ui_scale = 2.0f;
    Level level;
    TilePanel tile_panel;
    EditorScene scene;
    std::string last_export_path;
    ImGuiStyle original_style;
    std::unique_ptr<LevelCreationStrategy> level_creation_strategy = nullptr;
    NewLevelPopup newLevelPopup;
};

Editor::Editor() : pimpl(std::make_unique<PImpl>()) { pimpl->running = true; }

Editor::~Editor() {
    // Cleanup resources if needed
    shutdown();
}

void Editor::set_level_creation_strategy(
    std::unique_ptr<LevelCreationStrategy> strategy) {
    pimpl->level_creation_strategy = std::move(strategy);
}

void Editor::export_tilemap_json(const std::string &export_path) {
    nlohmann::json jmap;
    jmap["rows"] = pimpl->level.row_cnt;
    jmap["cols"] = pimpl->level.col_cnt;
    jmap["tiles"] = nlohmann::json::array();
    for (size_t i = 0; i < pimpl->level.tiles.size(); ++i) {
        size_t row = i / pimpl->level.col_cnt;
        size_t col = i % pimpl->level.col_cnt;
        ImU32 color = pimpl->level.tiles[i].color;
        jmap["tiles"].push_back({{"row", row}, {"col", col}, {"color", color}});
    }
    std::ofstream out(export_path);
    out << jmap.dump(2);
    out.close();
    pimpl->last_export_path = export_path;
}

void Editor::import_tilemap_json(const std::string &import_path) {
    std::ifstream in(import_path);
    std::cout << "Importing tilemap from: " << import_path << std::endl;
    if (!in.is_open()) {
        std::cerr << "Failed to open file: " << import_path << std::endl;
        return;
    }
    nlohmann::json jmap;
    in >> jmap;
    in.close();
    pimpl->level.clear();
    pimpl->level.row_cnt = jmap["rows"].get<size_t>();
    pimpl->level.col_cnt = jmap["cols"].get<size_t>();

    pimpl->level.reserve(pimpl->level.row_cnt * pimpl->level.col_cnt);
    pimpl->level.resize(pimpl->level.row_cnt, pimpl->level.col_cnt);

    for (const auto &tile : jmap["tiles"]) {
        size_t row = tile["row"].get<size_t>();
        size_t col = tile["col"].get<size_t>();
        ImU32 color = tile["color"].get<ImU32>();
        if (row < pimpl->level.row_cnt && col < pimpl->level.col_cnt) {
            Cell cell;
            cell.color = color;
            pimpl->level.tiles.at(row * pimpl->level.col_cnt + col) = cell;
        } else {
            std::cerr << "Tile at (" << row << ", " << col
                      << ") is out of bounds, skipping." << std::endl;
            break;
        }
    }
}

void Editor::new_tilemap(int rows, int cols) noexcept {
    pimpl->level.clear();
    pimpl->level.resize(static_cast<size_t>(rows), static_cast<size_t>(cols));
    
    // Initialize all tiles with default cells
    for (size_t i = 0; i < pimpl->level.row_cnt * pimpl->level.col_cnt; ++i) {
        Cell default_cell;
        default_cell.color = IM_COL32(240, 240, 240, 255);  // Light gray for visibility
        pimpl->level.tiles[i] = default_cell;
    }
}

void Editor::init() {
    InitWindow(1200, 800, "UDJourney Editor");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    io.FontGlobalScale = 2.0f;  // Set global font scale
    pimpl->original_style = ImGui::GetStyle();

    ImGui::StyleColorsDark();

    // Initialize OpenGL3 backend
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::GetStyle().ScaleAllSizes(pimpl->ui_scale);

    // Initialize with a default level size
    new_tilemap(60, 30);
}

void Editor::run() {
    while (!WindowShouldClose()) {
        // Global keyboard shortcuts (using raylib for global detection)
        if (IsKeyPressed(KEY_F1)) {
            pimpl->tile_panel.request_focus();
        }
        
        // Export as JSON shortcut (Ctrl+E)
        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
            IsKeyPressed(KEY_E)) {
            export_tilemap_json("tilemap_export.json");
        }
        ImGuiIO &io = ImGui::GetIO();

        // Set display size each frame (fixes your crash)
        io.DisplaySize = ImVec2(static_cast<float>(GetScreenWidth()),
                                static_cast<float>(GetScreenHeight()));

        // Update ImGui inputs from raylib
        update_imgui_input();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        // --- Menu Bar ---
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Export Tilemap as JSON", "Ctrl+E")) {
                    std::cout << "Opening file dialog..." << std::endl;
                    auto config = IGFD::FileDialogConfig();
                    config.fileName = "export_tilemap.json";
                    ImGuiFileDialog::Instance()->OpenDialog(
                        "ChooseFileToExportJsonKey",
                        "Choose File",
                        ".json",
                        config);
                }
                if (ImGui::MenuItem("Import Tilemap from JSON", "Ctrl+I")) {
                    std::cout << "Opening file dialog..." << std::endl;
                    auto config = IGFD::FileDialogConfig();
                    config.fileName = "export_tilemap.json";
                    ImGuiFileDialog::Instance()->OpenDialog(
                        "ChooseFileToImportJsonKey",
                        "Choose File",
                        ".json",
                        config);
                }
                if (ImGui::MenuItem("New", "Ctrl+N")) {
                    std::cout << "Opening file dialog..." << std::endl;

                    if (pimpl->level_creation_strategy) {
                        // somewhere in your Editor class
                        pimpl->newLevelPopup.level = &pimpl->level;
                        pimpl->newLevelPopup.strategy =
                            pimpl->level_creation_strategy.get();
                        pimpl->newLevelPopup.open();

                        pimpl->level_creation_strategy->create(
                            pimpl->level,
                            pimpl->level.col_cnt,
                            pimpl->level.row_cnt);
                    } else {
                        new_tilemap(60,
                                    30);  // Default to 60x30 if no strategy set
                    }
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings")) {
                // ...inside ImGui::BeginMenu("File")...
                if (ImGui::BeginMenu("UI Scale")) {
                    float scales[] = {1.0f, 1.5f, 2.0f, 2.5f, 3.0f};
                    for (float scale : scales) {
                        char label[16];
                        snprintf(label, sizeof(label), "x%.1f", scale);
                        if (ImGui::MenuItem(
                                label, nullptr, pimpl->ui_scale == scale)) {
                            pimpl->ui_scale = scale;
                            ImGuiIO &io = ImGui::GetIO();
                            io.FontGlobalScale = scale;
                            ImGui::GetStyle() =
                                pimpl->original_style;  // Reset to original
                                                        // before scaling
                            ImGui::GetStyle().ScaleAllSizes(scale);
                            pimpl->tile_panel.set_scale(scale);
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (ImGuiFileDialog::Instance()->Display("ChooseFileToImportJsonKey",
                                                 ImGuiWindowFlags_NoCollapse,
                                                 ImVec2(600, 400),
                                                 ImVec2(FLT_MAX, FLT_MAX))) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath =
                    ImGuiFileDialog::Instance()->GetFilePathName();
                import_tilemap_json(filePath);
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display("ChooseFileToExportJsonKey",
                                                 ImGuiWindowFlags_NoCollapse,
                                                 ImVec2(600, 400),
                                                 ImVec2(FLT_MAX, FLT_MAX))) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath =
                    ImGuiFileDialog::Instance()->GetFilePathName();
                export_tilemap_json(filePath);
            }
            ImGuiFileDialog::Instance()->Close();
        }

        pimpl->tile_panel.draw();

        // Render the main scene view using EditorScene
        pimpl->scene.render(pimpl->level, pimpl->tile_panel);
        
        // Render UI popups after scene
        pimpl->newLevelPopup.render();

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
