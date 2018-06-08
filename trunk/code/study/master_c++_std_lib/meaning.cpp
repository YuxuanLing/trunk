#include <vector>

std::vector<int> get_a_vector();

int main()
{
    std::vector<int> v0{1, 2, 3, 4, 5};

    // The lvalue `v0` is an `std::vector` which
    // owns a dynamically-allocated buffer.

    auto v1 = v0;

    // If we instantiate a new vector `v1` and
    // initialize it with the lvalue `v0`, we are
    // forced to perform a copy of `v0`'s internal
    // buffer.

    // An alternative consists of releasing the
    // ownership of the source vector's buffer
    // and giving it away to the destination.

    // When can that be done safely? Only when
    // the source is an rvalue, as it doesn't
    // have an "identity" and it is about to
    // "expire".

    auto v2 = get_a_vector();

    // In this case, `get_a_vector()` is an rvalue
    // expression - we know that the returned
    // vector has no "identity" and is about to
    // "expire" - therefore we can give away the
    // ownership of its internal buffer instead
    // of performing a copy of all its elements.

    // Why?
    // http://en.cppreference.com/w/cpp/container/vector/vector
}

std::vector<int> get_a_vector()
{
    return {1, 2, 3, 4, 5};
}
