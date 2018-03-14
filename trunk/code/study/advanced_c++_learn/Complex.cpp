#include "Complex.h"
#include <iostream>
using namespace std;

namespace caveofprogramming {

	Complex::Complex():real(0) , image(0) {
	}
	
	Complex::Complex(double real , double image ):real(real) , image(image) {
	}

	Complex::Complex(const Complex &other):real(other.real) , image(other.image) {
	  cout << "Copy"<<endl;	
		
	}
	
	const Complex &Complex::operator=(const Complex &other){
		real = other.real;
		image = other.image;
        cout << "Operator = " << endl;
		return *this;
	}


    ostream &operator<<(ostream &os , const Complex &c) {
    //ostream &Complex::operator<<(ostream &os) {
		os << "(" << c.getReal() << "," << c.getImage() << ")";
		return os;
	}
}
