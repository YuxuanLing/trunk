//
//  main.cpp
//  GtestDeamonMac
//
//  Created by Yuxuan-Ling on 19/05/2017.
//  Copyright © 2017 Yuxuan-Ling. All rights reserved.
//


#include <iostream>
#include <string>
#include <gtest/gtest.h>

int Foo(int a, int b)
{
    if (a == 0 || b == 0)
    {
        throw "don't do that";
    }
    int c = a % b;
    if (c == 0)
        return b;
    return Foo(b, c);
}


TEST(FooTest, HandleNoneZeroInput)
{
    EXPECT_EQ(2, Foo(4, 10));
    EXPECT_EQ(6, Foo(30, 18));
}


/*
int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}
*/
