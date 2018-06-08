#include <utility>
#include <vector>
#include <string>

// How does `std::forward` work?

void sink(int&);  // (0)
void sink(int&&); // (1)

template <typename T>
void pipe(T&& x)
{
    sink(std::forward<T>(x));
}

// You can think of `std::forward` as a
// "conditional move". When the passed
// expression is an lvalue, `std::forward`
// will return an lvalue reference.
// When the passed expression is an rvalue
// `std::forward` will return an rvalue
// reference.

// We need to explicitly pass the `T`
// template parameter to `std::forward`.
// Since `T` evaluates to `T&` for lvalues
// with forwarding references, it is the
// only way we can "inform" `std::forward`
// that the original value category of `x`
// was an lvalue.

// http://en.cppreference.com/w/cpp/utility/forward

int main()
{
}
