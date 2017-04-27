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



void test_func()
{
	A testA;
	B testB;
}

int main()
{
    test_func();

	return 0;
}
