// Copyright 2025 Quentin Cartier
#include "udjourney-editor/background/BackgroundPanel.hpp"
#include <cstring>
#include <cstdio>

BackgroundPanel::BackgroundPanel(
    BackgroundManager& manager, BackgroundObjectPresetManager& preset_manager) :
    manager_(manager), preset_manager_(preset_manager) {}

void BackgroundPanel::render() {
    ImGui::Begin("Background Layers");

    ImGui::Text("Layers: %zu / %d",
                manager_.get_layer_count(),
                BackgroundManager::MAX_LAYERS);
    ImGui::Separator();

    render_add_layer_controls();
    ImGui::Separator();

    render_layer_list();
    ImGui::Separator();

    render_layer_properties();
    ImGui::Separator();

    render_object_list();
    ImGui::Separator();

    render_add_object_controls();

    ImGui::End();
}

void BackgroundPanel::render_add_layer_controls() {
    ImGui::Text("Add New Layer");

    if (!manager_.can_add_layer()) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Maximum layers reached!");
        return;
    }

    ImGui::InputText("Name", new_layer_name_, sizeof(new_layer_name_));
    ImGui::SliderFloat("Parallax Factor", &new_parallax_factor_, 0.0f, 1.0f);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                       "(0.0 = static, 1.0 = moves with camera)");
    ImGui::InputInt("Depth", &new_depth_);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                       "(Lower depth = rendered first)");

    if (ImGui::Button("Add Layer")) {
        BackgroundLayer layer(
            new_layer_name_, new_parallax_factor_, new_depth_);
        if (manager_.add_layer(layer)) {
            std::snprintf(
                new_layer_name_, sizeof(new_layer_name_), "New Layer");
            new_parallax_factor_ = 0.5f;
            new_depth_ = 0;
        }
    }
}

void BackgroundPanel::render_layer_list() {
    ImGui::Text("Layer List");

    const auto& layers = manager_.get_layers();
    auto selected = manager_.get_selected_layer();

    for (size_t i = 0; i < layers.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));

        bool is_selected = selected.has_value() && selected.value() == i;

        char label[256];
        snprintf(label,
                 sizeof(label),
                 "%s (D:%d P:%.2f)",
                 layers[i].get_name().c_str(),
                 layers[i].get_depth(),
                 layers[i].get_parallax_factor());

        if (ImGui::Selectable(label, is_selected)) {
            manager_.select_layer(i);
        }

        ImGui::SameLine();
        if (ImGui::SmallButton("X")) {
            manager_.remove_layer(i);
        }

        ImGui::SameLine();
        if (ImGui::ArrowButton("##up", ImGuiDir_Up)) {
            manager_.move_layer_up(i);
        }

        ImGui::SameLine();
        if (ImGui::ArrowButton("##down", ImGuiDir_Down)) {
            manager_.move_layer_down(i);
        }

        ImGui::PopID();
    }
}

void BackgroundPanel::render_layer_properties() {
    auto selected = manager_.get_selected_layer();
    if (!selected.has_value()) {
        ImGui::Text("No layer selected");
        return;
    }

    BackgroundLayer* layer = manager_.get_layer(selected.value());
    if (!layer) return;

    ImGui::Text("Layer Properties");

    char name_buf[128];
    std::strncpy(name_buf, layer->get_name().c_str(), sizeof(name_buf) - 1);
    name_buf[sizeof(name_buf) - 1] = '\0';
    if (ImGui::InputText("Layer Name", name_buf, sizeof(name_buf))) {
        layer->set_name(name_buf);
    }

    char texture_buf[256];
    std::strncpy(texture_buf,
                 layer->get_texture_file().c_str(),
                 sizeof(texture_buf) - 1);
    texture_buf[sizeof(texture_buf) - 1] = '\0';
    if (ImGui::InputText("Texture File", texture_buf, sizeof(texture_buf))) {
        layer->set_texture_file(texture_buf);
    }

    float parallax = layer->get_parallax_factor();
    if (ImGui::SliderFloat("Parallax", &parallax, 0.0f, 1.0f)) {
        layer->set_parallax_factor(parallax);
    }

    int depth = layer->get_depth();
    if (ImGui::InputInt("Depth", &depth)) {
        layer->set_depth(depth);
    }

    ImGui::Text("Objects: %zu", layer->get_objects().size());
}

void BackgroundPanel::render_object_list() {
    auto selected = manager_.get_selected_layer();
    if (!selected.has_value()) {
        return;
    }

    BackgroundLayer* layer = manager_.get_layer(selected.value());
    if (!layer) return;

    ImGui::Text("Objects in Layer");

    const auto& objects = layer->get_objects();
    for (size_t i = 0; i < objects.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));

        ImGui::Text("%zu: %s (%.1f, %.1f) x%.2f",
                    i,
                    objects[i].sprite_name.c_str(),
                    objects[i].x,
                    objects[i].y,
                    objects[i].scale);

        ImGui::SameLine();
        if (ImGui::SmallButton("Remove")) {
            manager_.remove_object(selected.value(), i);
        }

        ImGui::PopID();
    }
}

void BackgroundPanel::render_add_object_controls() {
    auto selected = manager_.get_selected_layer();
    if (!selected.has_value()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                           "Select a layer to add objects");
        return;
    }

    ImGui::Text("Add Object to Selected Layer");

    // Preset selector
    if (preset_manager_.has_presets()) {
        const auto& presets = preset_manager_.get_presets();

        if (ImGui::BeginCombo("Preset",
                              presets[selected_preset_idx_].name.c_str())) {
            for (size_t i = 0; i < presets.size(); ++i) {
                bool is_selected =
                    (selected_preset_idx_ == static_cast<int>(i));
                if (ImGui::Selectable(presets[i].name.c_str(), is_selected)) {
                    selected_preset_idx_ = static_cast<int>(i);
                    // Update scale to preset default
                    new_object_scale_ = presets[i].default_scale;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    } else {
        ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "No presets loaded");
    }

    ImGui::InputFloat("X", &new_object_x_);
    ImGui::InputFloat("Y", &new_object_y_);
    ImGui::SliderFloat("Scale", &new_object_scale_, 0.1f, 5.0f);

    if (ImGui::Button("Add Object from Preset")) {
        if (preset_manager_.has_presets() && selected_preset_idx_ >= 0 &&
            selected_preset_idx_ <
                static_cast<int>(preset_manager_.get_presets().size())) {
            const auto& preset =
                preset_manager_.get_presets()[selected_preset_idx_];

            BackgroundObject obj;
            obj.sprite_name = preset.name;
            obj.x = new_object_x_;
            obj.y = new_object_y_;
            obj.scale = new_object_scale_;
            obj.rotation = 0.0f;

            // Set preset tile information
            obj.sprite_sheet = preset.sprite_sheet;
            obj.tile_size = preset.tile_size;
            obj.tile_row = preset.tile_row;
            obj.tile_col = preset.tile_col;

            manager_.add_object(selected.value(), obj);

            new_object_x_ = 0.0f;
            new_object_y_ = 0.0f;
            new_object_scale_ = preset.default_scale;
        }
    }
}
