#include <iostream>
#include <stack>
#include <queue>

using namespace std;

class Test {
	private:
		string name;
	public:
		Test():name(""){}
		Test(string name): name(name){
		
		}

		~Test() {
		
			cout<<"Object destroyed"<<endl;
		}

		void print() {
			cout << name << endl;		
		}

};


int main() {
	//LIFO
	//stack<Test> testStack;
	//FIFO
	queue<Test> testStack;

	testStack.push(Test("Mike"));
	testStack.push(Test("Jone"));
	testStack.push(Test("Sue"));

    cout << endl;	
	/*
	 *this is invaild code! obj is destroyed
	Test &test1 = testStack.top();
	testStack.pop();    //destoryed here
	test1.print();
	*/
	
   /* 
	testStack.pop();
	Test test2 = testStack.top();
	test2.print();
   */
	while(testStack.size() > 0) {
	
		//Test &test = testStack.top();
		Test &test = testStack.front();
		test.print();
		testStack.pop();
	
	}





	return 0;
}


