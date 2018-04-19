#include<iostream>
#include<memory.h>
#include<vector>
using namespace std;

class Test {
	private:
		int *_pBuffer;
		static const int SIZE = 100;
	public:
		Test(){
			cout << "constructor" << endl;
			_pBuffer = new int[SIZE]{};
		   //	memset(_pBuffer, 0, sizeof(int)*SIZE);
		}

		Test(int i) {
			_pBuffer = new int[SIZE]{};

			for(int i=0; i < SIZE; i++){
				_pBuffer[i] = 7*i;
			}
			cout << "Parametrized constructor" << endl;		
		}

		Test(const Test &other) {

			_pBuffer = new int[SIZE]{};
			memcpy(_pBuffer, other._pBuffer, SIZE*sizeof(int));
			cout << "copy constructor" << endl;	
		}

		Test &operator=(const Test &other) {
			_pBuffer = new int[SIZE]{};
			memcpy(_pBuffer, other._pBuffer, SIZE*sizeof(int));
			cout << "assignment " << endl;
			return *this;
		}

		~Test(){
            delete [] _pBuffer;		
			cout << "destructor" <<endl;
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
	Test test1 = getTest();

	cout << test1;

	vector<Test> vec;
	cout<<"----------"<<endl;
    vec.push_back(Test());
	
    Test &ltest1 = test1;

    Test &&rtest1=Test();	
    Test &&rtest=getTest();	
	
    check(test1);	
    check(getTest());	
    check(Test());	
	cout<<"**********"<<endl;


	
	
	int value1 = 7;

	int *pValue1 = &value1;
	//int *pValue2 = &7;
	
	Test *pTest1 = &test1;
	//Test *pTest2 = &getTest();

    int *pValue3 = &++value1;
    cout <<*pValue3<<endl;
   // int *pValue4 = &value1++;

	return 0;

}
