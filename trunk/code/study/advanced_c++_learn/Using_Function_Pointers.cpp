#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

bool match(string test) {

	return test.size() == 3;

}


int countStrings(vector<string> &t, bool (*match)(string test)) {
	int tally = 0;
	for(int i = 0; i < t.size(); i++) {
	   if(match(t[i])) {
	   		tally += 1;
	   }		
	}

	return tally;

}



int main() {

	vector<string> texts;
    texts.push_back("one");
    texts.push_back("two");
    texts.push_back("three");
    texts.push_back("two");
    texts.push_back("four");
    texts.push_back("two");
    texts.push_back("three");

    cout << match("one") << endl;
	cout << count_if(texts.begin(), texts.end(), match) << endl;
    cout << countStrings(texts, match);
	return 0;


}


