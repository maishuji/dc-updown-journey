// Copyright 2025 Quentin Cartier
#include <memory>

class Editor {
 public:
    Editor();
    ~Editor();

    void init();
    void run();
    void shutdown();

 private:
    void export_tilemap_json(const std::string& export_path);
    void draw_tiles_panel();
    void update_imgui_input();

    struct PImpl;
    std::unique_ptr<PImpl> pimpl;
};
