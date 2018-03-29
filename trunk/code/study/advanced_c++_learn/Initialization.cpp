#include<iostream>
#include<vector>
using namespace std;


int main() {
	int values[]={1,4,5};


	cout << values[0] << endl;
    
	class C {
		public:
			string text;
			int id;
	};

    C c1={"hello",7};

	cout << c1.text << endl;

	struct S {
		public:
			string text;
			int id;
	};

    S s1={"hello",7};

	cout << s1.text << endl;

	struct R {
		public:
			string text;
			int id;
	} r1 = {"hello", 7};


	cout << r1.text << endl;

    vector<string> strings;
    strings.push_back("One");
    strings.push_back("Two");
    strings.push_back("Three");
    strings.push_back("Four");

	return 0;
	
}
