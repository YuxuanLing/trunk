// Matrix multiplication kernel 
kernel void matrixMultiplyKernel(int M, int N, int K, const global float *a, const global float *b, global float *c) { 
	int by = get_group_id(1);
	int bx = get_group_id(0);
	int tx = get_local_id(0);
	int ty = get_local_id(1);

	local float ta[BS*unroll_m][BS]; 
	local float tb[BS*unroll_n][BS]; 

	int ab = K*BS*by*unroll_m;
	int ae = ab+K;

	int bb = BS*bx*unroll_n;

	float v[unroll_m][unroll_n]; 
    for(int ii = 0; ii < unroll_m; ii++) {
        for(int jj = 0; jj < unroll_n; jj++) {
             v[ii][jj] = 0.0f;
        }
    }

int i, j;
	for(i = ab, j = bb; i < ae; i += BS, j += BS*N) {
        for(int ii = 0; ii < unroll_m; ii++) {
    		ta[ii*BS+ty][tx] = a[ii*BS*K + i+ty*K+tx]; //code1
        }

        for(int jj = 0; jj < unroll_n; jj++){
		    tb[jj*BS+ty][tx] = b[jj*BS+j+ty*N+tx]; //code2
        }

		barrier(CLK_LOCAL_MEM_FENCE);
		for (int k = 0; k < BS; k++){
            for(int ii = 0; ii < unroll_m; ii++) {
                for(int jj = 0; jj < unroll_n; jj++) {
			        v[ii][jj] += ta[ii*BS+ty][k]*tb[jj*BS+k][tx];//code3
                }
            }
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

    for(int ii = 0; ii < unroll_m; ii++) {
        for(int jj = 0; jj < unroll_n; jj++) {
	        c[BS*N*by*unroll_m + bx*BS*unroll_n + ty*N + ii*BS*N + tx+jj*BS] = v[ii][jj];//code4
        }
    }
}

