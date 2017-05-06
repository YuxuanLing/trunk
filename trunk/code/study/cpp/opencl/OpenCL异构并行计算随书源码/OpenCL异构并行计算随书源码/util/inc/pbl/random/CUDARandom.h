/**
 * @file
 * @author yyfn
 * @date 20100921
 *
 * @brief rnd lib for CUDA
 **/
#ifdef __CUDACC__

#pragma once
#include <stdio.h>
#include <curand.h>
#include <curand_kernel.h>
#include "../CUDA.h"

namespace Curand {
/**
 * @class
 * @brief generate random data on gpu
 *
 * @notes -lcurand
 **/
class Generator {
protected:
	curandHandle_t g;
	Generator() {
	} //refuse other class access

public:
	/**
	 * @brief generate unsigned int rand data on GPU
	 *
	 * @param d_out global memory space which rand data store
	 * @param size the number of generated rand data
	 **/
	virtual void generate(unsigned int* d_out, size_t size) {
		CurandGenerate(g, d_out, size);
	}

	/**
	 * @brief generate uniform float rand data on GPU
	 *
	 * @param d_out global memory space which rand data store
	 * @param size the number of generated rand data
	 **/
	virtual void generateUniform(float *d_out, size_t size) {
		CurandGenerateUniform(g, d_out, size);
	}
	/**
	 * @brief generate uniform double rand data on GPU
	 *
	 * @param d_out global memory space which rand data store
	 * @param size the number of generated rand data
	 **/
	virtual void generateUniformDouble(double *d_out, size_t size) {
		CurandGenerateUniformDouble(g, d_out, size);
	}
	/**
	 * @brief generate normal float rand data on GPU
	 *
	 * @param d_out global memory space which rand data store
	 * @param size the number of generated rand data
	 * @param mean mean value of generated data
	 * @param dev standard deviate
	 **/
	virtual void generateNormal(float *d_out, size_t size, float mean,
			float dev) {
		CurandGenerateNormal(g, d_out, size, mean, dev);
	}

	virtual void generateNormalDouble(double* restrict d_out, size_t n, double mean, double dev) {
		CurandGenerateNormalDouble(g, d_out, n, mean, dev);
	}

	virtual void generateLogNormal(float* restrict d_out, size_t n, float mean, float dev) {
		CurandGenerateLogNormal(g, d_out, n, mean, dev);
	}

	virtual void generateLogNormalDouble(double *d_out, size_t n, double mean,
			double dev) {
		CurandGenerateLogNormalDouble(g, d_out, n, mean, dev);
	}

	virtual void generateLongLong(unsigned long long *d_out, size_t num) {
		CurandGenerateLongLong(g, d_out, num);
	}

	virtual void generatePoisson(unsigned int *d_out, size_t n, float lambda) {
		CurandGeneratePoisson(g, d_out, n, lambda);
	}

	/**
	 * @breif get curand version
	 *
	 **/
	static int getVersion() {
		int v;
		CurandGetVersion(&v);
		return v;
	}

	/**
	 * @breif set generater offset
	 *
	 **/
	virtual void setOffset(unsigned long long offset) {
		CurandSetGeneratorOffset(g, offset);
	}
	/**
	 *
	 **/
	virtual void setStream(cudaStream_t cst) {
		CurandSetStream(g, cst);
	}
	/**
	 *
	 *
	 **/
	virtual void setOrdering(curandOrdering_t cot) {
		CurandSetGeneratorOrdering(g, cot);
	}
};

class Pseudo:public Generator {
public:
	Pseudo(curandRngType_t rt) {
		if(CURAND_RNG_QUASI_DEFAULT == rt ||
				CURAND_RNG_QUASI_SOBOL32 == rt ||
				CURAND_RNG_QUASI_SCRAMBLED_SOBOL32 == rt ||
				CURAND_RNG_QUASI_SOBOL64 == rt||
				CURAND_RNG_QUASI_SCRAMBLED_SOBOL64 == rt) {
			rt = CURAND_RNG_PSEUDO_DEFAULT;
			printError("Random Type set error, use default\n");
		}
		CurandCreate(&g, rt);
	}

	Pseudo(){
		CurandCreate(&g, CURAND_RNG_PSEUDO_DEFAULT);
	}

	/**
	 * @brief set seed
	 *
	 **/
	void setSeed(unsigned long long seed) {
		CurandSetPseudoRandomGeneratorSeed(g, seed);
	}

	~Pseudo() {
		CurandDestroy(g);
	}
};

class Quasi:public Generator {
public:
	Quasi(curandRngType_t rt) {
		if(CURAND_RNG_PSEUDO_DEFAULT == rt ||
				CURAND_RNG_PSEUDO_XORWOW == rt ||
				CURAND_RNG_PSEUDO_MRG32K3A == rt ||
				CURAND_RNG_PSEUDO_MTGP32 == rt) {
			rt = CURAND_RNG_QUASI_DEFAULT;
			printError("Random Type set error, use default\n");
		}
		CurandCreate(&g, rt);
	}

	void getDirectionVectors32(curandDirectionVectors32_t *v[ ], curandDirectionVectorSet_t set) {
		CurandGetDirectionVectors32(v, set);
	}

	void getDirectionVectors64(curandDirectionVectors64_t *v[ ], curandDirectionVectorSet_t set){
		CurandGetDirectionVectors64(v, set);
	}

	void getScrambleConstants32(unsigned int **constants){
		CurandGetScrambleConstants32(constants);
	}

	void getScrambleConstants64(unsigned long long **constants){
		CurandGetScrambleConstants64(constants);
	}

	void SetDimension(unsigned int dim) {
		CurandSetQuasiRandomGeneratorDimensions(g, dim);
	}

	~Quasi() {
		CurandDestroy(g);
	}
};
}
#endif
