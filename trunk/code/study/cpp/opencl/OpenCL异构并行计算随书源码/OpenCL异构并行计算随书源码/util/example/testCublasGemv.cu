#include "../inc/yyfnutil.h"

int main(){
	cublasHandle_t cn;
	CublasCreate(&cn);

	int m = 1024, n = 256;
	float *A = (float*) Malloc(m*n*sizeof(float));
	for(int i = 0; i < m; i += 2){
		for(int j = 0; j < n; j++){
			A[i*n+j] = 1.0f;
		}
		for(int j = 0; j < n; j++){
			A[i*n+n+j] = 0.0f;
		}
	}

	float *V = (float*) Malloc(n*sizeof(float));
	for(int i = 0; i < n; i++) V[i] = 1.0f;

	float *y = (float*) Malloc(m*sizeof(float));

	float *d_A;
	CudaMalloc((void**)&d_A, m*n*sizeof(float));
	CudaMemcpy(d_A, A, m*n*sizeof(float), cudaMemcpyHostToDevice);
	float *d_V;
	CudaMalloc((void**)&d_V, n*sizeof(float));
	cudaMemcpy(d_V, V, n*sizeof(float), cudaMemcpyHostToDevice);

	float *d_y;
	CudaMalloc((void**)&d_y, m*sizeof(float));

	float alpha = 1.0f, beta = 0.0f;
	CublasSgemv(cn, CUBLAS_OP_N, m, n, &alpha, d_A, n, d_V, 1, &beta, d_y, 1);
CudaDeviceSynchronize();

	CudaMemcpy(y, d_y, m*sizeof(float), cudaMemcpyDeviceToHost);

	bool passed = true;
	for(int i = 0; i < m; i += 2){
		if(256.0f != y[i]){
			passed = false;
			printf("%d, %f\n", i, y[i]);
		}
		if(0.0f != y[i+1]){
			passed = false;
			printf("%d, %f\n", i, y[i+1]);
		}
	}

	printf("%s %s\n", __FILE__, passed?"Passed":"failed");
	free(A);
	free(V);
	free(y);

	CudaFree(d_A);
	CudaFree(d_V);
	CudaFree(d_y);

	CublasDestroy(cn);

	return 0;
}
