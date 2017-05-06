kernel void convolutionNaive(int imageOutSizeX, int imageOutSizeY, global T* imageIn, const global T* filter, global T* imageOut){
	int y = get_global_id(1);
	int x = get_global_id(0);
	int imageInSizeX = imageOutSizeX+filterSize-1;

	if(y < imageOutSizeY){
		if(x < imageOutSizeX){
			T sum = (T) 0;

			for(int fy = 0; fy < filterSize; fy++){
				for(int fx = 0; fx < filterSize; fx++){
					T filterItem = filter[fx + fy*filterSize];
					T imageItem = imageIn[x+fx + (fy+y)*imageInSizeX];
					sum += filterItem*imageItem;
				}
			}
			//imageOut[x+y*imageOutSizeX] = imageIn[1024*1023+1023];//sum;	
			imageOut[x+y*imageOutSizeX] = sum;	
		}
	}
}
