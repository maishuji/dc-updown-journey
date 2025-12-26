// Copyright 2025 Quentin Cartier
#pragma once
namespace udjourney {
class PlatformReuseStrategy {
 public:
    virtual ~PlatformReuseStrategy() = default;
    virtual void reuse(class Platform& platform) = 0;
};
}  // namespace udjourney
