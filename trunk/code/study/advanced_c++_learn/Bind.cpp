#include<iostream>
#include<functional>

using namespace std;
using namespace placeholders;

class Test {
	public:
      int add(int a, int b, int c)
      {
          cout<< a << ", "<< b <<", "<< c <<endl;
          return a+b+c;
      }


};


int run(function<int(int,int)> func)
{
  return	func(7,3);
}

int main() 
{
   
  // cout << add(1,2,3) << endl;
  Test test;
  auto calculator = bind(&Test::add,test, 200,_2,_1);
  

  cout<<run(calculator)<<endl;
  //cout << calculator(10,20,30) << endl;
   
   return 0;
}
