// Copyright 2025 Quentin Cartier
#pragma once
#include <memory>
#include "udjourney-editor/strategies/level/LevelCreationStrategy.hpp"

class Editor {
 public:
    Editor();
    ~Editor();

    void init();
    void run();
    void shutdown();

    void new_tilemap(int rows, int cols) noexcept;

    void set_level_creation_strategy(std::unique_ptr<LevelCreationStrategy> strategy);

 private:
    void export_tilemap_json(const std::string& export_path);
    void import_tilemap_json(const std::string& import_path);
    void draw_tiles_panel();
    void update_imgui_input();

    struct PImpl;
    std::unique_ptr<PImpl> pimpl;
};
