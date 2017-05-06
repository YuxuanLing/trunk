/**
 * @file
 * @author yyfn
 * @brief wrap for cublas
 **/ 

#ifdef __CUDACC__
#include <cublas_v2.h>
#include <cublas_api.h>
#include "../CUtil.h"

#define checkCublasError(errno) {\
	if(CUBLAS_STATUS_SUCCESS == errno){}\
	else if(CUBLAS_STATUS_NOT_INITIALIZED == errno){\
		printError("The CUBLAS library was not initialized.This is usually caused by the lack of a prior cublasCreate() call, an error in the CUDA Runtime API called by the CUBLAS routine,or an error in the hardware setup.\nTo correct: call cublasCreate() prior to the function call;and check that the hardware, an appropriate version of the driver,and the CUBLAS library are correctly installed.\n");\
		exit(0);\
	}else if(CUBLAS_STATUS_ALLOC_FAILED == errno){\
		printError("Resource allocation failed inside the CUBLAS library.usually caused by a cudaMalloc() This is failure.\nTo correct: prior to the function call,deallocate previously allocated memory as much as possible.\n");\
		exit(0);\
	}else if(CUBLAS_STATUS_INVALID_VALUE == errno){\
		printError("An unsupported value or parameter was passed to the function(a negative vector size, for example).\nTo correct: ensure that all the parameters being passed have valid values.\n");\
	}else if(CUBLAS_STATUS_ARCH_MISMATCH == errno){\
		printError("The function requires a feature absent from the device architecture;usually caused by the lack of support for double precision.\nTo correct: compile and run the application on a device with appropriate compute capability,which is 1.3 for double precision.\n");\
	}else if(CUBLAS_STATUS_MAPPING_ERROR == errno){\
		printError("An access to GPU memory space failed, which is usually caused by a failure to bind a texture. \nTo correct:prior to the function call, unbind any previously bound textures.\n");\
	}else if(CUBLAS_STATUS_EXECUTION_FAILED == errno){\
		printError("The GPU program failed to execute. This is often caused by a launch failure of the kernel on the GPU, which can be caused by multiple reasons.\nTo correct: check that the hardware, an appropriate version of the driver, and the CUBLAS library are correctly installed.\n");\
	}else if(CUBLAS_STATUS_INTERNAL_ERROR == errno){\
		printError("An internal CUBLAS operation failed. caused by a cudaMemcpyAsync() This error is usually failure. \nTo correct: check that the hardware, an appropriate version of the driver, and the CUBLAS library are correctly installed.Also,check that the memory passed as a parameter to the routine is not being deallocated prior to the routine's completion.\n");\
	}\
}

#define CublasCreate(handler) checkCublasError(cublasCreate(handler))
#define CublasDestroy(handler) checkCublasError(cublasDestroy(handler))

#define CublasGetVersion(handler, version) checkCublasError(cublasGetVersion(handler, version))

#define CublasSetStream(handler, streamid) checkCublasError(cublasSetStream(handler, streamid))
#define CublasGetStream(handler, streamid) checkCublasError(cublasGetStream(handler, streamid))

//blas1

#define CublasIsamax(handler, n, x, incx, result) checkCublasError(cublasIsamax(handler, n, x, incx, result))
#define CublasIdamax(handler, n, x, incx, result) checkCublasError(cublasIdamax(handler, n, x, incx, result))
#define CublasIcamax(handler, n, x, incx, result) checkCublasError(cublasIdamax(handler, n, x, incx, result))
#define CublasIzamax(handler, n, x, incx, result) checkCublasError(cublasIdamax(handler, n, x, incx, result))

#define CublasSasum(handler, n, x, incx, result) checkCublasError(cublasSasum(handler, n, x, incx, result))
#define CublasDasum(handler, n, x, incx, result) checkCublasError(cublasDasum(handler, n, x, incx, result))
#define CublasCasum(handler, n, x, incx, result) checkCublasError(cublasScasum(handler, n, x, incx, result))
#define CublasZasum(handler, n, x, incx, result) checkCublasError(cublasDzasum(handler, n, x, incx, result))

#define CublasSaxpy(handler, n, alpha, x, incx, y, incy) checkCublasError(cublasSaxpy(handler, n, alpha, x, incx, y, incy))
#define CublasDaxpy(handler, n, alpha, x, incx, y, incy) checkCublasError(cublasDaxpy(handler, n, alpha, x, incx, y, incy))
#define CublasCaxpy(handler, n, alpha, x, incx, y, incy) checkCublasError(cublasCaxpy(handler, n, alpha, x, incx, y, incy))
#define CublasZaxpy(handler, n, alpha, x, incx, y, incy) checkCublasError(cublasZaxpy(handler, n, alpha, x, incx, y, incy))

