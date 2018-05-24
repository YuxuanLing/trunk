#include <utility>
#include <vector>
#include <string>

class dictionary
{
private:
    std::vector<std::string> _words;

public:
    // We could use the "pass-by-value and move"
    // idiom:
    void add(std::string s)
    {
        _words.push_back(std::move(s));
    }
};

// This works, but it is not optimal. An
// extra move is executed on every invocation
// of `add`.

int main()
{
    dictionary d;

    std::string s{"hello"};
    d.add(s);

    d.add(std::string{"world"});
}
