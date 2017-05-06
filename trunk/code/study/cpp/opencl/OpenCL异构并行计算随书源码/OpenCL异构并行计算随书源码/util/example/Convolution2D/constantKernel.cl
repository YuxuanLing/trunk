kernel void convolutionConstant(int imageOutSizeX, int imageOutSizeY, global const T* imageIn, constant T c_filter[32*32], global T* imageOut){
	int y = get_global_id(1);
	int x = get_global_id(0);
	int imageInSizeX = imageOutSizeX+filterSize-1;

	if(y < imageOutSizeY){
		if(x < imageOutSizeX){
			T sum = (T) 0;
#pragma unroll
			for(int fy = 0; fy < filterSize; fy++){
#pragma unroll
				for(int fx = 0; fx < filterSize; fx++){
					T filterItem = c_filter[fx + fy*filterSize];
					T imageItem = imageIn[x+fx + (fy+y)*imageInSizeX];
					sum += filterItem*imageItem;
				}

			}
			imageOut[x+y*imageOutSizeX] = sum;	
		}
	}
}
