#include <iostream>
#include <string>
#include <utility>

// Before C++11, it was common to take
// "sink arguments" as `const&` and copy.

struct person
{
    std::string _name;

    person(const std::string& name) : _name{name}
    {
    }

    void set_name(const std::string& name)
    {
        _name = name;
    }
};

int main()
{
}
