#include<iostream>

using namespace std;

class Test {

};

template<typename T>
void call(T &&arg) {

    check(forward<T>(arg));
}

void check(Test &&test) 
{
  cout << "rvalue" << endl;
}

void check(Test &test)
{
   cout <<"lValue"<<endl;

}

int main() 
{
  Test test;
 
  call(Test());
  call(test);
  auto &&t = test;


  return 0;

}
