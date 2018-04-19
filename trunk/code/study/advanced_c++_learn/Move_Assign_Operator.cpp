#include<iostream>
#include<memory.h>
#include<vector>
using namespace std;

class Test {
	private:
		int *_pBuffer{nullptr};
		static const int SIZE = 100;
	public:
		Test(){
		//	cout << "defaule constructor"<<endl;
			_pBuffer = new int[SIZE]{};
		}

		Test(int i) {
			_pBuffer = new int[SIZE]{};
			for(int i=0; i < SIZE; i++){
				_pBuffer[i] = 7*i;
			}
		}

		Test(const Test &other) {
			_pBuffer = new int[SIZE]{};
			memcpy(_pBuffer, other._pBuffer, SIZE*sizeof(int));
		}

		Test(Test &&other) {
			cout << "Move Constructor"<<endl;
		   _pBuffer = other._pBuffer;	
	       other._pBuffer = nullptr;	
		
		}

		Test &operator=(const Test &other) {
			_pBuffer = new int[SIZE]{};
			memcpy(_pBuffer, other._pBuffer, SIZE*sizeof(int));
			return *this;
		}

    	Test &operator=(Test &&other) {
			delete [] _pBuffer;
			_pBuffer = other._pBuffer;
			other._pBuffer = nullptr;
			cout<<"Move Assign"<<endl;
			return *this;
		}

		~Test(){
		//	cout << "destrucotr"<<endl;
            delete [] _pBuffer;		
		}

		friend ostream &operator<<(ostream &out, const Test &test);

};

ostream &operator<<(ostream &out, const Test &test) {
	out << "Hello from test"<<endl;
	return out;
}

Test getTest(){
	return Test();
}

void check(const Test &value)
{
	cout<<"lValue function"<<endl;
}
void check(Test &&value)
{
	cout<<"Rvalue function"<<endl;
}


int main(){
	vector<Test> vec;
	cout<<"----------"<<endl;
    vec.push_back(Test());
	Test test;
	test = getTest();
	

	cout<<"**********"<<endl;


	return 0;

}
