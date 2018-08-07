#pragma once
#include "../CUtil.h"
/**
 ** @file float3.h
 ** @author yyfn
 ** @date 20101025
 **/
#ifndef __CUDACC__
typedef struct {
	float x;
	float y;
	float z;
} float3;

inline float3 make_float3(float x, float y, float z) {
	float3 v;

	v.x = x;
	v.y = y;
	v.z = z;

	return v;
}

/**
 ** @brief get a <i> float3 </i> by int3 value
 **
 **/
inline float3 make_float3(int3 a) {
	return make_float3(float(a.x), float(a.y), float(a.z));
}
#endif

/**
 ** @brief overload <i> negate </i> operator of <i> float3 </i>

 **/
inline CUDAC float3 operator-(float3 &a) {
	return make_float3(-a.x, -a.y, -a.z);
}
/*
 // min
 static __inline__ __host__ __device__ float3 fminf(float3 a, float3 b)
 {
 return make_float3(fminf(a.x,b.x), fminf(a.y,b.y), fminf(a.z,b.z));
 }

 // max
 static __inline__ __host__ __device__ float3 fmaxf(float3 a, float3 b)
 {
 return make_float3(fmaxf(a.x,b.x), fmaxf(a.y,b.y), fmaxf(a.z,b.z));
 }
 */

/**
 ** @brief overload <i> += </i> operator for <i> float3 </i>
 **
 **/
inline CUDAC void operator+=(float3 &a, float3 b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
}

/**
 ** @brief overload <i> + </i> operator for <i> float3 </i>
 **
 **/
inline CUDAC float3 operator+(float3 a, float3 b) {
	a += b;
	return a; //make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

/**
 ** @brief overload <i> -= </i> operator for <i> float3 </i>
 **
 **/
inline CUDAC void operator-=(float3 &a, float3 b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
}

/**
 ** @brief overload <i> - </i> operator for <i> float3 </i>
 **
 **/
inline CUDAC float3 operator-(float3 a, float3 b) {
	a -= b;
	return a; //make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}

/**
 ** @brief overload <i> *= </i> operator for <i>two float3</i>
 **
 **/
inline CUDAC void operator*=(float3 &a, float3 b) {
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
}

/**
 ** @brief overload <i> * </i> operator for <i>two float3 </i>
 **
 **/
inline CUDAC float3 operator*(float3 a, float3 b) {
	a *= b;
	return a; //make_float3(a.x * b.x, a.y * b.y, a.z * b.z);
}
/**
 ** @brief overload <i> *= </i> operator for <i> float3 and float </i>
 **
 **/
inline CUDAC void operator*=(float3 &a, float s) {
	a.x *= s;
	a.y *= s;
	a.z *= s;
}
/**
 ** @brief overload <i> * </i> operator for <i> float3 and float </i>
 **
 **/
inline CUDAC float3 operator*(float3 a, float s) {
	a *= s;
	return a; //make_float3(a.x*s, a.y*s, a.z*s);
}

inline CUDAC float3 operator*(float s, float3 a) {
	a *= s;
	return a; //make_float3(a.x*s, a.y*s, a.z*s);
}
/**
 ** @brief overload <i> /= </i> operator for <i> float3 </i>
 **
 **/
inline CUDAC void operator/=(float3& a, float3 b) {
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
}

/**
 ** @brief overload <i> / </i> operator for <i> float3 </i>
 **
 **/
inline CUDAC float3 operator/(float3 a, float3 b) {
	a /= b;
	return a; //make_float3(a.x/b.x, a.y/b.y, a.z/b.z);
}

/**
 ** @brief overload <i> /= </i> operator for <i> float3 and float </i>
 **
 **/
inline CUDAC void operator/=(float3 &a, float s) {
	a *= 1.0f/s;
}
/**
 ** @brief overload <i> / </i> operator for <i> float3 and float </i>
 **
 **/
inline CUDAC float3 operator/(float3 a, float s) {
	a /= s;
	return a;
}

/**
 ** @brief get a value between <i> a </i> and <i> b </i>
 **
 ** @param t ratio

 **/
inline CUDAC float3 lerp(float3 a, float3 b, float t) {
	return a + t*(b-a);
}

/**
 ** @brief get dot product of <i> two float3 </i>
 **
 **/
inline CUDAC float dot(float3 a, float3 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

/**
 ** @brief get cross product of <i> two float3 </i>
 **
 **/
inline CUDAC float3 cross(float3 a, float3 b) {
	return make_float3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

/**
 ** @brief get length of float3
 **
 **/
inline CUDAC float length(float3 v) {
	return sqrtf(dot(v, v));
}

/**
 ** @brief normalize of float3
 **
 **/
inline CUDAC float3 normalize(float3 v) {
	return v*rsqrtf(dot(v, v));
}
/*
 // floor
 inline __host__ __device__ float3 floor(const float3 v)
 {
 return make_float3(floor(v.x), floor(v.y), floor(v.z));
 }

 // reflect
 inline __host__ __device__ float3 reflect(float3 i, float3 n)
 {
 return i - 2.0f * n * dot(n,i);
 }
 */

/**
 ** @brief get absolute value of <i> float3 </i>
 **
 **/
inline CUDAC float3 fabs(float3 v) {
	return make_float3(::fabs(v.x), ::fabs(v.y), ::fabs(v.z));
}
