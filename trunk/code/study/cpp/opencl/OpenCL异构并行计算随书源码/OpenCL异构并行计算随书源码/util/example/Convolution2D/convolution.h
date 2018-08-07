#ifndef H_CONVOLUTION
#define H_CONVOLUTION

#include <assert.h>
#include <stdio.h>
#include <math.h>

template<typename T>
void randInit(int len, T* data){
	for(int i = 0; i < len; i++){
		data[i] = 1.0f*rand()/RAND_MAX;
	}
}

template<typename T>
void checkResult(int len, T* d1, T* d2){
	T l1 = (T)0;
	T l2 = (T)0;
	for(int i = 0; i < len; i++){
		T diff = d1[i] - d2[i];
		l1 += fabs(diff);
		l2 += diff*diff;
	}
	l1 /= len;
	l2 /= len;
	printf("%d %e %e\n", len, (float)l1, (float)l2);
}
template<int filterSize, int BX, int BY, typename T>
void convolutionSerialBlocking(int imageInSizeX, int imageInSizeY, const T* imageIn, const T* filter, T* imageOut){
	assert(1 == filterSize % 2);

	int imageOutSizeX = imageInSizeX - filterSize + 1;
	int imageOutSizeY = imageInSizeY - filterSize + 1;
	assert(0 == imageOutSizeX % BX);
	assert(0 == imageOutSizeY % BY);

	for(int y = 0; y < imageOutSizeY; y += BY){
		for(int x = 0; x < imageOutSizeX; x += BX){
			T sum[BX*BY] = {(T) 0};
#pragma unroll
			for(int fy = 0; fy < filterSize; fy++){
#pragma unroll
				for(int fx = 0; fx < filterSize; fx++){
					T filterItem = filter[fx + fy*filterSize];
#pragma unroll
					for(int i = 0; i < BY; i++){
#pragma unroll
						for(int j = 0; j < BX; j++){
							T imageItem = imageIn[x+j+fx + (fy+y+i)*imageInSizeX];
							sum[i*BX+j] += filterItem*imageItem;
						}
					}
				}

			}
#pragma unroll
			for(int i = 0; i < BY; i++){
#pragma unroll
				for(int j = 0; j < BX; j++){
					imageOut[x+j+(i+y)*imageOutSizeX] = sum[i*BX+j];	
				}
			}
		}
	}
}

#endif
