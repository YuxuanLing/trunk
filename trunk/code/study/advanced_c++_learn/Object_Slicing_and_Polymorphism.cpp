#include <iostream>
using namespace std;



class Parent {

	private:
		int one;
	public:
       Parent():one(0){
	   
	   
	   }

		Parent(const Parent &other) {
		 cout << "Copy Parent" << endl;
		}
		
		virtual void print() {
			cout << "Parent" << endl;
		}

		virtual ~Parent(){
		
		}


};


class Child: public Parent {
	private:
		int two;
	public:
        Child():two(9){
		
		}

		void print() {
			cout << "child" << endl;
		}

};


int main() {

    Child  c1;
	Parent &p1 = c1;
	p1.print();

	Parent p2 = Child();
	p2.print();


	return 0;
}
