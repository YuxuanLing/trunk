#include <iostream>
#include <list>

using namespace std;

int main() {

	list<int> numbers;

	numbers.push_back(1);
	numbers.push_back(2);
	numbers.push_back(3);
	numbers.push_front(0);
    
	list<int>::iterator it = numbers.begin();
    it++;
	numbers.insert(it,100);
	cout << "Element :" << *it << endl;

    list<int>::iterator eraseIt = numbers.begin();
    eraseIt++;
	eraseIt = numbers.erase(eraseIt);
	cout << "Element :" << *eraseIt << endl;

	for(list<int>::iterator it = numbers.begin(); it != numbers.end(); it++) {
	
			cout << *it << endl;
	
	}
	
	cout <<"-----------------------------------------------------"<<endl;
	for(list<int>::iterator it = numbers.begin(); it != numbers.end();) {
		cout << *it << endl << flush;
		if(*it == 2) {
			numbers.insert(it, 9);
		}
		if(*it == 1) {
			it = numbers.erase(it);
		}else {
			it++;
		}

		//it++; //be careful about this
	
	}
	

	cout <<"-----------------------------------------------------"<<endl;
    for(list<int>::iterator it_after = numbers.begin(); it_after !=
			numbers.end(); it_after++) {
			cout << *it_after << endl;
	}
	return 0;
}
