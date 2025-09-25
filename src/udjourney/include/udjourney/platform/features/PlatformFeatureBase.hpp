#pragma once

struct PlatformFeatureBase {
    virtual int_fast8_t get_type() const { return 0; }
    virtual ~PlatformFeatureBase() = default;
    virtual void draw(const Platform&) const {}
    virtual void handle_collision(Platform&, class IActor&) const {}
};