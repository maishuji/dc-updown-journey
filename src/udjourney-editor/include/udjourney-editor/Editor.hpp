// Copyright 2025 Quentin Cartier
#pragma once
#include <memory>
#include <string>

#include "udjourney-editor/strategies/level/LevelCreationStrategy.hpp"

class Editor {
 public:
    Editor();
    ~Editor();

    void init();
    void run();
    void shutdown();

    void new_tilemap(int rows, int cols) noexcept;

    void set_level_creation_strategy(
        std::unique_ptr<LevelCreationStrategy> strategy);

 private:
    void export_tilemap_json(const std::string& export_path);
    void import_tilemap_json(const std::string& import_path);
    void export_platform_level_json(const std::string& export_path);
    void import_platform_level_json(const std::string& import_path);
    void export_udjourney_scene(const std::string& export_path);
    void import_udjourney_scene(const std::string& import_path);
    void draw_tiles_panel();
    void update_imgui_input();

    struct PImpl;
    std::unique_ptr<PImpl> pimpl;

#ifdef EDITOR_TESTING

 public:
    // Test-only methods to access internal state
    Level& get_test_level();
    void test_export_tilemap_json(const std::string& path) {
        export_tilemap_json(path);
    }
    void test_import_tilemap_json(const std::string& path) {
        import_tilemap_json(path);
    }
    void test_export_platform_level_json(const std::string& path) {
        export_platform_level_json(path);
    }
    void test_import_platform_level_json(const std::string& path) {
        import_platform_level_json(path);
    }
#endif
};
