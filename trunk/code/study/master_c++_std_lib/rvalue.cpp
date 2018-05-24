int bar() { return 5; }

int main()
{
    // Rvalues can NOT appear on the left side of
    // the built-in assignment operator:
    5 = 0;
    bar() = 0;

    // The address of rvalues can NOT be taken:
    &5;
    &bar();

    // Rvalues do NOT bind to lvalue references:
    int& lv_ref0 = 5;
    int& lv_ref1 = bar();

    // Rvalues bind to rvalue references, introduced
    // in C++11:
    int&& rv_ref0 = 5;
    int&& rv_ref1 = bar();

    // Examples of rvalues:
    // (*) Numeric literals:
    5;
    10.33f;

    // (*) Built-in arithmetic expressions:
    5 + 10 * 3;

    // (*) Function calls returning non-references:
    bar();
}
