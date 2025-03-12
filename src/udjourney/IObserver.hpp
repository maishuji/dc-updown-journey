#ifndef IOBSERVER_HPP
#define IOBSERVER_HPP

#include <string>

class IObserver
{
public:
    virtual ~IObserver() = default;
    virtual void on_notify(const std::string &event) = 0;
};

#endif // IOBSERVER_HPP