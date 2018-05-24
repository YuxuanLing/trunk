#include <iostream>
#include <string>
#include <utility>

// Fortunately, the "pass-by-value and move"
// idiom comes to our rescue.

struct person
{
    std::string _name;

    person(std::string name) : _name{std::move(name)}
    {
    }

    void set_name(std::string name)
    {
        _name = std::move(name);
    }
};

// By taking "sink arguments" by value and
// then unconditionally moving them, we can
// achieve close-to-optimal behavior:
// * Lvalue => 1 copy + 1 move
// * Rvalue => 2 moves

// Since moves are cheap, the extra move is
// worth not having to provide 2^N overloads.

int main()
{
}
