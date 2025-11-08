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
#include "udjourney-editor/DockingHelper.hpp"
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

#ifdef EDITOR_TESTING
Level &Editor::get_test_level() { return pimpl->level; }
#endif

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
        default_cell.color =
            IM_COL32(240, 240, 240, 255);  // Light gray for visibility
        pimpl->level.tiles[i] = default_cell;
    }
}

void Editor::export_platform_level_json(const std::string &export_path) {
    nlohmann::json jlevel;

    // Level metadata
    jlevel["name"] = "Untitled Level";

    // Player spawn position
    jlevel["player_spawn"] = {{"x", pimpl->level.player_spawn_x},
                              {"y", pimpl->level.player_spawn_y}};

    // Platforms array
    jlevel["platforms"] = nlohmann::json::array();

    for (const auto &platform : pimpl->level.platforms) {
        nlohmann::json jplatform;
        jplatform["x"] = platform.tile_x;
        jplatform["y"] = platform.tile_y;
        jplatform["width"] = platform.width_tiles;
        jplatform["height"] = platform.height_tiles;

        // Behavior type
        switch (platform.behavior_type) {
            case PlatformBehaviorType::Static:
                jplatform["behavior"] = "static";
                break;
            case PlatformBehaviorType::Horizontal:
                jplatform["behavior"] = "horizontal";
                break;
            case PlatformBehaviorType::EightTurnHorizontal:
                jplatform["behavior"] = "eight_turn";
                break;
            case PlatformBehaviorType::OscillatingSize:
                jplatform["behavior"] = "oscillating_size";
                break;
        }

        // Features
        if (!platform.features.empty()) {
            jplatform["features"] = nlohmann::json::array();
            for (const auto &feature : platform.features) {
                switch (feature) {
                    case PlatformFeatureType::Spikes:
                        jplatform["features"].push_back("spikes");
                        break;
                    case PlatformFeatureType::Checkpoint:
                        jplatform["features"].push_back("checkpoint");
                        break;
                    case PlatformFeatureType::None:
                        break;
                }
            }
        }

        jlevel["platforms"].push_back(jplatform);
    }

    std::ofstream out(export_path);
    out << jlevel.dump(2);
    out.close();
    pimpl->last_export_path = export_path;
}

