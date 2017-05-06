#include "../inc/yyfnutil.h"

int main(){
	cublasHandle_t cn;
	CublasCreate(&cn);

	int m = 256, n = 256, k = 128;
	float *A = (float*) Malloc(m*k*sizeof(float));
	for(int i = 0; i < m; i += 2){
		for(int j = 0; j < k; j++){
			A[i*k+j] = 1.0f;
		}
		for(int j = 0; j < k; j++){
			A[i*k+k+j] = 0.0f;
		}
	}

	float *b = (float*) Malloc(k*n*sizeof(float));
	for(int i = 0; i < k; i++){
		for(int j = 0; j < n; j +=2){
			b[i*n+j] = 1.0f;
			b[i*n+j+1] = 0.0f;
		}
	}

	float *y = (float*) Malloc(m*n*sizeof(float));

	float *d_A;
	CudaMalloc((void**)&d_A, m*k*sizeof(float));
	CudaMemcpy(d_A, A, m*k*sizeof(float), cudaMemcpyHostToDevice);
	float *d_b;
	CudaMalloc((void**)&d_b, k*n*sizeof(float));
	cudaMemcpy(d_b, b, k*n*sizeof(float), cudaMemcpyHostToDevice);

	float *d_y;
	CudaMalloc((void**)&d_y, m*n*sizeof(float));

	float alpha = 1.0f, beta = 0.0f;
CublasSgemm(cn, CUBLAS_OP_N, CUBLAS_OP_N, m, n, k, &alpha, d_A, k, d_b, n, &beta, d_y, n);
CudaDeviceSynchronize();

	CudaMemcpy(y, d_y, m*n*sizeof(float), cudaMemcpyDeviceToHost);

	for(int i = 0; i < m; i++){
		printf("%d ", i);
		for(int j = 0; j < n; j++)
			printf(" %d", (int)y[i*n+j]);
		printf("\n\n");
	}

	free(A);
	free(b);
	free(y);

	CudaFree(d_A);
	CudaFree(d_b);
	CudaFree(d_y);

	CublasDestroy(cn);

	return 0;
}
