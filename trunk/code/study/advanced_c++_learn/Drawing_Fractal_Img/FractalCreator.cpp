#include"FractalCreator.h"
#include<assert.h>
namespace fractalofprogramming{

   FractalCreator::FractalCreator(int width,int
			height):m_width(width),m_height(height),
		    m_histogram(new int[Mandelbrot::MAX_ITERATIONS]{0}),
		    m_fractal(new int[width*height]{0}),
			m_bitmap(m_width, m_height),
			m_zoomList(m_width,m_height)
	{
       
		m_zoomList.add(Zoom(m_width/2, m_height/2, 4.0/m_width));	
	}
    FractalCreator::~FractalCreator()
	{
	
	}
    void FractalCreator::calculateIterations()
	{
		for(int y = 0; y < m_height; y++)
		for(int x = 0; x < m_width; x++) {
		    pair<double,double> coords = m_zoomList.doZoom(x,y);
            
			int iterations =
				Mandelbrot::getIterations(coords.first,coords.second);
            uint8_t color =
				(uint8_t)(256*(double)iterations/Mandelbrot::MAX_ITERATIONS);

			m_fractal[y*m_width + x] = iterations;
            if(iterations != Mandelbrot::MAX_ITERATIONS) {
               m_histogram[iterations]++;
			}
		}
	}
	
	void FractalCreator::calculateTotalIterations() 
	{
       for(int i= 0 ; i < Mandelbrot::MAX_ITERATIONS; i++) {
       	m_total += m_histogram[i];
       }
	}


    void FractalCreator::drawFractal()
	{
	  	
	  for(int y = 0; y < m_height; y++)
	   for(int x = 0; x < m_width; x++) {
           
           uint8_t red = 0 , green = 0, blue = 0;
		   int iterations  =  m_fractal[y*m_width + x]; 
           int range = getRange(iterations);
           int rangeStart = m_ranges[range];

		   int rangeTotal = m_rangeTotals[range];
		   RGB &startColor = m_colors[range];
		   RGB &endColor = m_colors[range+1];
	       RGB colorDiff = endColor - startColor;
		   
		   if(iterations != Mandelbrot::MAX_ITERATIONS) {
              int totalPixels = 0;

		      for(int i = rangeStart; i <= iterations; i++){
		          totalPixels += m_histogram[i];
		      }
		     
			  red  = startColor.r + colorDiff.r*(double)totalPixels/rangeTotal;
			  green= startColor.g + colorDiff.g*(double)totalPixels/rangeTotal;
			  blue = startColor.b + colorDiff.b*(double)totalPixels/rangeTotal;

		  }
		  
		      m_bitmap.setPixel(x, y, red, green, blue);
	   }
	
	}
    void FractalCreator::addZoom(const Zoom &zoom)
	{
         m_zoomList.add(zoom);	
	}
    void FractalCreator::writeBitmap(string name)
	{
        m_bitmap.write(name);	
	}


   void FractalCreator::run(string name)
   {
      calculateIterations();
      calculateTotalIterations();
	  calculateRangeTotals();
      drawFractal();
      writeBitmap(name);
   
   }


   void FractalCreator::addRange(double rangeEnd, const RGB &rgb)
   {
       m_ranges.push_back(rangeEnd*Mandelbrot::MAX_ITERATIONS);
	   m_colors.push_back(rgb);
       if(m_bGotFirstRange) {
	      m_rangeTotals.push_back(0);
	   } 

	   m_bGotFirstRange = true;
      
   }


   void FractalCreator::calculateRangeTotals()
   {
      int rangeIndex = 0;
	  
	  for(int i = 0; i < Mandelbrot::MAX_ITERATIONS; i++)
	  {
	  	int pixels = m_histogram[i];

		if(i >= m_ranges[rangeIndex + 1]){
		  rangeIndex++;
		}
		m_rangeTotals[rangeIndex] += pixels;
	  }

   }

   int FractalCreator::getRange(int iterations) const 
   {
   	   int range = 0;

	   for(int i = 1; i < m_ranges.size(); i++) {
	   
	     range = i;
		 if(m_ranges[i] > iterations) {
		 
		   break;
		 }
	   }
   
       range--;
       assert(range > -1 && range < m_ranges.size());
	   return range;
   
   }

}
