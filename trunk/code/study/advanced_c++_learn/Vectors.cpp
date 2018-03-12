#include <iostream>
#include <vector>


using namespace std;

int main() {
	//vector<string> strings(5);
	vector<string> strings;
    
	strings.push_back("one");
	strings.push_back("two");
	strings.push_back("three");


	for(int i = 0; i < strings.size(); i++) {
		cout << strings[i] << endl;
	}
    cout << strings.size()<<endl;


	for(vector<string>::iterator it = strings.begin(); it != strings.end();
			it++) {
		cout << *it <<endl;
	}
    
	vector<string>::iterator it =strings.begin();
	it += 2;

	cout << *it << endl;

}
