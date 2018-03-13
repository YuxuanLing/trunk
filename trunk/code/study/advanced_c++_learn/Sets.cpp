#include <iostream>
#include <set>

using namespace std;


class Test {
   int id;
   string name;
	public:
   Test(int id, string name): id(id), name(name) {
   
   }

   void print() const {
   	 cout <<"id : "<<id << " name :" << name << endl;
   }

   bool operator < (const Test &other) const  {
   		if(id == other.id) {
			return name < other.name;
		}else {
			return id < other.id;
		}
   }
  
};


int main() {
	
	set<int> numbers;

	numbers.insert(50);
	numbers.insert(10);
	numbers.insert(20);
	numbers.insert(20);
	numbers.insert(30);

	for(set<int>::iterator it = numbers.begin(); it != numbers.end(); it++) {
		cout<< *it <<endl;
	}


	set<int>::iterator itFound = numbers.find(20);

	if(itFound != numbers.end()) {
		cout <<"Found :" << *itFound << endl;
	}

	if(numbers.count(30)) {
		cout<<"Number Found."<<endl;
	}else {
		cout<<"Number Not Found."<<endl;
	}

	set<Test> tests;

	tests.insert(Test(10, "Mike"));
	tests.insert(Test(20, "Joe"));
	tests.insert(Test(200, "Joe"));
	tests.insert(Test(30, "Sue"));

   for(set<Test>::iterator it = tests.begin(); it != tests.end(); it++) {
  		it->print();
   }
}
