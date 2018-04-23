#include<iostream>
using namespace std;

class Parent {
public:
	virtual void speak() {
           cout << "parent!" << endl;
	}

};


class Brother: public Parent {

};


class Sister: public Parent {

};


int main() 
{
  Parent parent;
  Brother brother;
  Sister sister;

  Parent *ppb = &brother;

  //Brother *pbb = dynamic_cast<Brother *>(pbb);
  Sister *pbb = reinterpret_cast<Sister *>(pbb);

  if(pbb == nullptr) {
     cout << "Invalid cast" << endl;
  }else{
     cout<< pbb << endl;
  
  }

  return 0;
}

