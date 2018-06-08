#include <type_traits>
#include <utility>
#include <vector>

int main()
{
    // Let's now analyze a different scenario:
    // we have an lvalue `v0` that we want to
    // "move" to a destination vector.

    std::vector<int> v0{1, 2, 3, 4, 5};

    // We, as developers, know that `v0` won't
    // be used anymore in the current scope and
    // that an unnecessary copy can be avoided.
    // How can we inform the compiler of our
    // intentions?

    auto v1 = std::move(v0);

    // `std::move` does precisely that: it casts
    // an existing lvalue expression to an rvalue,
    // so that the implementation of `std::vector`
    // can take advantage of the fact that `v0`
    // is about to "expire".

    // How does `std::move` work? It literally
    // is only a `static_cast` to an rvalue
    // reference.

    // http://en.cppreference.com/w/cpp/utility/move

    auto v2 = static_cast<std::vector<int>&&>(v1);

    // In the line above, we perform the same the
    // same operation `std::move` does, albeit in
    // a more verbose and less expressive manner.

    // Note that using `v0` or `v1` here would
    // lead to undefined behavior.
}

// One very important thing to understand is that
// `std::move` doesn't actually "move" anything:
// it simply casts an expression to an rvalue.

// Think of `std::move` as a way of telling the
// compiler that "we would like to move this
// particular object if possible, and we swear
// that we're not going to use it again later".

void noop_example()
{
    std::vector<int> v0{1, 2, 3, 4, 5};

    std::move(v0); // No-op.

    v0.size(); // Perfectly safe.
}

// Another thing to be aware of is that using
// `std::move` when returning from a function is
// unnecessary and sometimes detrimental, as it
// prevents RVO ("return value optimization").

std::vector<int> return_example()
{
    std::vector<int> v0{1, 2, 3, 4, 5};

    // Wrong:
    // return std::move(v0);

    // Correct:
    return v0;
}
