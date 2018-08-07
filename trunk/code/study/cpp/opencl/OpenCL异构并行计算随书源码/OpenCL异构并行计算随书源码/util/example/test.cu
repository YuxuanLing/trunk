#include <stdio.h>

void __global__ kernel(int *x, int size){
	int id = blockIdx.x*blockDim.x + threadIdx.x;

	for(int i = id; i < size; i += blockDim.x*gridDim.x){
		x[i] = 1;
	}
}

void __global__ print(int * x, int size){
	int id = blockDim.x*blockIdx.x + threadIdx.x;

	for(;id < size; id += blockDim.x*gridDim.x){
		if(x[id] != 1)
		printf("%d\n", x[id]);		
	}
}

int main(){
	int size = 100000000;
	
	int *x;
	cudaMalloc((void**)&x, size*sizeof(int));

	cudaStream_t s[2];

	cudaStreamCreate(s+0);
	cudaStreamCreate(s+1);
	cudaMemsetAsync(x, 0xFF, size*sizeof(int), s[0]);

	kernel<<<100000, 32, 0, s[1]>>>(x, size);


	print<<<10000 , 32>>>(x, size);

	cudaStreamDestroy(s[0]);
	cudaStreamDestroy(s[1]);
	cudaFree(x);

	if(cudaSuccess != cudaDeviceReset()){
		printf("Error\n");
	}

	return 0;
}
