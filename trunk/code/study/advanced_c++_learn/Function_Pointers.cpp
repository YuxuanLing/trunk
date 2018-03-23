#include <iostream>
using namespace std;

void test() {
    cout << "Hello" << endl;
}


int main() {

   test();
   
   void (*pTest)();

   pTest = &test;
   pTest = test;
   
   (*pTest)();
   pTest();
   

   return 0;

}
