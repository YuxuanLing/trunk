#include <stdio.h>
#include "../inc/yyfnutil.h"

__global__ void testMyAtomFloat(float *r){
    atomicAdd(r, 1.0f);
}

__global__ void testMyAtomFloatShared(float *r){
    unsigned int id = blockDim.x*blockIdx.x + threadIdx.x;
    __shared__ double add;
    
    if(0 == id)
    add = 0.0;
    __syncthreads();
    atomicAdd(&add, 1.0);
    
	__syncthreads();
    if(0 == id)
     *r = (float)add;
}

void test(){
    float *rg;
    cudaMalloc((void**)&rg, sizeof(float));
    cudaMemset(rg, 0, sizeof(float));
    
    testMyAtomFloat<<<1, 256>>>(rg);
    
    float r;
    cudaMemcpy(&r, rg, sizeof(float), cudaMemcpyDeviceToHost);
    
    testMyAtomFloatShared<<<1, 256>>>(rg);
    
	float r2;
    cudaMemcpy(&r2, rg, sizeof(float), cudaMemcpyDeviceToHost);
    
	if(r == 256.0f && r == r2){
		printf("Passed\n");
	}
    cudaFree(rg);

}
int main(){
    test();
}
