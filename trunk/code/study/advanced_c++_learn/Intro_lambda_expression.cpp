#include<iostream>
using namespace std;

void test(void (*pFunc)()) {

    pFunc();

}	


int main() {
    
   auto func=[](){  cout << "Hello" << endl;};
    
   func();

   test(func);

   test([](){  cout << "Hello Again" << endl;});
   
   return 0;
}
