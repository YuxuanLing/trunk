// Matrix multiplication kernel 
kernel void matrixMultiplyKernel(int M, int N, int K, const global float *a, const global float4 *b, global float4 *c) { 
	int by = get_group_id(1);
	int bx = get_group_id(0);
	int tx = get_local_id(0);
	int ty = get_local_id(1);

#define unroll_m_float4  (unroll_m/4)

	local float4 ta[BS*unroll_m_float4][BS]; 
	local float4 tb[BS*unroll_n_float4][BS]; 

	int ab = unroll_m*K*BS*by;
	int ae = ab+K;

	int bb = BS*bx*unroll_n_float4;

	float4 v[unroll_m][unroll_n_float4]; 
    for(int ii = 0; ii < unroll_m; ii++) {
        for(int jj = 0; jj < unroll_n_float4; jj++) {
            v[ii][jj] = 0.0f;
        }
    }

    const int N_float4 = N/4;

int i, j;
	for(i = ab, j = bb; i < ae; i += BS, j += BS*N_float4) {
        for(int ii = 0; ii < unroll_m_float4; ii++) {
            float4 temp;
            temp.x = a[(4*ii+0)*BS*K + i+ty*K+tx]; //code1
            temp.y = a[(4*ii+1)*BS*K + i+ty*K+tx]; //code1
            temp.z = a[(4*ii+2)*BS*K + i+ty*K+tx]; //code1
            temp.w = a[(4*ii+3)*BS*K + i+ty*K+tx]; //code1
            ta[ii*BS+ty][tx] = temp;
        }

        for(int jj = 0; jj < unroll_n_float4; jj++) {
		    tb[jj*BS+ty][tx] = b[j+ty*N_float4+jj*BS+tx]; //code2
        }

		barrier(CLK_LOCAL_MEM_FENCE);
		for (int k = 0; k < BS; k++){
            for(int ii = 0; ii < unroll_m_float4; ii++) {
                for(int jj = 0; jj < unroll_n_float4; jj++) {
                    float4 temp_a = ta[ii*BS+ty][k];
                    float4 temp_b = tb[jj*BS+k][tx];
			        v[4*ii+0][jj] += temp_a.x*temp_b;//code3
			        v[4*ii+1][jj] += temp_a.y*temp_b;//code3
			        v[4*ii+2][jj] += temp_a.z*temp_b;//code3
			        v[4*ii+3][jj] += temp_a.w*temp_b;//code3
                }
            }
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

    for(int ii = 0; ii < unroll_m; ii++) {
        for(int jj = 0; jj < unroll_n_float4; jj++) {
	        c[N_float4*(BS*(ii+by*unroll_m) + ty) +  (bx*unroll_n_float4+jj)*BS+ tx] = v[ii][jj];//code4
        }
    }
}

