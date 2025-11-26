// Copyright 2025 Quentin Cartier
#pragma once
#include <string>
#include "udjourney-editor/strategies/level/LevelCreationStrategy.hpp"

// Forward declaration
class BackgroundManager;

struct ImportFromFileStrategy : public LevelCreationStrategy {
    explicit ImportFromFileStrategy(const std::string& file_path,
                                    BackgroundManager* bg_manager = nullptr);
    ~ImportFromFileStrategy() override = default;

    void create(Level& level, int tiles_x, int tiles_y) override;

    bool import_successful() const { return import_success_; }
    const std::string& get_error_message() const { return error_message_; }

 private:
    std::string file_path_;
    BackgroundManager* bg_manager_;
    bool import_success_ = false;
    std::string error_message_;
};
