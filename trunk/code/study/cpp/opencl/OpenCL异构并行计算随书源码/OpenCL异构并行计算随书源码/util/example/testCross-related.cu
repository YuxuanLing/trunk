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
__global__ void static kernel(int nx, int nz, const T* __restrict__ x, const T* __restrict__ y, T* __restrict__ z){
	extern T volatile __shared__ s[];

	if(blockIdx.x < nz){

		T ret = (T) 0;

		float2 *tx = (float2*)x;
		float2 r;
		r.x = 0.0f;
		r.y = 0.0f;

		if( blockIdx.x &1){
//#pragma unroll 24
			for(int i = threadIdx.x; i < (nx>>1); i += blockDim.x){
				float2 tmp = tx[i];
				r.x += tmp.x*y[blockIdx.x + (i<<1)];	
				r.y += tmp.y*y[blockIdx.x + (i<<1)+1];	
			}
		}else{
			float2 *ty = ((float2*)y);
#pragma unroll 24
			for(int i = threadIdx.x; i < (nx>>1); i += blockDim.x){
				float2 tmp = tx[i];
				float2 tmpy = ty[i+(blockIdx.x>>1)];
				r.x += tmp.x*tmpy.x;
				r.y += tmp.y*tmpy.y;
			}
		}

		ret += r.x + r.y;

		s[threadIdx.x] = ret;
		__syncthreads();

		for(int i = (blockDim.x>>1); i >= 64; i >>=1){
			if(threadIdx.x < i){
				s[threadIdx.x] += s[threadIdx.x+i];
			}
			__syncthreads();
		}

		if(threadIdx.x < 32) s[threadIdx.x] += s[threadIdx.x+32];
		if(threadIdx.x < 16) s[threadIdx.x] += s[threadIdx.x+16];
		if(threadIdx.x < 8) s[threadIdx.x] += s[threadIdx.x+8];
		if(threadIdx.x < 4) s[threadIdx.x] += s[threadIdx.x+4];
		if(threadIdx.x < 2) s[threadIdx.x] += s[threadIdx.x+2];

		if(0 == threadIdx.x)
			z[blockIdx.x] = s[0]+s[1];	
	}
}


template<typename T>
void crossRelated(int nx, int nz, const T* __restrict__ x, const T* __restrict__ y, T* __restrict__ z){
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
	int nz = 2000;

	float *x = (float*) Malloc(nx*sizeof(float));
	randomize(x, nx);

	float *y = (float*) Malloc((nx+nz)*sizeof(float));
	randomize(y, nx+nz);

	float *z = (float*) Malloc(nz*sizeof(float));

	runGPU<float>(nx, nz, x, y, z);
/*
	for(int i = 0; i < nz; i++){
		printf("%.5f\n", z[i]);
	}
*/
	free(x);
	free(y);
	free(z);

	return 0;
}
