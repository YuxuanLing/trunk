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
	 		//cout << "Copy constructor running"<<endl;
			//id = other.id;
			//name = other.name;
			*this = other;
		}

	 const Test	&operator = (const Test &other) {
	 	 
		 //cout <<"Assignment running"<<endl;
		 id = other.id;
		 name = other.name;
		 return *this;
	 }


   friend	ostream &operator << (ostream &out, const Test &t) {
	 	out << t.id << ":" << t.name;
		return out;
	 }

};

int main() {

    Test  test1(10,"Mike");

	test1.print();
	cout << test1 << endl;


	return 0;
}
