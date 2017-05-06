#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../inc/yyfnutil.h"

int main(int argc, char *argv[]){
    float *d_rand;
    size_t len = 1000;
    CudaMalloc((void**)&d_rand, len*sizeof(float));
    
    float *rand;
    CudaMallocHost((void**)&rand, len*sizeof(float));
    
    Curand::Generator *cr = new Curand::Pseudo;
    
    cr->setOffset(5);

//    cr.setOrdering(CURAND_ORDERING_PSEUDO_BEST);
    
//    cr->generateUniform(d_rand, len);

   cr->generateNormal(d_rand, len, 0.0f, 1.0f);
    
    CudaMemcpy(rand, d_rand, len*sizeof(float), cudaMemcpyDeviceToHost);
    
    printf("mean = %f\n", mean(rand, len));
    printf("mse = %f\n", mse(rand, len));
    printf("version = %d\n", Curand::Generator::getVersion());
/*
    for(int i = 0; i < len; i++){
        printf("%f \n", rand[i]);
    }
*/
    CudaFreeHost(rand);
    CudaFree(d_rand);
	
	delete cr;
}

