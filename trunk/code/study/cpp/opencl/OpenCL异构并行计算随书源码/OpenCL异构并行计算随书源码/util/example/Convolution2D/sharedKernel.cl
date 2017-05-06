kernel void convolutionConstantShared(int imageOutSizeX, int imageOutSizeY, global const T* imageIn, constant T c_filter[32*32], global T* imageOut){
	T local l_pixels[(BS+filterSize-1)*(BS+filterSize-1)];
	int tidy = get_local_id(1);
	int y = get_global_id(1);
	int tidx = get_local_id(0);
	int x = get_global_id(0);
	int imageInSizeX = imageOutSizeX+filterSize-1;
	//center
	l_pixels[tidx+tidy*(BS+filterSize-1)] = imageIn[y*imageInSizeX+x];
	//right
	if(tidx < filterSize-1){
		l_pixels[tidx+BS+tidy*(BS+filterSize-1)] = imageIn[y*imageInSizeX+x+BS];
	}
	if(tidy < filterSize-1){
		l_pixels[tidx+(tidy+BS)*(BS+filterSize-1)] = imageIn[(y+BS)*imageInSizeX+x];
	}
	if(tidy < filterSize-1 && tidx < filterSize-1){
		l_pixels[tidx+BS+(tidy+BS)*(BS+filterSize-1)] = imageIn[(y+BS)*imageInSizeX+BS+x];
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	T sum = (T) 0;
#pragma unroll
	for(int fy = 0; fy < filterSize; fy++){
#pragma unroll
		for(int fx = 0; fx < filterSize; fx++){
			T filterItem = c_filter[fx + fy*filterSize];
			T imageItem = l_pixels[tidx+fx + (fy+tidy)*(BS+filterSize-1)];
			sum += filterItem*imageItem;
		}

	}
	imageOut[x+y*imageOutSizeX] = sum;	
}