#define CublasSdot(handler, n, x, incx, y, incy, result) checkCublasError(cublasSdot(handler, n, x, incx, y, incy, result))
#define CublasDdot(handler, n, x, incx, y, incy, result) checkCublasError(cublasDdot(handler, n, x, incx, y, incy, result))
#define CublasCdot(handler, n, x, incx, y, incy, result) checkCublasError(cublasCdotu(handler, n, x, incx, y, incy, result))
#define CublasCdotc(handler, n, x, incx, y, incy, result) checkCublasError(cublasCdotc(handler, n, x, incx, y, incy, result))
#define CublasZdot(handler, n, x, incx, y, incy, result) checkCublasError(cublasZdotu(handler, n, x, incx, y, incy, result))
#define CublasZdotc(handler, n, x, incx, y, incy, result) checkCublasError(cublasZdotc(handler, n, x, incx, y, incy, result))

#define CublasSnrm2(handler, n, x, incx, result) checkCublasError(cublasSnrm2(handler, n, x, incx, result))
#define CublasDnrm2(handler, n, x, incx, result) checkCublasError(cublasDnrm2(handler, n, x, incx, result))
#define CublasCnrm2(handler, n, x, incx, result) checkCublasError(cublasScnrm2(handler, n, x, incx, result))
#define CublasZnrm2(handler, n, x, incx, result) checkCublasError(cublasDznrm2(handler, n, x, incx, result))

#define CublasSrot(handler, n, x, incx, y, incy, c, s) checkCublasError(cublasSrot(handler, n, x, incx, y, incy, c, s))
#define CublasDrot(handler, n, x, incx, y, incy, c, s) checkCublasError(cublasDrot(handler, n, x, incx, y, incy, c, s))
#define CublasCrot(handler, n, x, incx, y, incy, c, s) checkCublasError(cublasCrot(handler, n, x, incx, y, incy, c, s))
#define CublasCsrot(handler, n, x, incx, y, incy, c, s) checkCublasError(cublasCsrot(handler, n, x, incx, y, incy, c, s))
#define CublasZrot(handler, n, x, incx, y, incy, c, s) checkCublasError(cublasZrot(handler, n, x, incx, y, incy, c, s))
#define CublasZdrot(handler, n, x, incx, y, incy, c, s) checkCublasError(cublasZdrot(handler, n, x, incx, y, incy, c, s))

#define CublasSrotg(handler, a, b, c, s) checkCublasError(cublasSrotg(handler, a, b, c, s))
#define CublasDrotg(handler, a, b, c, s) checkCublasError(cublasDrotg(handler, a, b, c, s))
#define CublasCrotg(handler, a, b, c, s) checkCublasError(cublasCrotg(handler, a, b, c, s))
#define CublasZrotg(handler, a, b, c, s) checkCublasError(cublasZrotg(handler, a, b, c, s))

#define CublasSrotmg(handler, d1, d2, x1, y1, p) checkCublasError(cublasSrotmg(handler, d1, d2, x1, y1, p))
#define CublasDrotmg(handler, d1, d2, x1, y1, p) checkCublasError(cublasDrotmg(handler, d1, d2, x1, y1, p))

#define CublasSscal(handler, n, alpha, x, incx) checkCublasError(cublasSscal(handler, n, alpha, x, incx))
#define CublasDscal(handler, n, alpha, x, incx) checkCublasError(cublasDscal(handler, n, alpha, x, incx))
#define CublasCscal(handler, n, alpha, x, incx) checkCublasError(cublasCscal(handler, n, alpha, x, incx))
#define CublasCsscal(handler, n, alpha, x, incx) checkCublasError(cublasCsscal(handler, n, alpha, x, incx))
#define CublasZscal(handler, n, alpha, x, incx) checkCublasError(cublasZscal(handler, n, alpha, x, incx))
#define CublasZdscal(handler, n, alpha, x, incx) checkCublasError(cublasZdscal(handler, n, alpha, x, incx))

#define CublasSswap(handler, n, x, incx, y, incy) checkCublasError(cublasSswap(handler, n, x, incx, y, incy))
#define CublasDswap(handler, n, x, incx, y, incy) checkCublasError(cublasDswap(handler, n, x, incx, y, incy))
#define CublasCswap(handler, n, x, incx, y, incy) checkCublasError(cublasCswap(handler, n, x, incx, y, incy))
#define CublasZswap(handler, n, x, incx, y, incy) checkCublasError(cublasZswap(handler, n, x, incx, y, incy))

