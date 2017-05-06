#include "../inc/yyfnutil.h"


/*************************************************************************************************************************/
/*
              nx-1
	   z[i]=  sum x[j]*y[i+j] ; i=0,1,2,3,4...;
	          j=0
z: length nz
x: length nx
y: length (nx+nz)
*/
template<typename T>
__global__ void static kernel(int nx, int nz, T* x, T* y, T* z){
	extern T __shared__ s[];

	if(blockIdx.x < nz){

		T ret = (T) 0;

		for(int i = threadIdx.x; i < nx; i += blockDim.x){
			ret += x[i]*y[blockIdx.x + i];	
		}

		s[threadIdx.x] = ret;
		__syncthreads();

		for(int i = blockDim.x/2; i > 0; i /=2){
			if(threadIdx.x < i){
				s[threadIdx.x] += s[threadIdx.x+i];
			}
			__syncthreads();
		}

		if(0 == threadIdx.x)
			z[blockIdx.x] = s[0];	
	}
}


template<typename T>
void crossRelated(int nx, int nz,  T* x, T* y, T* z){
	int blockSize = 256;
	kernel<T><<<nz, blockSize, blockSize*sizeof(T)>>>(nx, nz, x, y, z);
}

template<typename T>
void runGPU(int nx, int nz, const T* x, const T* y, T* z){
	T* d_x;
	CudaMalloc((void**)&d_x, nx*sizeof(T));
	CudaMemcpy(d_x, x, nx*sizeof(T), cudaMemcpyHostToDevice);

	T* d_y;
	CudaMalloc((void**)&d_y, (nx+nz)*sizeof(T));
	CudaMemcpy(d_y, y, (nx+nz)*sizeof(T), cudaMemcpyHostToDevice);

	T* d_z;
	CudaMalloc((void**)&d_z, nz*sizeof(T));
	CudaMemcpy(d_z, z, nz*sizeof(T), cudaMemcpyHostToDevice);

	crossRelated<T>(nx, nz, d_x, d_y, d_z);

	CudaMemcpy(z, d_z, nz*sizeof(T), cudaMemcpyDeviceToHost);

	CudaFree(d_x);
	CudaFree(d_y);
	CudaFree(d_z);
}

template<typename T>
static void randomize(T *d, int size){
	for(int i = 0; i < size; i++){
		d[i] = 1.0f;
		//d[i] = 1.0f*rand()/RAND_MAX;
	}
}

int main(int argc, char *argv[]){
	int nx = 6000;
	int nz = 200;

	float *x = (float*) Malloc(nx*sizeof(float));
	randomize(x, nx);

	float *y = (float*) Malloc((nx+nz)*sizeof(float));
	randomize(y, nx+nz);

	float *z = (float*) Malloc(nz*sizeof(float));

	runGPU<float>(nx, nz, x, y, z);

	for(int i = 0; i < nz; i++){
		printf("%.5f\n", z[i]);
	}

	free(x);
	free(y);
	free(z);

	return 0;
}
