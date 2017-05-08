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

class C :public B, public  A{
	public:
		C(){
			cout<<"C start"<<endl;
		}
		~C(){
			cout<<"C destroy"<<endl;
		}
	private:
		B a;

};

void test_func_succee()
{
	A testA;
	B testB;
}

void test_func1_succee()
{
  C c;
}

TEST(func_test_class_success_suit, func)
{
	test_func_succee();
}

TEST(func_test_class_success_suit, func1)
{
	test_func1_succee();
}

/*
int main()
{
   //test_func();
   test_func1();
  //C c;
	
	cout<<"main end"<<endl;
	return 0;
}
*/
