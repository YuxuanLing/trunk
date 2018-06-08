#include <iostream>
#include <string>
#include <utility>

// Since C++11, we can support move operations
// for "sink arguments" to prevent unnecessary
// copies.

// Here's a version of `person` with two overloads
// per constructor/member function.

struct person
{
    std::string _name;

    person(const std::string& name) : _name{name}
    {
    }

    person(std::string&& name) : _name{std::move(name)}
    {
    }

    void set_name(const std::string& name)
    {
        _name = name;
    }

    void set_name(std::string&& name)
    {
        _name = std::move(name);
    }
};

// This is optimal for the users of `person`:
// * If they pass an lvalue, it will be copied
// * If they pass an rvalue, it will be moved

// This quickly gets out of hand, as we need
// to write 2^N overloads for N arguments!

int main()
{
}
