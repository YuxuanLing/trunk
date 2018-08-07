// Matrix multiplication kernel 
kernel void matrixMultiplyKernel(int M, int N, int K, const global float *a, const global float4 *b, global float4 *c) { 
	int by = get_group_id(1);
	int bx = get_group_id(0);
	int tx = get_local_id(0);
	int ty = get_local_id(1);

	local float4 ta[BS][BS]; 
	local float4 tb[BS][BS]; 

	int ab = 4*K*BS*by;
	int ae = ab+K;

	int bb = BS*bx;

	float4 v[4]; 
    for(int ii = 0; ii < 4; ii++) {
        v[ii] = 0.0f;
    }

    const int N_float4 = N/4;

int i, j;
	for(i = ab, j = bb; i < ae; i += BS, j += BS*N_float4) {
        float4 temp;
        temp.x = a[0*BS*K + i+ty*K+tx]; //code1
        temp.y = a[1*BS*K + i+ty*K+tx]; //code1
        temp.z = a[2*BS*K + i+ty*K+tx]; //code1
        temp.w = a[3*BS*K + i+ty*K+tx]; //code1
        ta[ty][tx] = temp;

		tb[ty][tx] = b[j+ty*N_float4+tx]; //code2

		barrier(CLK_LOCAL_MEM_FENCE);
		for (int k = 0; k < BS; k++){
			v[0] += ta[ty][k].x*tb[k][tx];//code3
			v[1] += ta[ty][k].y*tb[k][tx];//code3
			v[2] += ta[ty][k].z*tb[k][tx];//code3
			v[3] += ta[ty][k].w*tb[k][tx];//code3
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

    for(int ii = 0; ii < 4; ii++) {
	    c[N_float4*(BS*(ii+by*4) + ty) +  bx*BS+ tx] = v[ii];//code4
    }
}

