// Copyright 2025 Quentin Cartier
#include "udjourney-editor/Editor.hpp"

// C system headers (imgui treated as C system by cpplint)
#include <imgui.h>
#include <imgui_impl_opengl3.h>

// C++ standard library headers
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Third-party library headers
#include <nlohmann/json.hpp>

// Project headers
#include "ImGuiFileDialog.h"
#include "raylib/raylib.h"
#include "udjourney-editor/DockingHelper.hpp"
#include "udjourney-editor/EditorScene.hpp"
#include "udjourney-editor/Level.hpp"
#include "udjourney-editor/EditorPanel.hpp"
#include "udjourney-editor/EditorSettings.hpp"
#include "udjourney-editor/background/BackgroundManager.hpp"
#include "udjourney-editor/background/BackgroundObjectPresetManager.hpp"
#include "udjourney-editor/strategies/level/LevelCreationStrategy.hpp"
#include "udjourney-editor/strategies/level/ImportFromFileStrategy.hpp"
#include "udjourney-editor/ui/NewLevelPopup.hpp"
#include "udjourney-editor/AnimationPresetPanel.hpp"
#include "udjourney-editor/ParticlePresetPanel.hpp"
#include "udjourney-editor/ToastNotification.hpp"
#include "udj-core/CoreUtils.hpp"

struct Editor::PImpl {
    bool running = true;
    float ui_scale = 2.0f;
    Level level;
    EditorPanel editor_panel;
    EditorScene scene;
    BackgroundManager background_manager;
    BackgroundObjectPresetManager background_preset_manager;
    std::string last_export_path;
    ImGuiStyle original_style;
    std::unique_ptr<LevelCreationStrategy> level_creation_strategy = nullptr;
    NewLevelPopup newLevelPopup;
    udjourney::editor::AnimationPresetPanel animation_preset_panel;
    udjourney::editor::ParticlePresetPanel particle_preset_panel;
    udjourney::editor::ToastManager toast_manager;
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
    jlevel["scroll_speed"] = pimpl->level.scroll_speed;
    jlevel["gravity"] = pimpl->level.physics_config.gravity;
    jlevel["terminal_velocity"] = pimpl->level.physics_config.terminal_velocity;

    // Scene type
    jlevel["scene_type"] = (pimpl->level.scene_type == SceneType::UI_SCREEN)
                               ? "ui_screen"
                               : "level";

    // Player spawn position (only for levels)
    if (pimpl->level.scene_type == SceneType::LEVEL) {
        jlevel["player_spawn"] = {{"x", pimpl->level.player_spawn_x},
                                  {"y", pimpl->level.player_spawn_y}};
    }

