#include<iostream>
#include<functional>

using namespace std;
using namespace placeholders;

int add(int a, int b, int c)
{
    cout<< a << ", "<< b <<", "<< c <<", "<<endl;
    return a+b+c;

}


int main() 
{

   return 0;
}
