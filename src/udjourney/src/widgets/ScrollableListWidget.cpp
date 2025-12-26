// Copyright 2025 Quentin Cartier

#include <algorithm>
#include <string>
#include <vector>

#include "udjourney/widgets/ScrollableListWidget.hpp"
#include "udjourney/ActionDispatcher.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/LevelMetadata.hpp"
#include <udj-core/Logger.hpp>
namespace udjourney {
ScrollableListWidget::ScrollableListWidget(
    const IGame& game, const udjourney::scene::HUDData& hud) :
    IWidget(game),
    selected_index_(0),
    scroll_offset_(0),
    item_height_(80),
    visible_items_(0),
    bg_color_(ColorAlpha(BLACK, 0.8f)),
    selected_color_(YELLOW),
    hover_color_(ColorAlpha(YELLOW, 0.5f)),
    locked_color_(GRAY),
    border_color_(WHITE),
    font_size_(24),
    subtitle_font_size_(16),
    padding_(10),
    border_thickness_(2),
    scroll_animation_target_(0.0f),
    scroll_animation_speed_(10.0f) {
    udjourney::Logger::info("ScrollableListWidget constructor start");

    // Calculate anchor position (same as ButtonWidget)
    const float kBaseWidth = 640.0f;
    const float kBaseHeight = 480.0f;

    float anchor_x = 0.0f;
    float anchor_y = 0.0f;

    switch (hud.anchor) {
        case udjourney::scene::HUDAnchor::TopLeft:
            anchor_x = 0;
            anchor_y = 0;
            break;
        case udjourney::scene::HUDAnchor::TopCenter:
            anchor_x = kBaseWidth / 2;
            anchor_y = 0;
            break;
        case udjourney::scene::HUDAnchor::TopRight:
            anchor_x = kBaseWidth;
            anchor_y = 0;
            break;
        case udjourney::scene::HUDAnchor::MiddleLeft:
            anchor_x = 0;
            anchor_y = kBaseHeight / 2;
            break;
        case udjourney::scene::HUDAnchor::MiddleCenter:
            anchor_x = kBaseWidth / 2;
            anchor_y = kBaseHeight / 2;
            break;
        case udjourney::scene::HUDAnchor::MiddleRight:
            anchor_x = kBaseWidth;
            anchor_y = kBaseHeight / 2;
            break;
        case udjourney::scene::HUDAnchor::BottomLeft:
            anchor_x = 0;
            anchor_y = kBaseHeight;
            break;
        case udjourney::scene::HUDAnchor::BottomCenter:
            anchor_x = kBaseWidth / 2;
            anchor_y = kBaseHeight;
            break;
        case udjourney::scene::HUDAnchor::BottomRight:
            anchor_x = kBaseWidth;
            anchor_y = kBaseHeight;
            break;
    }

    rect_ = Rectangle{anchor_x + hud.offset_x,
                      anchor_y + hud.offset_y,
                      hud.size_x,
                      hud.size_y};

    // Load custom properties
    try {
        udjourney::Logger::info("Loading scrollable list properties...");
        if (hud.properties.count("data_source")) {
            data_source_ = hud.properties.at("data_source");
            udjourney::Logger::info("data_source: %", data_source_);
        }
        if (hud.properties.count("item_action")) {
            item_action_template_ = hud.properties.at("item_action");
            udjourney::Logger::info("item_action: %", item_action_template_);
        }
        if (hud.properties.count("item_height")) {
            item_height_ = std::stoi(hud.properties.at("item_height"));
        }
        if (hud.properties.count("font_size")) {
            font_size_ = std::stoi(hud.properties.at("font_size"));
        }
        if (hud.properties.count("subtitle_font_size")) {
            subtitle_font_size_ =
                std::stoi(hud.properties.at("subtitle_font_size"));
        }
        if (hud.properties.count("visible_items")) {
            visible_items_ = std::stoi(hud.properties.at("visible_items"));
        }
    } catch (const std::exception& e) {
        udjourney::Logger::error("Error loading scrollable list properties: %",
                                 e.what());
    }

    udjourney::Logger::info("Calculating visible items...");
    // Calculate visible items from height if not specified
    if (visible_items_ == 0) {
        visible_items_ = static_cast<int>(rect_.height) / item_height_;
    }

    // Load data based on source type
    if (!data_source_.empty()) {
        udjourney::Logger::info("Loading data from source: %", data_source_);
        load_data_from_source(data_source_);
    }

    udjourney::Logger::info(
        "ScrollableListWidget created with data_source='%', items=%",
        data_source_,
        items_.size());
}

void load_data_from_level_source_(
    std::vector<ScrollableListWidget::ListItem>& items) {
    // Load levels
    auto levels = udjourney::LevelMetadata::load_all_levels();

    for (const auto& level : levels) {
        ScrollableListWidget::ListItem item;
        item.id = level.id;
        item.display_text = level.display_name;

        // Build subtitle with difficulty stars
        std::string stars(level.difficulty, '*');
        item.subtitle = stars;

        if (level.completed) {
            item.subtitle += " [COMPLETED]";
        }

        item.selectable = level.unlocked;
        item.locked = !level.unlocked;
        item.text_color = level.unlocked ? WHITE : GRAY;

        // Store level metadata
        item.metadata["filename"] = level.filename;
        item.metadata["difficulty"] = level.difficulty;
        item.metadata["completed"] = level.completed;
        item.metadata["best_time"] = level.best_time;

        items.push_back(item);
    }
}

void ScrollableListWidget::load_data_from_source(
    const std::string& source_type) {
    items_.clear();

    if (source_type == "levels") {
        load_data_from_level_source_(items_);
    } else if (source_type == "settings") {
        // Example: Settings menu
        items_ = {{"sound",
                   "Sound Volume",
                   "Adjust sound effects volume",
                   "",
                   {},
                   true,
                   false,
                   WHITE},
                  {"music",
                   "Music Volume",
                   "Adjust background music volume",
                   "",
                   {},
                   true,
                   false,
                   WHITE},
                  {"fullscreen",
                   "Fullscreen",
                   "Toggle fullscreen mode",
                   "",
                   {},
                   true,
                   false,
                   WHITE},
                  {"controls",
                   "Controls",
                   "Configure input controls",
                   "",
                   {},
                   true,
                   false,
                   WHITE},
                  {"language",
                   "Language",
                   "Change game language",
                   "",
                   {},
                   true,
                   false,
                   WHITE}};
    } else if (source_type == "scores") {
        // Example: High scores (would load from save file)
        items_ = {
            {"score1", "Player 1", "12,345 points", "", {}, false, false, GOLD},
            {"score2",
             "Player 2",
             "10,200 points",
             "",
             {},
             false,
             false,
             LIGHTGRAY},
            {"score3",
             "Player 3",
             "8,750 points",
             "",
             {},
             false,
             false,
             BROWN}};
    } else if (source_type == "custom") {
        // Use custom data provider if set
        if (data_provider_) {
            items_ = data_provider_();
        }
    }
}

void ScrollableListWidget::draw() const {
    try {
        // Widgets are drawn in screen space (no camera offset needed for UI
        // screens)
        Rectangle screen_rect = rect_;

        // Draw background
        DrawRectangleRec(screen_rect, bg_color_);
        DrawRectangleLinesEx(
            screen_rect, static_cast<float>(border_thickness_), border_color_);

        if (items_.empty()) {
            DrawText("No levels available",
                     static_cast<int>(screen_rect.x + padding_),
                     static_cast<int>(screen_rect.y + padding_ + 20),
                     font_size_,
                     WHITE);
            return;
        }

        // Draw scrollable items with manual clipping
        // We clip by only drawing items that are within the visible bounds
        int start_index = scroll_offset_ / item_height_;
        int end_index = std::min(start_index + visible_items_ + 1,
                                 static_cast<int>(items_.size()));

        for (int i = start_index; i < end_index; ++i) {
            const auto& item = items_[i];

            // Calculate item position
            float item_y = screen_rect.y + (i * item_height_) - scroll_offset_;

            // Manual clipping: Skip items that are completely outside bounds
            if (item_y + item_height_ < screen_rect.y ||
                item_y > screen_rect.y + screen_rect.height) {
                continue;
            }

            // Clip the item rectangle to the container bounds
            Rectangle item_rect = {screen_rect.x,
                                   item_y,
                                   screen_rect.width,
                                   static_cast<float>(item_height_)};

            // Adjust rectangle if partially visible at top
            if (item_rect.y < screen_rect.y) {
                float clip_amount = screen_rect.y - item_rect.y;
                item_rect.y = screen_rect.y;
                item_rect.height -= clip_amount;
            }

            // Adjust rectangle if partially visible at bottom
            if (item_rect.y + item_rect.height >
                screen_rect.y + screen_rect.height) {
                item_rect.height =
                    (screen_rect.y + screen_rect.height) - item_rect.y;
            }

            bool is_selected = (i == selected_index_);
            bool is_hovered = false;

            draw_list_item(item, item_rect, is_selected, is_hovered);
        }

        // Draw scrollbar if needed
        int total_height = items_.size() * item_height_;
        if (total_height > rect_.height) {
            float scrollbar_ratio =
                rect_.height / static_cast<float>(total_height);
            float scrollbar_height = scrollbar_ratio * rect_.height;
            float scroll_progress =
                scroll_offset_ /
                static_cast<float>(total_height - rect_.height);
            float scrollbar_y =
                screen_rect.y +
                scroll_progress * (rect_.height - scrollbar_height);

            Rectangle scrollbar = {screen_rect.x + screen_rect.width - 10,
                                   scrollbar_y,
                                   8,
                                   scrollbar_height};

            DrawRectangleRec(scrollbar, ColorAlpha(WHITE, 0.5f));
        }
    } catch (const std::exception& e) {
        udjourney::Logger::error("Exception in ScrollableListWidget::draw(): %",
                                 e.what());
    }
}

void ScrollableListWidget::draw_list_item(const ListItem& item,
                                          Rectangle item_rect, bool is_selected,
                                          bool is_hovered) const {
    Color bg = ColorAlpha(BLACK, 0.0f);
    Color text_color = item.text_color;

    if (item.locked) {
        text_color = locked_color_;
    } else if (is_selected) {
        bg = ColorAlpha(selected_color_, 0.3f);
        text_color = selected_color_;
    } else if (is_hovered) {
        bg = ColorAlpha(hover_color_, 0.2f);
    }

    // Draw item background
    DrawRectangleRec(item_rect, bg);

    if (is_selected) {
        DrawRectangleLinesEx(item_rect, 2.0f, selected_color_);
    }

    // Draw main text
    DrawText(item.display_text.c_str(),
             static_cast<int>(item_rect.x + padding_),
             static_cast<int>(item_rect.y + padding_),
             font_size_,
             text_color);

    // Draw subtitle if present
    if (!item.subtitle.empty()) {
        DrawText(item.subtitle.c_str(),
                 static_cast<int>(item_rect.x + padding_),
                 static_cast<int>(item_rect.y + padding_ + font_size_ + 5),
                 subtitle_font_size_,
                 ColorAlpha(text_color, 0.7f));
    }

    // Draw lock icon if locked
    if (item.locked) {
        DrawText("[LOCKED]",
                 static_cast<int>(item_rect.x + item_rect.width - 100),
                 static_cast<int>(item_rect.y + padding_),
                 16,
                 locked_color_);
    }
}

void ScrollableListWidget::update(float delta) {
    // Smooth scroll animation
    if (std::abs(scroll_offset_ - scroll_animation_target_) > 1.0f) {
        float diff = scroll_animation_target_ - scroll_offset_;
        scroll_offset_ +=
            static_cast<int>(diff * scroll_animation_speed_ * delta);
    } else {
        scroll_offset_ = static_cast<int>(scroll_animation_target_);
    }

    update_components(delta);
}

void ScrollableListWidget::process_input() {
    // Handled externally by Game
}

bool ScrollableListWidget::contains_point(Vector2 point) const {
    // Point is already in game coordinates from Game::update's mouse
    // transformation Just check collision directly
    return CheckCollisionPointRec(point, rect_);
}

void ScrollableListWidget::on_click() {
    // on_click is now used for Enter key activation only
    // Mouse clicks are ignored in favor of keyboard navigation
    udj::core::Logger::info(
        "ScrollableListWidget::on_click() - Enter key pressed");
    const ListItem* item = get_selected_item();
    if (!item || !item->selectable) {
        udj::core::Logger::info("No item selected or item not selectable");
        return;
    }

    udj::core::Logger::info("Loading level: %", item->display_text);

    if (!item_action_template_.empty()) {
        // Execute action if template is defined
        execute_item_action(*item);
    } else if (on_item_selected_) {
        // Otherwise use callback if set
        on_item_selected_(*item, const_cast<IGame*>(&get_game()));
    } else if (data_source_ == "levels" &&
               item->metadata.contains("filename")) {
        // Default behavior for levels
        std::string filename = item->metadata["filename"].get<std::string>();
        std::string action = "load_level:" + filename;
        ActionDispatcher::execute(action, const_cast<IGame*>(&get_game()));
    }
}

void ScrollableListWidget::execute_item_action(const ListItem& item) {
    std::string action = item_action_template_;

    // Check if item has custom action in metadata
    if (item.metadata.contains("action")) {
        action = item.metadata["action"].get<std::string>();
    } else {
        // Replace placeholders with item data
        // {id} -> item.id
        size_t pos = 0;
        while ((pos = action.find("{id}", pos)) != std::string::npos) {
            action.replace(pos, 4, item.id);
            pos += item.id.length();
        }

        // {filename} -> item.metadata["filename"]
        if (item.metadata.contains("filename")) {
            pos = 0;
            std::string filename = item.metadata["filename"].get<std::string>();
            while ((pos = action.find("{filename}", pos)) !=
                   std::string::npos) {
                action.replace(pos, 10, filename);
                pos += filename.length();
            }
        }

        // {index} -> selected_index_
        pos = 0;
        std::string index_str = std::to_string(selected_index_);
        while ((pos = action.find("{index}", pos)) != std::string::npos) {
            action.replace(pos, 7, index_str);
            pos += index_str.length();
        }
    }

    // Execute the action
    udjourney::Logger::info("ScrollableListWidget executing action: %", action);
    ActionDispatcher::execute(action, const_cast<IGame*>(&get_game()));
}

void ScrollableListWidget::on_hover() { is_hovered_ = true; }

void ScrollableListWidget::on_focus() { is_focused_ = true; }

void ScrollableListWidget::scroll_up() {
    if (selected_index_ > 0) {
        selected_index_--;

        // Adjust scroll if selection goes above visible area
        int selected_y = selected_index_ * item_height_;
        if (selected_y < scroll_offset_) {
            scroll_animation_target_ = static_cast<float>(selected_y);
        }
    }
}

void ScrollableListWidget::scroll_down() {
    if (selected_index_ < static_cast<int>(items_.size()) - 1) {
        selected_index_++;

        // Adjust scroll if selection goes below visible area
        int selected_y = (selected_index_ + 1) * item_height_;
        int viewport_bottom = scroll_offset_ + static_cast<int>(rect_.height);

        if (selected_y > viewport_bottom) {
            scroll_animation_target_ =
                static_cast<float>(selected_y - static_cast<int>(rect_.height));
        }
    }
}

void ScrollableListWidget::scroll_to(int index) {
    if (index >= 0 && index < static_cast<int>(items_.size())) {
        selected_index_ = index;

        // Center the selected item
        int center_y = (index * item_height_) -
                       (static_cast<int>(rect_.height) / 2) +
                       (item_height_ / 2);
        scroll_animation_target_ = static_cast<float>(std::max(0, center_y));
    }
}

void ScrollableListWidget::page_up() {
    selected_index_ = std::max(0, selected_index_ - visible_items_);
    scroll_animation_target_ =
        static_cast<float>(selected_index_ * item_height_);
}

void ScrollableListWidget::page_down() {
    selected_index_ = std::min(static_cast<int>(items_.size()) - 1,
                               selected_index_ + visible_items_);
    int selected_y = (selected_index_ + 1) * item_height_;
    scroll_animation_target_ =
        static_cast<float>(selected_y - static_cast<int>(rect_.height));
}

void ScrollableListWidget::set_data_provider(DataProvider provider) {
    data_provider_ = provider;
    refresh_data();
}

void ScrollableListWidget::set_item_selected_callback(
    ItemSelectedCallback callback) {
    on_item_selected_ = callback;
}

void ScrollableListWidget::refresh_data() {
    load_data_from_source(data_source_);
}

const ScrollableListWidget::ListItem* ScrollableListWidget::get_selected_item()
    const {
    if (selected_index_ >= 0 &&
        selected_index_ < static_cast<int>(items_.size())) {
        return &items_[selected_index_];
    }
    return nullptr;
}
}  // namespace udjourney
