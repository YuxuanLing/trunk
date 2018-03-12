#include <iostream>
#include <map>

using namespace std;


class Person {

	private:
		string name;
		int age;
	public:
		Person():name(""), age(0) {
		
		}

		Person(string name, int age):name(name), age(age) {
		
		}

		void print() const {
		
			cout << "name :" << name << "  age: " << age <<endl;
		}


};


int main() {

	map<int, Person> people;

	people[50] = Person("Mike",40);
	people[1] = Person("Vicky",30);
	people[32] = Person("Raj",20);

	for(map<int ,Person>::iterator it = people.begin(); it != people.end();
			it++) {
		cout << it->first << ":" << flush;	
	 	it->second.print();
	}



}
