#ifndef H_Y_MATH
#define H_Y_MATH
/**
 * @file ymath.h
 * @brief util for math
 *
 * @author yyfn
 * @date 2010090
 **/

#include <math.h>
#include "CUtil.h"
/**
 * @brief mean
 * @param input the data input
 * @param len input's length
 * @return mean of input
 */
template<typename T>
T mean(const T *input, const int len) {
	T result = (T) 0;

	for (int i = 0; i < len; i++) {
		result += input[i];
	}

	return (result / len);
}
/**
 * @brief length
 *
 **/
template<typename T>
T length(const T *input, const size_t len) {
	double sum = 0.0;

	for (int i = 0; i < len; i++) {
		sum += input[i] * input[i];
	}

	return sqrt(sum);
}
/**
 * @brief dot product
 *
 * @param size vector dimension
 * @param a vector
 * @param b vector
 **/
template<typename T>
T dot(const T *a, const T *b, const size_t size) {
	T sum = (T) 0;

	for (size_t i = 0; i < size; i++) {
		sum += a[i] * b[i];
	}

	return sum;
}
/**
 * @brief get vector distance
 * @param size vector size
 * @param a vector
 * @param b vector
 **/
template<typename T>
T getDistanceCPU(const T *a, const T *b, const size_t size) {
	T dis = 0.0f;
	T temp;

	for (int i = 0; i < size; i++) {
		temp = a[i] - b[i];
		dis += temp * temp;
	}

	return sqrtf(dis);
}
/**
 * @brief normalize
 *
 **/
template<typename T>
void normalize(T *input, const size_t len) {
	double l = length(len, input);

	for (int i = 0; i < len; i++) {
		input[i] /= l;
	}
}
/**
 * @brief mean square error of input
 *
 * @param input data input
 * @param len input's length
 */
template<typename T>
T mse(const T *input, const int len) {
	T m = mean(input, len);
	T temp;
	T result = (T) 0;

	for (int i = 0; i < len; i++) {
		temp = input[i] - m;
		result += temp * temp;
	}

	return (sqrt(result / (len - 1)));
}

/**
 * @brief Round Up to multiple
 *
 * @param a larger one
 * @param b small one
 * @return multiple of b, nearest to a
 **/
int roundToMultiple(int a, int b) {
	int r = a % b;
	return (r == 0) ? a : (a + b - r);
}
/**
 * @brief return larger than x and power of 2
 *
 **/
int nextPow2(int x) {
	--x;

	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);

	return (x + 1);
}
/**
 * @brief return smaller than x and power of 2
 *
 **/
int previousPow2(int x) {
	if (0 == (x & (x - 1)))
		return x;
	return nextPow2(x) >> 1;
}

typedef unsigned int uint;
typedef unsigned short ushort;

#ifndef __CUDACC__
#include <math.h>

inline float fminf(float a, float b) {
	return a < b ? a : b;
}

inline float fmaxf(float a, float b) {
	return a > b ? a : b;
}

inline int max(int a, int b) {
	return a > b ? a : b;
}

inline int min(int a, int b) {
	return a < b ? a : b;
}

inline float rsqrtf(float x) {
	return 1.0f / sqrtf(x);
}
#endif

/**
 ** @breif get a value between <i>a</i> and <i>b</i> by ratio
 **
 ** @param a smeller value
 ** @param b larger value
 ** @param t ratio
 ** @return float
 **/
inline CUDAC float lerp(float a, float b, float t) {
	return a + t * (b - a);
}
/**
 ** @brief clamp <i> f </i> to <i> [a, b] </i>
 **
 **/
inline CUDAC float clamp(float f, float a, float b) {
	return fmaxf(a, fminf(f, b));
}
/*
 // smoothstep
 inline __device__ __host__ float smoothstep(float a, float b, float x)
 {
 float y = clamp((x - a) / (b - a), 0.0f, 1.0f);
 return (y*y*(3.0f - (2.0f*y)));
 }
 */

#include "ymath/int2.h"
#include "ymath/int3.h"
#include "ymath/int4.h"

#include "ymath/float2.h"
#include "ymath/float3.h"
#include "ymath/float4.h"

#include "ymath/double2.h"
#include "ymath/double3.h"
#include "ymath/double4.h"

#endif
