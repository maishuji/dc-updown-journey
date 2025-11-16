// Copyright 2025 Quentin Cartier
#pragma once

#include <memory>
#ifdef EDITOR_TESTING

class GraphicsTestManager {
 public:
    static GraphicsTestManager& instance() {
        static GraphicsTestManager instance;
        return instance;
    }

    void init_if_needed();
    void cleanup();
    bool is_initialized() const { return initialized_; }

 private:
    GraphicsTestManager() = default;
    ~GraphicsTestManager() = default;

    bool initialized_ = false;
};

// RAII wrapper for graphics context
class GraphicsTestContext {
 public:
    GraphicsTestContext() { GraphicsTestManager::instance().init_if_needed(); }

    ~GraphicsTestContext() {
        // Don't cleanup here - let the manager handle lifecycle
    }
};

#endif  // EDITOR_TESTING
