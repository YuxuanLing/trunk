#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <mutex>
using namespace std;


struct foo
{
};

void pair_and_tuple()
{
    // General-purpose utility classes such as
    // `std::pair` and `std::tuple` are move-aware.

    std::pair<foo, int> p;
    auto p_copy = p;
    auto p_move = std::move(p);
    // http://en.cppreference.com/w/cpp/utility/pair/pair

    std::tuple<foo, int, char> t;
    auto t_copy = t;
    auto t_move = std::move(t);
    // http://en.cppreference.com/w/cpp/utility/tuple/tuple

    // If the items contained in them have valid
    // move operations, they will be properly used
    // when moving the pair/tuple.
}

void containers()
{
    // Every container in C++ is move-aware in one
    // way or another. The entire container can be
    // moved to a destination, or items can be moved
    // inside the containers.

    std::vector<foo> v;
    auto v_copy = v;
    auto v_move = std::move(v);

    // Moves the temporary inside the vector.
    v.push_back(foo{});

    foo f;

    // Copies `f` inside the vector.
    v.push_back(f);

    // Moves `f` inside the vector.
    v.push_back(std::move(f));

    // http://en.cppreference.com/w/cpp/container/vector/push_back
}

void move_only()
{
    // Some classes provided by the Standard Library
    // can only be moved. These classes represent
    // "unique ownership" over a resource: copying
    // them would be nonsensical.

    std::thread t{[] { std::cout << "hello!"; }};
    // auto t_copy = t; // Does not compile.
    auto t_move = std::move(t);

    // http://en.cppreference.com/w/cpp/thread/thread/thread

    // Similarly, `unique_lock` and `unique_ptr`
    // are move-only.

    {
        std::mutex m;
        std::unique_lock<std::mutex> ul{m};

        // auto ul_copy = ul; // Does not compile.
        auto ul_move = std::move(ul);
    }

    {
        std::unique_ptr<int> up = std::make_unique<int>(1);

        // auto up_copy = up; // Does not compile.
        auto up_move = std::move(up);
    }

    // Using `std::move` is required when working
    // with these classes, as we need to express
    // the intent of "transferring ownership" to
    // the compiler.
}

int main()
{
}
