#include <iostream>
using namespace std;

namespace caveofprogramming {

class Complex {
	private:
		double real;
		double image;

	public:
		Complex();
		Complex(double real, double image);
		Complex(const Complex &otheR);
        const Complex &operator=(const Complex &other); 
		double getReal() const { return real;}
		double getImage()const { return image;}

		friend ostream &operator<<(ostream &os, const Complex &c);
		//ostream &operator<<(ostream &os);
};

}
