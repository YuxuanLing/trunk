#include <iostream>

void foo(int&)
{
    std::cout << "non-const lvalue ref\n";
}

void foo(int&&)
{
    std::cout << "non-const rvalue ref\n";
}

void bar(const int&)
{
    std::cout << "const lvalue ref\n";
}

int main()
{
    int a = 0;
    foo(a);

    foo(5);

    bar(a);
    bar(5);
}
