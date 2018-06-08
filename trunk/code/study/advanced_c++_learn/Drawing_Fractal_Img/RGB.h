#ifndef RGB_H_
#define RGB_H_

namespace fractalofprogramming {

struct RGB {
		double r;
		double g;
		double b;

		RGB(double r,double g, double b);
        
		friend RGB operator - (const RGB &first, const RGB &second);  
};

}

#endif
