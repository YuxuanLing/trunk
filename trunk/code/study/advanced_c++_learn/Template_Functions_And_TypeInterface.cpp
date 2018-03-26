#include <iostream>
using namespace std;


template<class T>
void print(T n) {

 cout << "Template Version :" << n << endl;

}


void print(int v) {
 cout << "NoN-template Version :" << v << endl;
}
template<class T>
void show() {

 cout << T() << endl;

}



int main() {

	print<string>("Hello");
	print<int>(5);

	print("Hi Here");
    print(6);
    print<>(5);
    show<double>();

}


