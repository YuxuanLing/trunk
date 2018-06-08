#include <utility>
#include <vector>
#include <string>

class dictionary
{
private:
    std::vector<std::string> _words;

public:
    // We could provide two overloads:
    void add(const std::string& s)
    {
        _words.push_back(s);
    }

    void add(std::string&& s)
    {
        _words.push_back(std::move(s));
    }
};

// This works as intended, but causes code
// repetition.

int main()
{
    dictionary d;

    std::string s{"hello"};
    d.add(s);

    d.add(std::string{"world"});
}
