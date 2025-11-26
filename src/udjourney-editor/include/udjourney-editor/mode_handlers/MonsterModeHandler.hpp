// Copyright 2025 Quentin Cartier
#pragma once

#include <string>

#include "udjourney-editor/mode_handlers/IModeHandler.hpp"
#include "udjourney-editor/Level.hpp"
#include "udjourney-editor/MonsterPresetManager.hpp"

/**
 * @brief Handler for Monster edit mode
 */
class MonsterModeHandler : public IModeHandler {
 public:
    MonsterModeHandler();

    void render() override;
    void set_scale(float scale) override { scale_ = scale; }

    // Monster-specific API
    const std::string& get_selected_monster_preset() const {
        return selected_monster_preset_;
    }
    EditorMonster* get_selected_monster() const { return selected_monster_; }
    void set_selected_monster(EditorMonster* monster) {
        selected_monster_ = monster;
    }
    bool should_delete_selected_monster() const {
        return delete_selected_monster_;
    }
    void clear_delete_flag() {
        delete_selected_monster_ = false;
        selected_monster_ = nullptr;
    }
    void initialize_presets();

 private:
    float scale_ = 1.0f;
    std::string selected_monster_preset_ = "goblin";
    EditorMonster* selected_monster_ = nullptr;
    bool delete_selected_monster_ = false;

    udjourney::editor::MonsterPresetManager monster_preset_manager_;

    void render_monster_editor();
};
