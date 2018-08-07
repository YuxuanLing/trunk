kernel void convolutionConstantSharedUnroll(int imageOutSizeX, int imageOutSizeY, global const T* imageIn, constant T c_filter[32*32], global T* imageOut){
	T local l_pixels[(BS*BX+filterSize-1)*(BS*BY+filterSize-1)];
	int tidy = get_local_id(1);
	int y = get_global_id(1);
	int tidx = get_local_id(0);
	int x = get_global_id(0);
	int imageInSizeX = imageOutSizeX+filterSize-1;
	//center
	for(int i = 0; i < BX; i++){
		for(int j = 0; j < BY; j++){
			l_pixels[tidx+BS*i+(tidy+BS*j)*(BS*BX+filterSize-1)] = imageIn[(BS*j+y)*imageInSizeX+BS*i+x];
		}
	}
	//right
	if(tidx < filterSize-1){
		for(int j = 0; j < BY; j++){
			l_pixels[tidx+BS*BX+(tidy+BS*j)*(BS*BX+filterSize-1)] = imageIn[(y+j*BS)*imageInSizeX+x+BX*BS];
		}
	}
	if(tidy < filterSize-1){
		for(int i = 0; i < BX; i++){
			l_pixels[tidx+BS*i+(tidy+BS*BY)*(BS*BX+filterSize-1)] = imageIn[(y+BS*BY)*imageInSizeX+x+BS*i];
		}
	}
	if(tidy < filterSize-1 && tidx < filterSize-1){
		l_pixels[tidx+BS*BX+(tidy+BS*BY)*(BS*BX+filterSize-1)] = imageIn[(y+BS*BY)*imageInSizeX+BS*BX+x];
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	T sum[BX*BY] = {(T) 0};
#pragma unroll
	for(int fy = 0; fy < filterSize; fy++){
#pragma unroll
		for(int fx = 0; fx < filterSize; fx++){
			T filterItem = c_filter[fx + fy*filterSize];
#pragma unroll
			for(int i = 0; i < BX; i++){
#pragma unroll
				for(int j = 0; j < BY; j++){
					T imageItem = l_pixels[BX*tidx+i+fx + (fy+tidy*BY+j)*(BS*BX+filterSize-1)];
					sum[i+j*BX] += filterItem*imageItem;
				}
			}
		}

	}
#pragma unroll
	for(int i = 0; i < BX; i++){
#pragma unroll
		for(int j = 0; j < BY; j++){
			imageOut[x+tidx*(BX-1)+i+(y+tidy*(BY-1)+j)*imageOutSizeX] = sum[j*BX+i];	
		}
	}
}

