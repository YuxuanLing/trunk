#include<gtest/gtest.h>
#include<iostream>
using namespace std;
class A{
	public:
		A(){
			cout<<"A start"<<endl;
		}
		~A(){
			cout<<"A destroy"<<endl;
		}
};

class B{
	public:
		B(){
			cout<<"B start"<<endl;
		}
		~B(){
			cout<<"B destroy"<<endl;
		}
};

class C{
	public:
		C(){
			cout<<"C start"<<endl;
		}
		~C(){
			cout<<"C destroy"<<endl;
		}
	private:
		A a;

};

void test_func()
{
	A testA;
	B testB;
}

void test_func1()
{
  C c;

}


TEST(func_test_class_suit, func)
{
	test_func();
}

TEST(func_test_class_suit, func1)
{
	test_func1();
}
/*
int main()
{
    //test_func();
   // test_func1();
  C c;
	
	cout<<"main end"<<endl;
	return 0;
}
*/