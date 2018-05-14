int main()
{
    int a;

    // Lvalues can appear on the left side of
    // the built-in assignment operator:
    a = 0;

    // The address of lvalues can be taken:
    int* a_ptr = &a;

    // Lvalues can bind to lvalue references:
    int& a_ref = a;

    // Examples of lvalues:
    // (*) Name of a variable:
    a;

    // (*) "Member of object" expression:
    struct foo { int _b; };
    foo f;
    f._b;

    // (*) Function call returning lvalue reference:
    int& bar();
    &bar();
    bar() = 5;
}

int& bar() { static int i = 0; return i; }
