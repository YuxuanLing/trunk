// Matrix multiplication kernel
kernel void matrixMultiplyNaiveKernel(int M, int N, int K, const global T *a, const global T *b, global T *c){ 
	int i = get_global_id(1);
	int j = get_global_id(0);

	T v = 0;
	for (int k = 0; k < K; k++){
		v += a[i * K + k] * b[k * N + j];
	} 
	c[i*N + j] = v;
}

