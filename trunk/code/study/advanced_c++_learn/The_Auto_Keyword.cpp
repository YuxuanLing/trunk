#include <iostream>
using namespace std;


template<class T, class S>
auto test(T value, S value2) -> decltype(value ){

    return value + value2;
}

int get(){
   return 999;
}


auto test2() -> decltype(get()){
    return get();
}

int main(){
    auto value = 9;
    auto text="Hello!";

    cout << value << endl;
    cout << text << endl;
    cout << test(5,7) << endl;
    cout << test2() << endl;

    return 0;
}
