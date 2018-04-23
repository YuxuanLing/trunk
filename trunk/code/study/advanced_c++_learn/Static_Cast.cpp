#include<iostream>
using namespace std;



class Parent {

};

class Brother: public Parent {


};


class Sister: public Parent {

};

int main() {

	Parent parent;
	Brother brother;

	return 0;

}
