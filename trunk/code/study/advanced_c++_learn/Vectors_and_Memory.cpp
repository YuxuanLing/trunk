#include <iostream>
#include <vector>

using namespace std;


int main() {
	vector<double> numbers(20);

	cout <<"Size :" << numbers.size() << endl;
	int capacity = numbers.capacity();


	cout <<"Capacity: "<< capacity << endl;

	for(int i = 0; i < 1000; i++) {
		if(numbers.capacity() != capacity) {
			capacity = numbers.capacity();
			cout <<"Capacity: "<< capacity << endl;
		}

		numbers.push_back(i);
	}

	numbers.reserve(1500);
	
	cout <<"Size :" << numbers.size() << endl;

	cout <<"Capacity: "<< numbers.capacity() << endl;
}
