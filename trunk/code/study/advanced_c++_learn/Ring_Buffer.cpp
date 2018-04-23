#include<iostream>
#include"ring.h"
using namespace std;


int main(){
    

	ring<string> textring(3);
    textring.add("one");
    textring.add("two");
    textring.add("three");
    //textring.add("four");
    //textring.add("five");

    for(int i = 0; i < textring.size(); i++) {
	
		cout << textring.get(i) << endl;
	}
   
    //C++ 98
	for(ring<string>::iterator it = textring.begin(); it!=textring.end();it++)
	{
		cout << *it << endl;
	
	}

	cout << endl << endl;
    //C++ 11 style
	for(auto value: textring) {
		cout << value << endl;
	}


	return 0;


}
