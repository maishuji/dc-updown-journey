// Copyright 2025 Quentin Cartier
#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_MANAGERS_TEXTUREMANAGER_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_MANAGERS_TEXTUREMANAGER_HPP_

#include <string>
#include <unordered_map>

#include "raylib/raylib.h"

class TextureManager {
 public:
    static TextureManager& get_instance();

    Texture2D get_texture(const std::string& path);
    void unload_all();

 private:
    TextureManager() = default;
    ~TextureManager();

    std::unordered_map<std::string, Texture2D> textures;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_MANAGERS_TEXTUREMANAGER_HPP_
