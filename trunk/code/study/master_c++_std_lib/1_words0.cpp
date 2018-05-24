#include <utility>
#include <vector>
#include <string>

// Let's say we are writing a class that
// represents a vector of words.

class dictionary
{
private:
    std::vector<std::string> _words;

public:
    // ???
};

// We want to provide a generic `.add` member
// function that accepts an `std::string` and
// moves it inside `_words` when possible.

int main()
{
    dictionary d;

    std::string s{"hello"};
    d.add(s); // Should copy.

    d.add(std::string{"world"}); // Should move.
}
