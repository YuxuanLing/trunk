#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

std::string read_large_file(int index);
std::vector<int> get_multiples(int x);
void consume_multiples(std::map<int, std::vector<int>> m);

void moving_into_containers()
{
    // As briefly mentioned earlier, moving
    // into containers can be a huge performance
    // win.

    // Consider the case where we read large
    // files into an `std::string`, then put
    // the string inside an `std::vector`:

    std::vector<std::string> files;
    files.reserve(10);

    for(int i = 0; i < 10; ++i)
    {
        std::string s = read_large_file(i);
        // ...do some processing on `s`...
        files.push_back(std::move(s));
    }

    // What is wrong with the code above?
    // How can we improve it?
}

void moving_containers_instead_of_copying()
{
    // `std::string` is a container of characters.
    // The example we looked at above applies to
    // any other container, but sometimes it is
    // less obvious.

    // It is fairly common to have an `std::map`
    // where the value is another container.

    std::map<int, std::vector<int>> multiples_of;

    for(int i = 0; i < 100; ++i)
    {
        multiples_of[i] = get_multiples(i);
    }

    consume_multiples(std::move(multiples_of));

    // What is wrong with the code above?
    // How can we improve it?
}

void bar(const std::vector<int>& v);
void bar(std::vector<int>&& v);

int main()
{
    std::vector<int> v;
    bar(v);
}
