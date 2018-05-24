#include <utility>
#include <vector>
#include <string>

// That's where "perfect forwarding" and
// `std::forward` come into play.

class dictionary
{
private:
    std::vector<std::string> _words;

public:
    // If we take `s` as a forwarding reference,
    // we can then "forward" it to `push_back`
    // in order to achieve optimal behavior without
    // code repetition.

    template <typename T>
    void add(T&& s)
    {
        _words.push_back(std::forward<T>(s));
    }
};

// This works and it is now optimal. The only
// drawback is that `add` is now an unconstrained
// template, and might need constraints like
// `enable_if` to make it play nicely with
// overload resolution and produce better compiler
// errors.

int main()
{
    dictionary d;

    std::string s{"hello"};
    d.add(s);

    d.add(std::string{"world"});
}
