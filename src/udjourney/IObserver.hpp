// Copyright 2025 Quentin Cartier

#ifndef SRC_UDJOURNEY_IOBSERVER_HPP_
#define SRC_UDJOURNEY_IOBSERVER_HPP_

#include <string>

class IObserver {
 public:
    virtual ~IObserver() = default;
    virtual void on_notify(const std::string &event) = 0;
};

#endif  // SRC_UDJOURNEY_IOBSERVER_HPP_
