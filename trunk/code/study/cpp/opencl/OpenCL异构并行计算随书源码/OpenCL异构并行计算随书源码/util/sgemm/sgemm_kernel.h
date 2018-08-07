#ifndef _SGEMM_KERNEL_H
#define _SGEMM_KERNEL_H

#include <immintrin.h>
#include <stdio.h>

template<int BM, int BK, int BN>
void sgemm_kernel(float *a, float *b, float *c) {
#define B_REG_M 2
	//12
	__m256 c_vec[B_REG_M*(BN/8)];

	for(int i = 0; i < BM; i += B_REG_M) {
		for(int k = 0; k < B_REG_M*(BN/8); k++){
			c_vec[k] = _mm256_setzero_ps();
		}

		for(int k = 0; k < BK; k++) {
			__m256 b_vec[BN/8];
			for(int jj = 0; jj < BN/8; jj++){
				b_vec[jj] = _mm256_load_ps(b+k*BN+jj*8);
			}

			for(int ii = 0; ii < B_REG_M; ii++){
				__m256 a_vec = _mm256_broadcast_ss(a+(i+ii)*BK + k);

				for(int jj = 0; jj < BN/8; jj++) {//6
					__m256 temp = _mm256_mul_ps(a_vec, b_vec[jj]);
					c_vec[ii*(BN/8)+jj] = _mm256_add_ps(temp , c_vec[ii*(BN/8)+jj]);
					//c_vec[ii*(BN/8)+jj] = _mm256_fmadd_ps(a_vec[ii], b_vec[jj], c_vec[ii*(BN/8)+jj]);
				}
			}
		}

		for(int ii = 0; ii < B_REG_M; ii++){
			for(int jj = 0; jj < BN/8; jj++){
				_mm256_store_ps(c+(i+ii)*BN+jj*8, c_vec[ii*(BN/8)+jj]);
			}
		}
	}
#undef B_REG_M
}

//40 gflops
template<int BM, int BK, int BN>
void sgemm_kernel_v1(float *a, float *b, float *c) {
#define B_REG_M 2
	//12
	__m256 c_vec[B_REG_M*(BN/8)];

	for(int i = 0; i < BM; i += B_REG_M) {
		for(int k = 0; k < B_REG_M*(BN/8); k++){
			c_vec[k] = _mm256_setzero_ps();
		}

		for(int k = 0; k < BK; k++) {
			__m256 a_vec[B_REG_M];
			for(int ii = 0; ii < B_REG_M; ii++){
				a_vec[ii] = _mm256_broadcast_ss(a+(i+ii)*BK + k);
			}
			__m256 b_vec[BN/8];
			for(int jj = 0; jj < BN/8; jj++){
				b_vec[jj] = _mm256_load_ps(b+k*BN+jj*8);
			}

			for(int jj = 0; jj < BN/8; jj++) {//6

				for(int ii = 0; ii < B_REG_M; ii++){
					__m256 temp = _mm256_mul_ps(a_vec[ii], b_vec[jj]);
					c_vec[ii*(BN/8)+jj] = _mm256_add_ps(temp , c_vec[ii*(BN/8)+jj]);
					//c_vec[ii*(BN/8)+jj] = _mm256_fmadd_ps(a_vec[ii], b_vec[jj], c_vec[ii*(BN/8)+jj]);
				}
			}
		}

		for(int ii = 0; ii < B_REG_M; ii++){
			for(int jj = 0; jj < BN/8; jj++){
				_mm256_store_ps(c+(i+ii)*BN+jj*8, c_vec[ii*(BN/8)+jj]);
			}
		}
	}
#undef B_REG_M
}

//6.5 gflops
template<int BM, int BK, int BN>
void sgemm_kernel_v0(float *a, float *b, float *c) {
	for(int i = 0; i < BM; i++) {
		for(int k = 0; k < BK; k++) {
			float amult = a[i * BK + k];
			for(int j = 0; j < BN; j++) {
				c[i * BN + j] += amult* b[k * BN + j];
			}
		}
	}
}

#endif

