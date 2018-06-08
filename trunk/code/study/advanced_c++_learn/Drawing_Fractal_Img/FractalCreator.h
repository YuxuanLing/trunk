#ifndef FRACTALCREATOR_H_
#define FRACTALCREATOR_H_

#include<string>
#include"Mandelbrot.h"
#include"Bitmap.h"
#include"RGB.h"
#include"Zoom.h"
#include"ZoomList.h"
#include<vector>
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

		  vector<int> m_ranges;
		  vector<int> m_rangeTotals;
		  vector<RGB> m_colors;
          bool m_bGotFirstRange{false};



	      void calculateIterations();
	      void calculateTotalIterations();
	      void calculateRangeTotals();
	      void drawFractal();
	      void writeBitmap(string name);
		public:
	      void run(string name);
		  void addRange(double rangeEnd, const RGB &rgb);
	      FractalCreator(int width,int height);
		  int getRange(int iterations) const;
	      void addZoom(const Zoom &zoom);
	      virtual ~FractalCreator();
	};


}

#endif
