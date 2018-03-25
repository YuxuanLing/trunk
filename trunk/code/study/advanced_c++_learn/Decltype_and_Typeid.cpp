#include<iostream>
#include<typeinfo>
using namespace std;


int main(){
     string i;

     cout<<typeid(i).name()<<endl;

     cout << " Hello World!" <<endl;

     decltype(i) name="Bob";

     cout<<name<<endl;

     return 0;
}
