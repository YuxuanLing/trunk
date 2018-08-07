#ifndef H_FLOAT2
#define H_FLOAT2

#include "../CUtil.h"
/**
 ** @file float2.h
 ** @author yyfn
 ** @date 20101025
 **/

#ifndef __CUDACC__
typedef struct {
	float x;
	float y;
} float2;

inline float2 make_float2(float x, float y) {
	float2 r;

	r.x = x;
	r.y = y;

	return r;
}

/**
 ** @brief get a <i> float2 </i> by int2 value
 **
 **/
inline float2 make_float2(int2 a) {
	return make_float2((float) a.x, (float) a.y);
}
#endif

/**
 ** @brief overload <i> negate </i> operator for <i> float2 </i>
 **
 **/
inline CUDAC float2 operator-(float2 &a) {
	return make_float2(-a.x, -a.y);
}

/**
 ** @brief overload <i> += </i> operator for <i> float2 </i>
 **
 **/
inline CUDAC void operator+=(float2 &a, float2 b) {
	a.x += b.x;
	a.y += b.y;
}

/**
 ** @brief overload <i> + </i> operator for <i> float2 </i>
 **
 **/
inline CUDAC float2 operator+(float2 a, float2 b) {
	a += b;
	return a; //make_float2(a.x+b.x, a.y+b.y);
}

/**
 ** @brief overload <i> -= </i> operator for <i> float2 </i>
 **
 **/
inline CUDAC void operator-=(float2 &a, float2 b) {
	a.x -= b.x;
	a.y -= b.y;
}

/**
 ** @brief overload <i> += </i> operator for <i> float2 </i>
 **
 **/
inline CUDAC float2 operator-(float2 a, float2 b) {
	a -= b;
	return a; //make_float2(a.x-b.x, a.y-b.y);
}

/**
 ** @brief overload <i> *= </i> operator for <i> float2 and float</i>
 **
 **/
inline CUDAC void operator*=(float2 &a, float2 &b) {
	a.x *= b.x;
	a.y *= b.y;
}

/**
 ** @brief overload <i> * </i> operator for <i> float2 </i>
 **
 **/
inline CUDAC float2 operator*(float2 a, float2 b) {
	a *= b;
	return a; //make_float2(a.x*b.x, a.y*b.y);
}

/**
 ** @brief overload <i> *= </i> operator for <i> float2 and float </i>
 **
 **/
inline CUDAC void operator*=(float2& a, float s) {
	a.x *= s;
	a.y *= s;
}

/**
 ** @brief overload <i> * </i> operator for <i> float2 and float </i>
 **
 **/
inline CUDAC float2 operator*(float2 a, float s) {
	a *= s;
	return a; //make_float2(a.x*s, a.y*s);
}

inline CUDAC float2 operator*(float s, float2 a) {
	a *= s;
	return a; //make_float2(a.x*s, a.y*s);
}

/**
 ** @brief overload <i> /= </i> operator for <i> float2 </i>
 **
 **/
inline CUDAC void operator/=(float2& a, float2 b) {
	a.x /= b.x;
	a.y /= b.y;
}

/**
 ** @brief overload <i> / </i> operator for <i> float2 </i>
 **
 **/
inline CUDAC float2 operator/(float2 a, float2 b) {
	a /= b;
	return a; //make_float2(a.x/b.x, a.y/b.y);
}

/**
 ** @brief overload <i> /= </i> operator for <i> float2 and float</i>
 **
 **/
inline CUDAC void operator/=(float2 &a, float s) {
	a *= 1.0f/s;;
}

/**
 ** @brief overload <i> / </i> operator for <i> float2 and float</i>
 **
 **/
inline CUDAC float2 operator/(float2 a, float s) {
	a /= s;
	return a;
}

/**
 ** @brief get a value between <i> a </i> and <i> b </i>
 **
 ** @param t ratio

 **/
inline CUDAC float2 lerp(float2 a, float2 b, float t) {
	return a + t*(b-a);
}

/**
 ** @brief get dot product of <i> float2 </i>
 **
 **/
inline CUDAC float dot(float2 a, float2 b) {
	return a.x*b.x + a.y*b.y;
}

/**
 **  @brief get length of <i> float2 </i>
 **
 **/
inline CUDAC float length(float2 v) {
	return sqrtf(dot(v, v));
}

/**
 ** @brief normalize of <i> float2 </i>
 **
 **/
inline CUDAC float2 normalize(float2 v) {
	return v * rsqrtf(dot(v, v));
}
/*
 // floor
 inline __host__ __device__ float2 floor(const float2 v)
 {
 return make_float2(floor(v.x), floor(v.y));
 }

 // reflect
 inline __host__ __device__ float2 reflect(float2 i, float2 n)
 {
 return i - 2.0f * n * dot(n,i);
 }
 */
/**
 ** @brief get absolute value of <i> float2 </i>
 **
 **/
inline CUDAC float2 fabs(float2 v) {
	return make_float2(::fabs(v.x), ::fabs(v.y));
}

#endif
