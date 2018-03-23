#include <iostream>
using namespace std;


class Animal {
	public:
		virtual void run() = 0;
		virtual void speak() = 0;

};


class Dog: public Animal {
	public:
		virtual void speak() {
			cout <<"Wang !" << endl;
		}

	//	virtual void run () {
	//	    cout <<"Wang run !" << endl;
	//	}

};



class Labrador: public Dog{
	public:
	//	virtual void speak() {
	//		cout <<"Wang !" << endl;
	//	}

		virtual void run () {
		    cout <<"Labrador run !" << endl;
		}

};







int main() {

	
	Labrador lab;
	//dog.speak();
    lab.run();
	lab.speak();

	Animal *animals[5];

	animals[0] = &lab;
	animals[0]->speak();
	

	return 0;
}
