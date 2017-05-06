#ifndef H_CHECK_CUDA_ERROR
#define H_CHECK_CUDA_ERROR

/**
 * @file checkCUDAError.h
 * @author yyfn
 * @brief just one macro for checking cuda error
 **/ 
#ifdef __CUDACC__
#include <stdio.h>
#include <stdlib.h>
#include "../CUtil.h"

#define checkCUDAError(err) {\
	cudaError_t errno = err;\
	if(cudaSuccess != errno){\
		printError(cudaGetErrorString(errno));\
		exit(0);\
	}\
}
#endif

#endif

