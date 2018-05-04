#include<iostream>
#include<cstdint>
#include<memory>
#include<math.h>
#include "FractalCreator.h"
#include"RGB.h"
using namespace std;
using namespace fractalofprogramming;
int main()
{
	RGB rgb1(4,5,6);
	RGB rgb2(1,2,3);

	RGB rgb3 = rgb1 - rgb2;

	cout << rgb3.r << rgb3.g << rgb3.b << endl;
		
	FractalCreator fractalCreator(800,600);
    fractalCreator.run("test.bmp");
	
	cout << "Hello World !" << endl;

}
