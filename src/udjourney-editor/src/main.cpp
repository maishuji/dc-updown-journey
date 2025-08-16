// Copyright 2025 Quentin Cartier
#include "udjourney-editor/Editor.hpp"
#include "udjourney-editor/strategies/level/BlankLevelStrategy.hpp"

int main() {
    Editor editor;
    editor.init();

    editor.set_level_creation_strategy(std::make_unique<BlankLevelStrategy>());

    editor.run();

    editor.shutdown();
    return 0;
}
