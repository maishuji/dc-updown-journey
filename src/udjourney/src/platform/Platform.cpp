// Copyright 2025 Quentin Cartier

#include "udjourney/platform/Platform.hpp"

#include <algorithm>
#include <any>
#include <cmath>
#include <map>
#include <memory>
#include <utility>

#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/managers/TextureManager.hpp"
namespace udjourney {

namespace {
void draw_texture_tiled(const Texture2D &texture, Rectangle dest, Color tint) {
    if (texture.id == 0 || dest.width <= 0.0f || dest.height <= 0.0f) {
        return;
    }

    const float tex_w = static_cast<float>(texture.width);
    const float tex_h = static_cast<float>(texture.height);
    if (tex_w <= 0.0f || tex_h <= 0.0f) {
        return;
    }

    const float right = dest.x + dest.width;
    const float bottom = dest.y + dest.height;

    for (float y = dest.y; y < bottom; y += tex_h) {
        const float remaining_h = bottom - y;
        const float draw_h = std::min(tex_h, remaining_h);

        for (float x = dest.x; x < right; x += tex_w) {
            const float remaining_w = right - x;
            const float draw_w = std::min(tex_w, remaining_w);

            Rectangle src = {0.0f, 0.0f, draw_w, draw_h};
            Rectangle dst = {x, y, draw_w, draw_h};
            DrawTexturePro(texture, src, dst, Vector2{0.0f, 0.0f}, 0.0f, tint);
        }
    }
}

void draw_rounded_rect_outline(Rectangle rect, float radius_px, float thickness,
                               Color color, int segments) {
    if (rect.width <= 0.0f || rect.height <= 0.0f) {
        return;
    }

    const float min_side = std::min(rect.width, rect.height);
    float radius = std::clamp(radius_px, 0.0f, min_side * 0.5f);
    if (radius <= 0.0f || thickness <= 0.0f) {
        DrawRectangleLinesEx(rect, thickness, color);
        return;
    }

    segments = std::max(segments, 4);

    const float x = rect.x;
    const float y = rect.y;
    const float w = rect.width;
    const float h = rect.height;

    const Vector2 tl = {x + radius, y + radius};
    const Vector2 tr = {x + w - radius, y + radius};
    const Vector2 br = {x + w - radius, y + h - radius};
    const Vector2 bl = {x + radius, y + h - radius};

    auto draw_arc = [&](Vector2 center, float start_deg, float end_deg) {
        constexpr float pi = 3.14159265358979323846f;
        const float start = start_deg * (pi / 180.0f);
        const float end = end_deg * (pi / 180.0f);
        const float step = (end - start) / static_cast<float>(segments);

        for (int i = 0; i < segments; ++i) {
            const float a0 = start + step * static_cast<float>(i);
            const float a1 = start + step * static_cast<float>(i + 1);

            const Vector2 p0 = {center.x + std::cos(a0) * radius,
                                center.y + std::sin(a0) * radius};
            const Vector2 p1 = {center.x + std::cos(a1) * radius,
                                center.y + std::sin(a1) * radius};
            DrawLineEx(p0, p1, thickness, color);
        }
    };

    // Straight edges between arcs
    DrawLineEx(
        Vector2{x + radius, y}, Vector2{x + w - radius, y}, thickness, color);
    DrawLineEx(Vector2{x + w, y + radius},
               Vector2{x + w, y + h - radius},
               thickness,
               color);
    DrawLineEx(Vector2{x + w - radius, y + h},
               Vector2{x + radius, y + h},
               thickness,
               color);
    DrawLineEx(
        Vector2{x, y + h - radius}, Vector2{x, y + radius}, thickness, color);

    // Corner arcs (degrees)
    draw_arc(tl, 180.0f, 270.0f);
    draw_arc(tr, 270.0f, 360.0f);
    draw_arc(br, 0.0f, 90.0f);
    draw_arc(bl, 90.0f, 180.0f);
}
}  // namespace

Platform::Platform(const IGame &iGame, Rectangle iRect, Color iColor,
                   bool iIsRepeatedY,
                   std::unique_ptr<PlatformReuseStrategy> reuseStrategy) :
    IActor(iGame),
    m_reuse_strategy(std::move(reuseStrategy)),
    m_rect(iRect),
    m_color(iColor),
    m_repeated_y(iIsRepeatedY),
    m_behavior(std::make_unique<StaticBehaviorStrategy>()) {}

Rectangle Platform::get_drawing_rect() const {
    auto rect = m_rect;
    const auto &game = get_game();
    // Convert to screen coordinates
    rect.x -= game.get_rectangle().x;
    rect.y -= game.get_rectangle().y;
    return rect;
}

void Platform::draw() const {
    Rectangle rect = get_drawing_rect();

    bool drew_texture = false;
    if (!m_texture_file.empty()) {
        Texture2D texture =
            TextureManager::get_instance().get_texture(m_texture_file);
        if (texture.id != 0) {
            Rectangle src = {0.0f,
                             0.0f,
                             static_cast<float>(texture.width),
                             static_cast<float>(texture.height)};
            Vector2 origin = {0.0f, 0.0f};
            if (m_texture_tiled) {
                (void)src;
                (void)origin;
                draw_texture_tiled(texture, rect, WHITE);
            } else {
                DrawTexturePro(texture, src, rect, origin, 0.0f, WHITE);
            }
            drew_texture = true;
        }
    }

    if (!drew_texture) {
        DrawRectangleRec(rect, m_color);
    } else {
        // Visually "round" textured platforms by drawing a rounded
        // outline over the texture (no shaders, no special textures).
        const float radius_px = 4.0f;
        const float thickness = 4.0f;
        const int segments = 12;

        // Adjust rect to account for outline thickness
        // Radius need ajustment to void visual artifacts
        auto rect_copy = rect;
        rect_copy.x -= thickness / 4.0f;
        rect_copy.y -= thickness / 4.0f;
        rect_copy.width += thickness / 2.0f;
        rect_copy.height += thickness / 2.0f;

        draw_rounded_rect_outline(
            rect_copy, radius_px, thickness, Fade(BLACK, 1.0f), segments);
    }

    for (const auto &feature : m_features) {
        feature->draw(*this);
    }
}

void Platform::update(float iDelta) {
    auto original_rect = m_rect;
    m_behavior->update(*this, iDelta);
    m_delta_x = m_rect.x - original_rect.x;
}

void Platform::process_input() {
    // Do nothing
}

void Platform::move(float iValX, float iValY) noexcept {
    m_rect.x += iValX;
    m_rect.y += iValY;
}

void Platform::resize(float iNewWidth, float iNewHeight) noexcept {
    if (iNewHeight > 0 && iNewWidth > 0) {
        m_rect.width = iNewWidth;
        m_rect.height = iNewHeight;
    }
}

void Platform::add_feature(std::unique_ptr<PlatformFeatureBase> feature) {
    int_fast8_t new_type = feature->get_type();
    auto it =
        std::find_if(m_features.begin(),
                     m_features.end(),
                     [new_type](const std::unique_ptr<PlatformFeatureBase> &f) {
                         return f->get_type() == new_type;
                     });
    if (it == m_features.end()) {
        m_features.push_back(std::move(feature));
    }
}
}  // namespace udjourney
