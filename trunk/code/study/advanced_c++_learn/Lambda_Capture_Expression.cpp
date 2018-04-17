#include<iostream>
using namespace std;

int main(){
 int one = 1;
 int two = 2;
 int three = 3;
//capture one and two by values
 [one , two](){cout <<one<<", "<<two<<endl;}();
//capture all local variables by value
 [=](){cout <<one<<", "<<two<<endl;}();
 
 //default capture  all local var by value, but capture three by reference
 [=,&three](){three = 8; cout <<one<<", "<<two<<endl;}();
 cout<<three<<endl;
 [&](){three = 6;two = 90; cout <<one<<", "<<two<<endl;}();
 cout<<two<<","<<three<<endl;

 [&,two,three](){one= 5; cout <<one<<", "<<two<<endl;}();
 cout<<one<<endl;


}

