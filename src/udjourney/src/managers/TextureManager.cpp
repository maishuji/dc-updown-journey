// Copyright 2025 Quentin Cartier
#include "udjourney/managers/TextureManager.hpp"

#include <string>

#include <udj-core/CoreUtils.hpp>

TextureManager& TextureManager::get_instance() {
    static TextureManager instance;
    return instance;
}

Texture2D TextureManager::get_texture(const std::string& path) {
    auto iter = textures.find(path);
    if (iter != textures.end()) {
        return iter->second;
    }

    Texture2D tex =
        LoadTexture(udjourney::coreutils::get_assets_path(path).c_str());
    textures[path] = tex;
    return tex;
}

void TextureManager::unload_all() {
    for (auto& [_, tex] : textures) {
        UnloadTexture(tex);
    }
    textures.clear();
}

TextureManager::~TextureManager() { unload_all(); }
