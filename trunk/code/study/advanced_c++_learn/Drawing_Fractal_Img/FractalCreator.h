#ifndef FRACTALCREATOR_H_
#define FRACTALCREATOR_H_

#include<string>
#include"Mandelbrot.h"
#include"Bitmap.h"
#include"Zoom.h"
#include"ZoomList.h"
using namespace std;

namespace fractalofprogramming {

	class FractalCreator {
		private:
          int m_width{0};
		  int m_height{0};
		  int m_total{0};
		  unique_ptr<int[]> m_histogram;
	      unique_ptr<int[]> m_fractal;
	      Bitmap m_bitmap;
	      ZoomList m_zoomList;

	      void calculateIterations();
	      void calculateTotalIterations();
	      void drawFractal();
	      void addZoom(const Zoom &zoom);
	      void writeBitmap(string name);
		public:
	      void run(string name);
	      FractalCreator(int width,int height);
	      virtual ~FractalCreator();
	};


}

#endif
