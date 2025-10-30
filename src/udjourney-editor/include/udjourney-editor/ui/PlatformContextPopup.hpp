#pragma once

struct PlatformContextPopup {
    bool show = false;
    float platform_x = 0.0f;
    float platform_y = 0.0f;
    float platform_width = 64.0f;
    float platform_height = 16.0f;

    void open(float x, float y, float width, float height) {
        platform_x = x;
        platform_y = y;
        platform_width = width;
        platform_height = height;
        show = true;
    }

    void render() { 
        /* Not yet implemented */
    }
};