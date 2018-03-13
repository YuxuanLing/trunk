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
		
//		Person(const Person &other) {
//			name = other.name;
//			age  = other.age;
//			cout <<"Copy constructor running : "<< name << "  "<< age<<endl;
//		
//		}


		Person(string name, int age):name(name), age(age) {
		
		}

		void print() const {
		
			cout << "name :" << name << "  age: " << age <<flush;
		}


		bool operator <(const Person &other) const {
			if(name == other.name) {
				return age < other.age;
			}else {
			    return name < other.name; 
			}
		}
};


int main() {

	map<Person,int> people;
	people[Person("Mike",40)] = 40;
	people[Person("Mike",450)] = 123;
	people[Person("Vicky",30)] = 30;
	people[Person("Raj",20)] = 20;

	//people.insert(make_pair(55,Person("Bob", 45)));

	for(map<Person ,int>::iterator it = people.begin(); it != people.end();
			it++) {
		cout << it->second<< ":" << flush;	
	 	it->first.print();
		cout<<endl;
	}


}
