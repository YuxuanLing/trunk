#include <iostream>
#include <vector>
#include "Complex.h"

using namespace std;
using namespace caveofprogramming;

int main() {
   Complex c(2,3),c2;
   Complex c1 = c;
   c2 = c; 

   cout << c2 << endl;

   return 0;

}