//blas2 all matrix are row-first not column first
//#define CublasSgbmv cublasSgbmv(cublasHandle_t handle, 
//cublasStatus_t cublasDgbmv(cublasHandle_t handle, cublasOperation_t
//cublasStatus_t cublasCgbmv(cublasHandle_t handle, cublasOperation_t
//cublasStatus_t cublasZgbmv(cublasHandle_t handle, cublasOperation_t

//tested lda in item not byte
#define CublasSgemv(handler, op, m, n, alpha, A, lda, x, incx, beta, y, incy) {\
	cublasOperation_t opt;\
	if(CUBLAS_OP_C == op){\
		printf("operation set error\n");\
	}else if(CUBLAS_OP_N == op){\
		opt = CUBLAS_OP_T;\
	}else if(CUBLAS_OP_T == op){\
		opt = CUBLAS_OP_N;\
	}\
	checkCublasError(cublasSgemv(handler, opt, n, m, alpha, A, lda, x, incx, beta, y, incy));\
}

#define CublasDgemv(handler, op, m, n, alpha, A, lda, x, incx, beta, y, incy) {\
	cublasOperation_t opt;\
	if(CUBLAS_OP_C == op){\
		printf("operation set error\n");\
	}else if(CUBLAS_OP_N == op){\
		opt = CUBLAS_OP_T;\
	}else if(CUBLAS_OP_T == op){\
		opt = CUBLAS_OP_N;\
	}\
	checkCublasError(cublasDgemv(handler, opt, n, m, alpha, A, lda, x, incx, beta, y, incy));\
}

#define CublasCgemv(handler, op, m, n, alpha, A, lda, x, incx, beta, y, incy) {\
	cublasOperation_t opt;\
	if(CUBLAS_OP_C|CUBLAS_OP_N == op){\
		opt = CUBLAS_OP_C|CUBLAS_OP_T;\
	}else if(CUBLAS_OP_T|CUBLAS_OP_C == op){\
		opt = CUBLAS_OP_C|CUBLAS_OP_N;\
	else if(CUBLAS_OP_N == op){\
		opt = CUBLAS_OP_T;\
	}else if(CUBLAS_OP_T == op){\
		opt = CUBLAS_OP_N;\
	}\
	checkCublasError(cublasCgemv(handler, opt, n, m, alpha, A, lda, x, incx, beta, y, incy));\
}

#define CublasZgemv(handler, op, m, n, alpha, A, lda, x, incx, beta, y, incy) {\
	cublasOperation_t opt;\
	if(CUBLAS_OP_C|CUBLAS_OP_N == op){\
		opt = CUBLAS_OP_C|CUBLAS_OP_T;\
	}else if(CUBLAS_OP_T|CUBLAS_OP_C == op){\
		opt = CUBLAS_OP_C|CUBLAS_OP_N;\
	else if(CUBLAS_OP_N == op){\
		opt = CUBLAS_OP_T;\
	}else if(CUBLAS_OP_T == op){\
		opt = CUBLAS_OP_N;\
	}\
	checkCublasError(cublasZgemv(handler, opt, n, m, alpha, A, lda, x, incx, beta, y, incy));\
}

//#define CublasSger(handler, m, n, alpha, x, incx, y, incy, A, lda) {\
}
//cublasDger (cublasHandle_t handle, int m,

//cublasCgeru(cublasHandle_t handle, int m,
//cublasCgerc(cublasHandle_t handle, int m,
//cublasZgeru(cublasHandle_t handle, int m,
//cublasZgerc(cublasHandle_t handle, int m,

//#define CublasSsymv(handler,  cublasSsymv(cublasHandle_t handle, cublasFillMode_t
//cublasStatus_t cublasDsymv(cublasHandle_t handle, cublasFillMode_t
//cublasStatus_t cublasCsymv(cublasHandle_t handle, cublasFillMode_t uplo,
//cublasStatus_t cublasZsymv(cublasHandle_t handle, cublasFillMode_t uplo,

//blas 3
#define CublasSgemm(handler, opa, opb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc) {\
	checkCublasError(cublasSgemm(handler, opb, opa, n, m, k, alpha, B, ldb, A, lda, beta, C, ldc));\
}
#define CublasDgemm(handler, opa, opb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc) {\
	checkCublasError(cublasDgemm(handler, opb, opa, n, m, k, alpha, B, ldb, A, lda, beta, C, ldc));\
}
#define CublasCgemm(handler, opa, opb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc) {\
	checkCublasError(cublasCgemm(handler, opb, opa, n, m, k, alpha, B, ldb, A, lda, beta, C, ldc));\
}
#define CublasZgemm(handler, opa, opb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc) {\
	checkCublasError(cublasZgemm(handler, opb, opa, n, m, k, alpha, B, ldb, A, lda, beta, C, ldc));\
}

#endif

