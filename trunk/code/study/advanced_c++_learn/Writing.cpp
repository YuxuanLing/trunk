#include <iostream>
//#include <ofstream>
#include <fstream>
using namespace std;
int main() {

	ofstream outFile;
	string outputFileName = "text.txt";

	outFile.open(outputFileName.c_str());
	
	if(outFile.is_open()) {
		outFile << "hello here" << endl;
		outFile << "123" << endl;
		outFile.close();
    }else {
		cout << "Could not create file: "<< outputFileName << endl;
	}

}
