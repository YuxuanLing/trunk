#include <iostream>
#include <vector>

using namespace std;

class Test {
	private:
		int id;
		string name;
	public:
		Test():id(0),name("") {
		
		}

		Test(int id, string name): id(id), name(name) {
		
		}

		void print() const {
			cout << id <<" : " << name << endl;
		}


		Test (const Test &other) {
			cout << "Copy constructor running"<<endl;
		//	id = other.id;
		//	name = other.name;
			*this = other;
		}


	 const Test	&operator = (const Test &other) {
	 	 
		 cout <<"Assignment running"<<endl;
		 id = other.id;
		 name = other.name;
	     
		 return *this;
	 }

};


int main() {
	
    Test test1(10,"Mike");
    Test test2(20,"Bob");

	test1.print();
	cout << "test1  print" << endl;
	
	test2 = test1;
	test2.print();

	cout << "test2  print" << endl;
    Test test3;
	//test3 = test2 = test1;
	
	
	test3.operator=(test2);
	test3.print();

	cout << "test3  print" << endl;


	//Copy initialization
	Test test4 = test1;
	test4.print();


	return 0;
}
