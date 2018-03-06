#include<iostream>
using namespace std;

class CanGoWrong{
	public:
		CanGoWrong() {
			char *pMemory = new char[99999999999999];
			delete [] pMemory;
		}
};


int main(){
	try {
	  CanGoWrong wrong;
	}
	catch (bad_alloc &e){
	  cout <<"Caught Excetion:" <<e.what() <<endl;
	}
    cout <<"still running "<<endl;	
	return 0;
}
