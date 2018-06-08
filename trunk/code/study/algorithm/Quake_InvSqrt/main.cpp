#include<stdio.h>
#include<cstdlib>
#include<cmath>
#include <Windows.h>
#include<WinBase.h>
#include <xmmintrin.h>


#define ITERATIN_BASE (2500)

static const size_t cIteration = ITERATIN_BASE*4;
static const size_t cTestSize = ITERATIN_BASE*4;
static __declspec(align(16)) float gTestData[cTestSize];

#define TEST(func)                                                                                  \
{                                                                                                   \
	float sum = 0.0f; 																				\
	LARGE_INTEGER start, end, freq;																	\
	QueryPerformanceCounter(&start);																\
	for (size_t j = 0; j < cIteration; j++)															\
		for (size_t i = 0; i < cTestSize; i++)														\
			sum += func(gTestData[i]);																\
	QueryPerformanceCounter(&end);																	\
	QueryPerformanceFrequency(&freq);																\
	double duration = (end.QuadPart - start.QuadPart) * 1000.0 / double(freq.QuadPart);				\
	double peakError = 0.0f;																		\
	for (size_t i = 0; i < cTestSize; i++) {															\
		double actual = 1.0 / sqrt((double)gTestData[i]);											\
		peakError = max(peakError, abs((func(gTestData[i]) - actual)) / actual);					\
	}																								\
	printf("%s %.1fms  error=%.8f%%\n", #func, duration, peakError * 100.0, sum);						\
}

#define TEST_SS(func)																											  \
{																																  \
	__m128 sum = _mm_setzero_ps();																								  \
	LARGE_INTEGER start, end, freq;																								  \
	QueryPerformanceCounter(&start);																							  \
	for (size_t j = 0; j < cIteration; j++)																						  \
		for (size_t i = 0; i < cTestSize; i++)																					  \
			sum = _mm_add_ss(sum, func(_mm_load_ss(&gTestData[i])));															  \
	QueryPerformanceCounter(&end);																								  \
	QueryPerformanceFrequency(&freq);																							  \
	double duration = (end.QuadPart - start.QuadPart) * 1000.0 / double(freq.QuadPart);											  \
	double peakError = 0.0f;																									  \
	for (size_t i = 0; i < cTestSize; i++) {																						  \
		double actual = 1.0 / sqrt((double)gTestData[i]);																		  \
		peakError = max(peakError, abs((_mm_cvtss_f32(func(_mm_load_ss(&gTestData[i]))) - actual)) / actual);					  \
	}																															  \
	printf("%s %.1fms  error=%.8f%%\n", #func, duration, peakError * 100.0, _mm_cvtss_f32(sum));									  \
}

#define TEST_PS(func)																											   \
{																																   \
	__m128 sum = _mm_setzero_ps();																								   \
	LARGE_INTEGER start, end, freq;																								   \
	QueryPerformanceCounter(&start);																							   \
	for (size_t j = 0; j < cIteration; j++)																						   \
		for (size_t i = 0; i < cTestSize; i += 4)																				   \
			sum = _mm_add_ps(sum, func(_mm_load_ps(&gTestData[i])));															   \
	QueryPerformanceCounter(&end);																								   \
	QueryPerformanceFrequency(&freq);																							   \
	double duration = (end.QuadPart - start.QuadPart) * 1000.0 / double(freq.QuadPart);											   \
	double peakError = 0.0f;																									   \
	for (size_t i = 0; i < cTestSize; i += 4) {																					   \
		__m128 r = func(_mm_load_ps(&gTestData[i]));																			   \
		for (size_t j = 0; j < 4; j++) {																							   \
			double actual = 1.0 / sqrt((double)gTestData[i+j]);																	   \
			peakError = max(peakError, abs(r.m128_f32[j] - actual) / actual);													   \
		}																														   \
	}																															   \
	printf("%s %.1fms  error=%.8f%%\n", #func, duration, peakError * 100.0, _mm_cvtss_f32(sum));									   \
}

__forceinline float dummy(float x) {
	return x;
}

__forceinline float standard(float x) {
	return 1.0f / sqrtf(x);
}

__forceinline float quake(float x) {
	long i;
	float x2, y;
	const float threehalfs = 1.5F;
	x2 = x * 0.5F;
	y = x;
	i = *(long *)&y;
	i = 0x5f3759df - (i >> 1);
	y = *(float *)&i;
	y = y * (threehalfs - (x2 * y * y));
	return y;
}

__forceinline float quake2nd(float x) {
	long i;
	float x2, y;
	const float threehalfs = 1.5F;
	x2 = x * 0.5F;
	y = x;
	i = *(long *)&y;
	i = 0x5f3759df - (i >> 1);
	y = *(float *)&i;
	y = y * (threehalfs - (x2 * y * y));
	y = y * (threehalfs - (x2 * y * y));
	return y;
}

__forceinline __m128 dummy_ss(__m128 x) {
	return x;
}

__forceinline __m128 divsqrt_ss(__m128 x) {
	return _mm_div_ss(_mm_set1_ps(1.0f), _mm_sqrt_ss(x));
}

__forceinline __m128 rsqrt_ss(__m128 x) {
	return _mm_rsqrt_ss(x);
}

__forceinline __m128 rsqrt2nd_ss(__m128 x) {
	const __m128 x2 = _mm_mul_ss(x, _mm_set1_ps(0.5f));
	const __m128 y = _mm_rsqrt_ss(x);
	return _mm_mul_ss(y, _mm_sub_ss(_mm_set1_ps(1.5f), _mm_mul_ss(x2, _mm_mul_ss(y, y))));
}

__forceinline __m128 dummy_ps(__m128 x) {
	return x;
}

__forceinline __m128 divsqrt_ps(__m128 x) {
	return _mm_div_ps(_mm_set1_ps(1.0f), _mm_sqrt_ps(x));
}

__forceinline __m128 rsqrt_ps(__m128 x) {
	return _mm_rsqrt_ps(x);
}

__forceinline __m128 rsqrt2nd_ps(__m128 x) {
	const __m128 x2 = _mm_mul_ps(x, _mm_set1_ps(0.5f));
	const __m128 y = _mm_rsqrt_ps(x);
	return _mm_mul_ps(y, _mm_sub_ps(_mm_set1_ps(1.5f), _mm_mul_ps(x2, _mm_mul_ps(y, y))));
}

void main() {
	for (size_t i = 0; i < cTestSize; i++)
		gTestData[i] = (float)rand() / RAND_MAX + 0.5f; // [0.5, 1.5)

	TEST(dummy);
	TEST(standard);
	TEST(quake);
	TEST(quake2nd);
	TEST_SS(dummy_ss);
	TEST_SS(divsqrt_ss);
	TEST_SS(rsqrt_ss);
	TEST_SS(rsqrt2nd_ss);
	TEST_PS(dummy_ps);
	TEST_PS(divsqrt_ps);
	TEST_PS(rsqrt_ps);
	TEST_PS(rsqrt2nd_ps);
}