// Copyright 2025 Quentin Cartier
#pragma once

#include <imgui.h>
#include <raylib/raylib.h>

#include <memory>
#include <string>
#include <unordered_map>

// Forward declaration
struct FUDElement;

// Forward declaration for texture loading
Texture2D load_texture_cached(const std::string& filename);

/**
 * @brief Interface for FUD element renderers in the editor
 *
 * This strategy pattern allows each FUD type to have its own custom
 * rendering logic, making the codebase more maintainable and extensible.
 */
class IFUDRenderer {
 public:
    virtual ~IFUDRenderer() = default;

    /**
     * @brief Render the FUD element content in the editor
     * @param fud The FUD element to render
     * @param draw_list ImGui draw list for rendering
     * @param fud_pos Top-left position of FUD on screen
     * @param fud_end Bottom-right position of FUD on screen
     * @param is_selected Whether this FUD is currently selected
     */
    virtual void render(const FUDElement& fud, ImDrawList* draw_list,
                        const ImVec2& fud_pos, const ImVec2& fud_end,
                        bool is_selected) = 0;

 protected:
    /**
     * @brief Helper to render background sprite for FUD element
     * @param fud The FUD element
     * @param draw_list ImGui draw list
     * @param fud_pos Top-left position
     * @param fud_end Bottom-right position
     */
    void render_background_sprite(const FUDElement& fud, ImDrawList* draw_list,
                                  const ImVec2& fud_pos, const ImVec2& fud_end);

    /**
     * @brief Helper to render foreground sprite for FUD element
     * @param fud The FUD element
     * @param draw_list ImGui draw list
     * @param fud_pos Top-left position
     * @param fud_end Bottom-right position
     */
    void render_foreground_sprite(const FUDElement& fud, ImDrawList* draw_list,
                                  const ImVec2& fud_pos, const ImVec2& fud_end);

 private:
    /**
     * @brief Helper to render a sprite with tile/center/stretch modes
     */
    void render_sprite_with_mode(const FUDElement& fud, ImDrawList* draw_list,
                                 const ImVec2& fud_pos, const ImVec2& fud_end,
                                 const std::string& sheet, int tile_size,
                                 int tile_col, int tile_row, int tile_width,
                                 int tile_height, int render_mode,
                                 unsigned char alpha);
};

/**
 * @brief Factory to manage and retrieve FUD renderers
 *
 * Singleton pattern provides centralized registration and access
 * to all FUD renderer implementations.
 */
class FUDRendererFactory {
 public:
    static FUDRendererFactory& instance();

    /**
     * @brief Register a renderer for a specific FUD type
     * @param type_id The FUD type identifier
     * @param renderer Unique pointer to the renderer implementation
     */
    void register_renderer(const std::string& type_id,
                           std::unique_ptr<IFUDRenderer> renderer);

    /**
     * @brief Get the renderer for a specific FUD type
     * @param type_id The FUD type identifier
     * @return Pointer to renderer, or nullptr if not registered
     */
    IFUDRenderer* get_renderer(const std::string& type_id);

 private:
    FUDRendererFactory() = default;
    std::unordered_map<std::string, std::unique_ptr<IFUDRenderer>> renderers_;
};
