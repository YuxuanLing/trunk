#include <stdio.h>
#include <cublas_v2.h>

#include "../inc/yyfnutil.h"

int main(){
	cublasHandle_t handle;
	CublasCreate(&handle);

	int n = 8;
	int k = n-1;

	float *A = (float*)Malloc(n*n*sizeof(float));
	for(int i = 0; i < n; i++){
		for(int j = 0; j < n-i; j++){
			A[i*n+j] = 1.0f;
		}
	}

	float *x = (float*)Malloc(n*sizeof(float));
	for(int i = 0; i < n; i++){
		//x[i] = n-i;
		x[i] = i+1;
	}

	float *d_A;
	CudaMalloc((void**)&d_A, n*n*sizeof(float));
	CudaMemcpy(d_A, A, n*n*sizeof(float), cudaMemcpyHostToDevice);

	float *d_x;
	CudaMalloc((void**)&d_x, n*sizeof(float));
	CudaMemcpy(d_x, x, n*sizeof(float), cudaMemcpyHostToDevice);
	
	cublasStbsv(handle, CUBLAS_FILL_MODE_LOWER, CUBLAS_OP_N, CUBLAS_DIAG_NON_UNIT, n, k, d_A, n, d_x, 1);

	CudaMemcpy(x, d_x, n*sizeof(float), cudaMemcpyDeviceToHost);

	for(int i = 0; i < n; i++){
		printf("%E\n", x[i]);
	}

	CublasDestroy(handle);

	CudaFree(d_x);
	CudaFree(d_A);
	free(A);
	free(x);

	return 0;
}
