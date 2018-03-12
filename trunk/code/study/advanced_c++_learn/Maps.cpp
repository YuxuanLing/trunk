#include <iostream>
#include <map>

using namespace std;

int main() {
	map<string, int> ages;

	ages["Mike"] = 40;
	ages["Raj"] = 20;
	ages["Vicky"] = 30;
	ages["Mike"] = 70;



	//cout << ages["Raj"] << endl;
	//cout << ages["Sue"] << endl;
	//pair<string,int> addToMap("Peter",100);

	ages.insert(make_pair("Peter",100));

	if(ages.find("Vicky1") != ages.end()) {
		cout << "Found Vicky" <<endl;
	}else {
	
		cout <<"key not found" << endl;
	}

    for(map<string, int>::iterator it = ages.begin(); it != ages.end(); it++) {
	
	    pair<string,int> age = *it;	
		cout << age.first <<": " << age.second << endl;
	}
    for(map<string, int>::iterator it = ages.begin(); it != ages.end(); it++) {
		cout << it->first <<": " << it->second << endl;
	}


	return 0;
}
