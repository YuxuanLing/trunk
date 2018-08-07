/**
 * @file
 * @author yyfn
 * @brief  wrap for curand
 *
 **/
#ifdef __CUDACC__

#pragma once
#include <stdio.h>
#include <curand.h>
#include <curand_kernel.h>
#include "../CUtil.h"

#define checkCurrandError(errno) {\
	if(CURAND_STATUS_SUCCESS == errno){\
	}else if(CURAND_STATUS_VERSION_MISMATCH == errno){\
		printError("Header file and linked library version do not match.\n");\
		exit(0);\
	}else if(CURAND_STATUS_NOT_INITIALIZED == errno){\
		printError("Generator not initialized.\n");\
		exit(0);\
	}else if(CURAND_STATUS_ALLOCATION_FAILED == errno){\
		printError("Memory allocation failed.\n");\
		exit(0);\
	}else if(CURAND_STATUS_TYPE_ERROR == errno){\
		printError("Generator is wrong type.\n");\
		exit(0);\
	}else if(CURAND_STATUS_OUT_OF_RANGE == errno){\
		printError("Argument out of range.\n");\
		exit(0);\
	}else if(CURAND_STATUS_LENGTH_NOT_MULTIPLE == errno){\
		printError("Length requested is not a multple of dimension.\n");\
		exit(0);\
	}else if(CURAND_STATUS_DOUBLE_PRECISION_REQUIRED == errno){\
		printError("GPU does not have double precision required by MRG32k3a.\n");\
		exit(0);\
	}else if(CURAND_STATUS_LAUNCH_FAILURE == errno){\
		printError("Kernel launch failure.\n");\
		exit(0);\
	}else if(CURAND_STATUS_PREEXISTING_FAILURE == errno){\
		printError("Preexisting failure on library entry.\n");\
		exit(0);\
	}else if(CURAND_STATUS_INITIALIZATION_FAILED == errno){\
		printError("Initialization of CUDA failed.\n");\
		exit(0);\
	}else if(CURAND_STATUS_ARCH_MISMATCH == errno){\
		printError("Architecture mismatch, GPU does not support requested feature.\n");\
		exit(0);\
	}else if(CURAND_STATUS_INTERNAL_ERROR == errno){\
		printError("Internal library error.\n");\
		exit(0);\
	}\
}

#define curandHandle_t curandGenerator_t

#define CurandCreate(generator, rng_type) checkCurrandError(curandCreateGenerator(generator, rng_type))
#define CurandCreateHost(generator, rng_type) checkCurrandError(curandCreateGeneratorHost(generator, rng_type))
#define CurandDestroy(generator) checkCurrandError(curandDestroyGenerator(generator))

#define CurandCreatePoissonDistribution(lambda, dis) checkCurrandError(curandCreatePoissonDistribution(lambda, dis))
#define CurandDestroyDistribution(dis) checkCurrandError(curandDestroyDistribution(dis))

#define CurandGenerate(generator, outputPtr, num) checkCurrandError(curandGenerate(generator, outputPtr, num))
#define CurandGenerateLogNormal(g, out, n, mean, dev) checkCurrandError(curandGenerateLogNormal(g, out, n, mean, dev))
#define CurandGenerateLogNormalDouble(g, out, n, mean, dev) checkCurrandError(curandGenerateLogNormalDouble(g, out, n, mean, dev))
#define CurandGenerateLongLong(g, out, num) checkCurrandError(curandGenerateLongLong(g, out, num))
#define CurandGenerateNormal(g, out, n, mean, dev) checkCurrandError(curandGenerateNormal(g, out, n, mean, dev))
#define CurandGenerateNormalDouble(g, out, n, mean, dev) checkCurrandError(curandGenerateNormalDouble(g, out, n, mean, dev))
#define CurandGeneratePoisson(g, out, n, lambda) checkCurrandError(curandGeneratePoisson(g, out, n, lambda))
#define CurandGenerateSeeds(g) checkCurrandError(curandGenerateSeeds(g))
#define CurandGenerateUniform(g, out, num) checkCurrandError(curandGenerateUniform(g, out, num))
#define CurandGenerateUniformDouble(g, out, num) checkCurrandError(curandGenerateUniformDouble(g, out, num))

#define CurandGetDirectionVectors32(v, set) checkCurrandError(curandGetDirectionVectors32(v, set))
#define CurandGetDirectionVectors64(v, set) checkCurrandError(curandGetDirectionVectors64(v, set))

#define CurandGetScrambleConstants32(constants) checkCurrandError(curandGetScrambleConstants32(constants))
#define CurandGetScrambleConstants64(constants) checkCurrandError(curandGetScrambleConstants64 (constants))
#define CurandGetVersion(version) checkCurrandError(curandGetVersion(version))
#define CurandSetGeneratorOffset(g, offset) checkCurrandError(curandSetGeneratorOffset(g, offset))

#define CurandSetGeneratorOrdering(g, order) checkCurrandError(curandSetGeneratorOrdering(g, order))
#define CurandSetPseudoRandomGeneratorSeed(g, seed) checkCurrandError(curandSetPseudoRandomGeneratorSeed(g, seed))
#define CurandSetQuasiRandomGeneratorDimensions(g, n) checkCurrandError(curandSetQuasiRandomGeneratorDimensions(g, n))
#define CurandSetStream(g, stream) checkCurrandError(curandSetStream(g, stream))


#endif