void Editor::import_platform_level_json(const std::string &import_path) {
    std::ifstream in(import_path);
    std::cout << "Importing platform level from: " << import_path << std::endl;
    if (!in.is_open()) {
        std::cerr << "Failed to open file: " << import_path << std::endl;
        return;
    }

    nlohmann::json jlevel;
    in >> jlevel;
    in.close();

    // Clear existing platforms
    pimpl->level.platforms.clear();

    // Import player spawn position
    if (jlevel.contains("player_spawn")) {
        pimpl->level.player_spawn_x = jlevel["player_spawn"]["x"].get<int>();
        pimpl->level.player_spawn_y = jlevel["player_spawn"]["y"].get<int>();
    }

    // Import platforms
    if (jlevel.contains("platforms")) {
        for (const auto &jplatform : jlevel["platforms"]) {
            EditorPlatform platform;
            platform.tile_x = jplatform["x"].get<int>();
            platform.tile_y = jplatform["y"].get<int>();
            platform.width_tiles = jplatform["width"].get<float>();
            platform.height_tiles = jplatform["height"].get<float>();

            // Parse behavior type
            std::string behavior = jplatform["behavior"].get<std::string>();
            if (behavior == "static") {
                platform.behavior_type = PlatformBehaviorType::Static;
            } else if (behavior == "horizontal") {
                platform.behavior_type = PlatformBehaviorType::Horizontal;
            } else if (behavior == "eight_turn") {
                platform.behavior_type =
                    PlatformBehaviorType::EightTurnHorizontal;
            } else if (behavior == "oscillating_size") {
                platform.behavior_type = PlatformBehaviorType::OscillatingSize;
            }

            // Parse features
            platform.features.clear();
            if (jplatform.contains("features")) {
                for (const auto &feature_str : jplatform["features"]) {
                    std::string feature = feature_str.get<std::string>();
                    if (feature == "spikes") {
                        platform.features.push_back(
                            PlatformFeatureType::Spikes);
                    } else if (feature == "checkpoint") {
                        platform.features.push_back(
                            PlatformFeatureType::Checkpoint);
                    }
                }
            }

            // Set platform color based on behavior and features
            PlatformFeatureType primary_feature =
                platform.features.empty() ? PlatformFeatureType::None
                                          : platform.features[0];

            // Simple color assignment based on behavior type
            switch (platform.behavior_type) {
                case PlatformBehaviorType::Static:
                    platform.color = IM_COL32(0, 0, 255, 255);  // Blue
                    break;
                case PlatformBehaviorType::Horizontal:
                    platform.color = IM_COL32(255, 128, 0, 255);  // Orange
                    break;
                case PlatformBehaviorType::EightTurnHorizontal:
                    platform.color = IM_COL32(128, 0, 255, 255);  // Purple
                    break;
                case PlatformBehaviorType::OscillatingSize:
                    platform.color = IM_COL32(0, 255, 128, 255);  // Light Green
                    break;
            }

            // Override with feature colors if present
            if (primary_feature == PlatformFeatureType::Spikes) {
                platform.color = IM_COL32(255, 0, 0, 255);  // Red for spikes
            } else if (primary_feature == PlatformFeatureType::Checkpoint) {
                platform.color =
                    IM_COL32(0, 255, 0, 255);  // Green for checkpoint
            }

            pimpl->level.platforms.push_back(platform);
        }
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

    // Enable docking
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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

        // Export platform level shortcut (Ctrl+Shift+E)
        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
            (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
            IsKeyPressed(KEY_E)) {
            export_platform_level_json("platform_level_export.json");
        }

        // Import platform level shortcut (Ctrl+Shift+I)
        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
            (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) &&
            IsKeyPressed(KEY_I)) {
            // This would open the file dialog, but for quick access we'll use a
            // fixed name Users can still use the menu for file dialog
            std::cout
                << "Platform import shortcut - use File menu for file selection"
                << std::endl;
        }

        ImGuiIO &io = ImGui::GetIO();

        // Set display size each frame (fixes your crash)
        io.DisplaySize = ImVec2(static_cast<float>(GetScreenWidth()),
                                static_cast<float>(GetScreenHeight()));

        // Update ImGui inputs from raylib
        update_imgui_input();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        // --- Create Dockspace ---
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags dockspace_flags =
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        dockspace_flags |=
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        dockspace_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        dockspace_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus |
                           ImGuiWindowFlags_NoNavFocus;
        dockspace_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpace", nullptr, dockspace_flags);
        ImGui::PopStyleVar(3);

        // Create the dockspace - windows can now be docked
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(
            dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        // First time setup: create default docking layout using helper
        static bool first_time = true;
        if (first_time) {
            first_time = false;
            udjourney::DockingHelper::SetupDefaultLayout(dockspace_id,
                                                         viewport->WorkSize);
        }

        // --- Menu Bar ---
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Export Tilemap as JSON", "Ctrl+E")) {
                    std::cout << "Opening file dialog..." << std::endl;
                    auto config = IGFD::FileDialogConfig();
                    config.fileName = "my_tilemap.json";  // Default name but
                                                          // user can change it
                    ImGuiFileDialog::Instance()->OpenDialog(
                        "ChooseFileToExportJsonKey",
                        "Export Tilemap as JSON",
                        ".json",
                        config);
                }
                if (ImGui::MenuItem("Import Tilemap from JSON", "Ctrl+I")) {
                    std::cout << "Opening file dialog..." << std::endl;
                    auto config = IGFD::FileDialogConfig();
                    config.fileName =
                        "";  // No default for import - user browses files
                    ImGuiFileDialog::Instance()->OpenDialog(
                        "ChooseFileToImportJsonKey",
                        "Import Tilemap from JSON",
                        ".json",
                        config);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Export Platform Level", "Ctrl+Shift+E")) {
                    std::cout << "Opening platform level export dialog..."
                              << std::endl;
                    auto config = IGFD::FileDialogConfig();
                    config.fileName =
                        "my_platform_level.json";  // Default name but user can
                                                   // change it
                    ImGuiFileDialog::Instance()->OpenDialog(
                        "ChooseFileToExportPlatformKey",
                        "Export Platform Level as JSON",
                        ".json",
                        config);
                }
                if (ImGui::MenuItem("Import Platform Level", "Ctrl+Shift+I")) {
                    std::cout << "Opening platform level import dialog..."
                              << std::endl;
                    auto config = IGFD::FileDialogConfig();
                    config.fileName =
                        "";  // No default for import - user browses files
                    ImGuiFileDialog::Instance()->OpenDialog(
                        "ChooseFileToImportPlatformKey",
                        "Import Platform Level from JSON",
                        ".json",
                        config);
                }
                ImGui::Separator();
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

            ImGui::EndMenuBar();
        }

        ImGui::End();  // End DockSpace window

        if (ImGuiFileDialog::Instance()->Display("ChooseFileToImportJsonKey",
                                                 ImGuiWindowFlags_None,
                                                 ImVec2(1000, 700),
                                                 ImVec2(FLT_MAX, FLT_MAX))) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath =
                    ImGuiFileDialog::Instance()->GetFilePathName();
                import_tilemap_json(filePath);
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display("ChooseFileToExportJsonKey",
                                                 ImGuiWindowFlags_None,
                                                 ImVec2(1000, 700),
                                                 ImVec2(FLT_MAX, FLT_MAX))) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath =
                    ImGuiFileDialog::Instance()->GetFilePathName();
                export_tilemap_json(filePath);
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display(
                "ChooseFileToImportPlatformKey",
                ImGuiWindowFlags_None,
                ImVec2(1000, 700),
                ImVec2(FLT_MAX, FLT_MAX))) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath =
                    ImGuiFileDialog::Instance()->GetFilePathName();
                import_platform_level_json(filePath);
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display(
                "ChooseFileToExportPlatformKey",
                ImGuiWindowFlags_None,
                ImVec2(1000, 700),
                ImVec2(FLT_MAX, FLT_MAX))) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath =
                    ImGuiFileDialog::Instance()->GetFilePathName();
                export_platform_level_json(filePath);
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
            case KEY_DELETE:
                return ImGuiKey_Delete;
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
            case KEY_HOME:
                return ImGuiKey_Home;
            case KEY_END:
                return ImGuiKey_End;
            case KEY_PAGE_UP:
                return ImGuiKey_PageUp;
            case KEY_PAGE_DOWN:
                return ImGuiKey_PageDown;

            // Modifier keys
            case KEY_LEFT_CONTROL:
            case KEY_RIGHT_CONTROL:
                return ImGuiKey_LeftCtrl;
            case KEY_LEFT_SHIFT:
            case KEY_RIGHT_SHIFT:
                return ImGuiKey_LeftShift;
            case KEY_LEFT_ALT:
            case KEY_RIGHT_ALT:
                return ImGuiKey_LeftAlt;
            case KEY_LEFT_SUPER:
            case KEY_RIGHT_SUPER:
                return ImGuiKey_LeftSuper;

            // Letter keys
            case KEY_A:
                return ImGuiKey_A;
            case KEY_B:
                return ImGuiKey_B;
            case KEY_C:
                return ImGuiKey_C;
            case KEY_D:
                return ImGuiKey_D;
            case KEY_E:
                return ImGuiKey_E;
            case KEY_F:
                return ImGuiKey_F;
            case KEY_G:
                return ImGuiKey_G;
            case KEY_H:
                return ImGuiKey_H;
            case KEY_I:
                return ImGuiKey_I;
            case KEY_J:
                return ImGuiKey_J;
            case KEY_K:
                return ImGuiKey_K;
            case KEY_L:
                return ImGuiKey_L;
            case KEY_M:
                return ImGuiKey_M;
            case KEY_N:
                return ImGuiKey_N;
            case KEY_O:
                return ImGuiKey_O;
            case KEY_P:
                return ImGuiKey_P;
            case KEY_Q:
                return ImGuiKey_Q;
            case KEY_R:
                return ImGuiKey_R;
            case KEY_S:
                return ImGuiKey_S;
            case KEY_T:
                return ImGuiKey_T;
            case KEY_U:
                return ImGuiKey_U;
            case KEY_V:
                return ImGuiKey_V;
            case KEY_W:
                return ImGuiKey_W;
            case KEY_X:
                return ImGuiKey_X;
            case KEY_Y:
                return ImGuiKey_Y;
            case KEY_Z:
                return ImGuiKey_Z;

            // Number keys
            case KEY_ZERO:
                return ImGuiKey_0;
            case KEY_ONE:
                return ImGuiKey_1;
            case KEY_TWO:
                return ImGuiKey_2;
            case KEY_THREE:
                return ImGuiKey_3;
            case KEY_FOUR:
                return ImGuiKey_4;
            case KEY_FIVE:
                return ImGuiKey_5;
            case KEY_SIX:
                return ImGuiKey_6;
            case KEY_SEVEN:
                return ImGuiKey_7;
            case KEY_EIGHT:
                return ImGuiKey_8;
            case KEY_NINE:
                return ImGuiKey_9;

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

    // Text input for typing in text fields
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key < 127) {  // Printable ASCII characters
            io.AddInputCharacter((unsigned int)key);
        }
        key = GetCharPressed();  // Get next character in queue
    }

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
