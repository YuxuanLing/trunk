#include <iostream>
#include <fstream>

using namespace std;

int main() {
   
	string inFileName ="test.txt";
	ifstream inFile;

	inFile.open(inFileName.c_str());

	if(inFile.is_open()) {
       string line;

	   while(!inFile.eof()) {
	   	  getline(inFile, line);
		  cout << line << endl;
	   }

	   inFile.close();

	
	} else {
		cout << "Cannot open file :" << inFileName << endl;
	}

    

	return 0;


}