    // Platforms array (only for levels)
    if (pimpl->level.scene_type == SceneType::LEVEL) {
        jlevel["platforms"] = nlohmann::json::array();

        for (const auto &platform : pimpl->level.platforms) {
            nlohmann::json jplatform;
            jplatform["x"] = platform.tile_x;
            jplatform["y"] = platform.tile_y;
            jplatform["width"] = platform.width_tiles;
            jplatform["height"] = platform.height_tiles;

            if (!platform.texture_file.empty()) {
                jplatform["texture"] = platform.texture_file;
                if (platform.texture_tiled) {
                    jplatform["texture_tiled"] = true;
                }
                if (platform.use_atlas) {
                    jplatform["use_atlas"] = true;
                    jplatform["source_rect"] = {
                        {"x", platform.source_rect.x},
                        {"y", platform.source_rect.y},
                        {"width", platform.source_rect.width},
                        {"height", platform.source_rect.height}};
                }
            }

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
                case PlatformBehaviorType::CameraFollowVertical:
                    jplatform["behavior"] = "camera_follow_vertical";
                    break;
            }

            // Behavior parameters
            if (!platform.behavior_params.empty()) {
                jplatform["behavior_params"] = nlohmann::json::object();
                for (const auto &[key, value] : platform.behavior_params) {
                    jplatform["behavior_params"][key] = value;
                }
            }

            // Features
            if (!platform.features.empty()) {
                jplatform["features"] = nlohmann::json::array();
                for (const auto &feature : platform.features) {
                    switch (feature) {
                        case PlatformFeatureType::Spikes:
                            jplatform["features"].push_back("spikes");
                            break;
                        case PlatformFeatureType::DownwardSpikes:
                            jplatform["features"].push_back("downward_spikes");
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
    }  // end LEVEL-only platforms

    // Monsters array (only for levels)
    if (pimpl->level.scene_type == SceneType::LEVEL) {
        jlevel["monsters"] = nlohmann::json::array();
        for (const auto &monster : pimpl->level.monsters) {
            nlohmann::json jmonster;
            jmonster["x"] = monster.tile_x;
            jmonster["y"] = monster.tile_y;
            jmonster["preset_name"] = monster.preset_name;

            // Only include overrides if they're set (not -1)
            if (monster.health_override != -1) {
                jmonster["health"] = monster.health_override;
            }
            if (monster.speed_override != -1) {
                jmonster["speed"] = monster.speed_override;
            }

            jlevel["monsters"].push_back(jmonster);
        }
    }  // end LEVEL-only monsters

    // Export background data (always - used by both levels and UI screens)
    jlevel["backgrounds"] = pimpl->background_manager.to_json();

    // Export FUDs
    jlevel["huds"] = nlohmann::json::array();
    for (const auto &hud : pimpl->level.huds) {
        nlohmann::json jfud = hud;  // Uses to_json from HUDElement
        jlevel["huds"].push_back(jfud);
    }

    std::ofstream out(export_path);
    out << jlevel.dump(2);
    out.close();
    pimpl->last_export_path = export_path;

    // Show success toast
    std::filesystem::path path(export_path);
    pimpl->toast_manager.add_toast(
        "Successfully exported level at " + path.filename().string(),
        udjourney::editor::ToastType::Success,
        4.0f);
}

void Editor::import_platform_level_json(const std::string &import_path) {
    // Use ImportFromFileStrategy to load the level
    auto import_strategy = std::make_unique<ImportFromFileStrategy>(
        import_path, &pimpl->background_manager);

    // Create will handle the entire import process
    import_strategy->create(
        pimpl->level, pimpl->level.col_cnt, pimpl->level.row_cnt);

    if (!import_strategy->import_successful()) {
        std::cerr << "Import failed: " << import_strategy->get_error_message()
                  << std::endl;
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

    // Load background object presets
    std::string preset_path =
        udj::core::filesystem::get_assets_path("") + "background_presets.json";
    if (!pimpl->background_preset_manager.load_from_file(preset_path)) {
        std::cerr << "Warning: Failed to load background presets from: "
                  << preset_path << std::endl;
    }

    // Set background managers on tile panel
    pimpl->editor_panel.set_background_managers(
        &pimpl->background_manager, &pimpl->background_preset_manager);

    // Initialize with a default level size
    new_tilemap(60, 30);
}

void Editor::run() {
    while (!WindowShouldClose()) {
        // Global keyboard shortcuts (using raylib for global detection)
        if (IsKeyPressed(KEY_F1)) {
            pimpl->editor_panel.request_focus();
        }

        // Toggle grid visibility (Ctrl+G)
        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) &&
            IsKeyPressed(KEY_G)) {
            EditorSettings::instance().show_grid =
                !EditorSettings::instance().show_grid;
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
                    // Open the new level popup for user to choose template
                    pimpl->newLevelPopup.level = &pimpl->level;
                    pimpl->newLevelPopup.open();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                auto &settings = EditorSettings::instance();
                if (ImGui::MenuItem(
                        "Show Grid", "Ctrl+G", &settings.show_grid)) {
                    // Grid visibility toggled
                }

                ImGui::MenuItem("Show Background Placeable Rect",
                                nullptr,
                                &settings.show_background_placeable_rect);
                ImGui::MenuItem("Show Background Visible Rect",
                                nullptr,
                                &settings.show_background_visible_rect);
                ImGui::MenuItem(
                    "Show Background to Scene Center Hints",
                    nullptr,
                    &settings.show_background_to_scene_center_hints);
                ImGui::MenuItem(
                    "Show Tiles Hints", nullptr, &settings.show_tiles_hints);

                ImGui::Separator();

                // HUD Snap Grid submenu
                if (ImGui::BeginMenu("HUD Snap Grid")) {
                    const int snap_values[] = {1, 4, 8, 16, 32, 64};
                    const char *snap_labels[] = {
                        "1 (No Snap)", "4", "8", "16", "32", "64"};
                    for (int i = 0; i < 6; i++) {
                        if (ImGui::MenuItem(
                                snap_labels[i],
                                nullptr,
                                settings.hud_snap_grid == snap_values[i])) {
                            settings.hud_snap_grid = snap_values[i];
                        }
                    }
                    ImGui::EndMenu();
                }

                // Platform Snap Grid submenu
                if (ImGui::BeginMenu("Platform Snap Grid")) {
                    const int snap_values[] = {1, 4, 8, 16, 32, 64};
                    const char *snap_labels[] = {
                        "1 (No Snap)", "4", "8", "16", "32", "64"};
                    for (int i = 0; i < 6; i++) {
                        if (ImGui::MenuItem(
                                snap_labels[i],
                                nullptr,
                                settings.platform_snap == snap_values[i])) {
                            settings.platform_snap = snap_values[i];
                        }
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings")) {
                if (ImGui::MenuItem("Animation Preset Editor")) {
                    pimpl->animation_preset_panel.set_open(true);
                }
                if (ImGui::MenuItem("Particles Presets Editor")) {
                    pimpl->particle_preset_panel.set_open(true);
                }
                ImGui::Separator();
                // Scene Type toggle
                if (ImGui::BeginMenu("Scene Type")) {
                    if (ImGui::MenuItem(
                            "Level (Gameplay)",
                            nullptr,
                            pimpl->level.scene_type == SceneType::LEVEL)) {
                        pimpl->level.scene_type = SceneType::LEVEL;
                    }
                    if (ImGui::MenuItem(
                            "UI Screen (Menus)",
                            nullptr,
                            pimpl->level.scene_type == SceneType::UI_SCREEN)) {
                        pimpl->level.scene_type = SceneType::UI_SCREEN;
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                // Scroll Speed (for gameplay levels only)
                if (pimpl->level.scene_type == SceneType::LEVEL) {
                    ImGui::Text("Scroll Speed (Gameplay)");
                    if (ImGui::SliderFloat("##ScrollSpeed",
                                           &pimpl->level.scroll_speed,
                                           0.1f,
                                           5.0f,
                                           "%.1f px/frame")) {
                        // Value updated by ImGui
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip(
                            "Camera vertical scrolling speed during "
                            "gameplay\n"
                            "Default: 1.0 px/frame\n"
                            "Lower = slower scrolling, Higher = faster "
                            "scrolling");
                    }

                    ImGui::Separator();

                    // Gravity settings
                    ImGui::Text("Gravity");
                    if (ImGui::SliderFloat("##Gravity",
                                           &pimpl->level.physics_config.gravity,
                                           0.0f,
                                           3.0f,
                                           "%.2f acceleration")) {
                        // Value updated by ImGui
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip(
                            "Gravity acceleration per frame\n"
                            "Default: 0.5 (Player), 1.0 (Monster)\n"
                            "Lower = moon gravity, Higher = heavy gravity");
                    }

                    ImGui::Text("Terminal Velocity");
                    if (ImGui::SliderFloat(
                            "##TerminalVelocity",
                            &pimpl->level.physics_config.terminal_velocity,
                            1.0f,
                            20.0f,
                            "%.1f max speed")) {
                        // Value updated by ImGui
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip(
                            "Maximum falling speed\n"
                            "Default: 10.0\n"
                            "Lower = slower falls, Higher = faster falls");
                    }

                    ImGui::Separator();
                }
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
                            pimpl->editor_panel.set_scale(scale);
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

        // Set current level for editor panel (for scene type awareness)
        pimpl->editor_panel.set_current_level(&pimpl->level);

        pimpl->editor_panel.draw();
        pimpl->editor_panel.render_file_dialogs();

        // Render the main scene view using EditorScene
        pimpl->scene.render(pimpl->level,
                            pimpl->editor_panel,
                            &pimpl->background_manager,
                            &pimpl->background_preset_manager);

        // Render UI popups after scene
        pimpl->newLevelPopup.render();

        // Draw animation preset panel
        pimpl->animation_preset_panel.draw();
        pimpl->particle_preset_panel.draw();

        // Draw toast notifications
        pimpl->toast_manager.draw();

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
