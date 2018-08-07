#pragma once
#include "../CUtil.h"
#include <math.h>
/**
 ** @file double3.h
 ** @author yyfn
 ** @date 20101025
 **/
#ifndef __CUDACC__
typedef struct {
	double x;
	double y;
	double z;
} double3;

inline double3 make_double3(double x, double y, double z) {
	double3 v;

	v.x = x;
	v.y = y;
	v.z = z;

	return v;
}

/**
 ** @brief get a <i> double3 </i> by int3 value
 **
 **/
inline double3 make_double3(float3 a) {
	return make_double3(double(a.x), double(a.y), double(a.z));
}
#endif

/**
 ** @brief overload <i> negate </i> operator of <i> double3 </i>

 **/
inline CUDAC double3 operator-(double3 &a) {
	return make_double3(-a.x, -a.y, -a.z);
}
/*
 // min
 static __inline__ __host__ __device__ double3 fminf(double3 a, double3 b)
 {
 return make_float3(fminf(a.x,b.x), fminf(a.y,b.y), fminf(a.z,b.z));
 }

 // max
 static __inline__ __host__ __device__ double3 fmaxf(double3 a, double3 b)
 {
 return make_float3(fmaxf(a.x,b.x), fmaxf(a.y,b.y), fmaxf(a.z,b.z));
 }
 */

/**
 ** @brief overload <i> += </i> operator for <i> double3 </i>
 **
 **/
inline CUDAC void operator+=(double3 &a, double3 b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
}

/**
 ** @brief overload <i> + </i> operator for <i> double3 </i>
 **
 **/
inline CUDAC double3 operator+(double3 a, double3 b) {
	a += b;
	return a; //make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

/**
 ** @brief overload <i> -= </i> operator for <i> double3 </i>
 **
 **/
inline CUDAC void operator-=(double3 &a, double3 b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
}

/**
 ** @brief overload <i> - </i> operator for <i> double3 </i>
 **
 **/
inline CUDAC double3 operator-(double3 a, double3 b) {
	a -= b;
	return a; //make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}

/**
 ** @brief overload <i> *= </i> operator for <i>two double3</i>
 **
 **/
inline CUDAC void operator*=(double3 &a, double3 b) {
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
}

/**
 ** @brief overload <i> * </i> operator for <i>two double3 </i>
 **
 **/
inline CUDAC double3 operator*(double3 a, double3 b) {
	a *= b;
	return a; //make_float3(a.x * b.x, a.y * b.y, a.z * b.z);
}
/**
 ** @brief overload <i> *= </i> operator for <i> double3 and double </i>
 **
 **/
inline CUDAC void operator*=(double3 &a, double s) {
	a.x *= s;
	a.y *= s;
	a.z *= s;
}
/**
 ** @brief overload <i> * </i> operator for <i> double3 and double </i>
 **
 **/
inline CUDAC double3 operator*(double3 a, double s) {
	a *= s;
	return a; //make_float3(a.x*s, a.y*s, a.z*s);
}

inline CUDAC double3 operator*(double s, double3 a) {
	a *= s;
	return a; //make_float3(a.x*s, a.y*s, a.z*s);
}
/**
 ** @brief overload <i> /= </i> operator for <i> double3 </i>
 **
 **/
inline CUDAC void operator/=(double3& a, double3 b) {
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
}

/**
 ** @brief overload <i> / </i> operator for <i> double3 </i>
 **
 **/
inline CUDAC double3 operator/(double3 a, double3 b) {
	a /= b;
	return a; //make_float3(a.x/b.x, a.y/b.y, a.z/b.z);
}

/**
 ** @brief overload <i> /= </i> operator for <i> double3 and double </i>
 **
 **/
inline CUDAC void operator/=(double3 &a, double s) {
	a *= 1.0f / s;
}
/**
 ** @brief overload <i> / </i> operator for <i> double3 and double </i>
 **
 **/
inline CUDAC double3 operator/(double3 a, double s) {
	a /= s;
	return a;
}

/**
 ** @brief get a value between <i> a </i> and <i> b </i>
 **
 ** @param t ratio

 **/
inline CUDAC double3 lerp(double3 a, double3 b, double t) {
	return a + t * (b - a);
}

/**
 ** @brief get dot product of <i> two double3 </i>
 **
 **/
inline CUDAC double dot(double3 a, double3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

/**
 ** @brief get cross product of <i> two double3 </i>
 **
 **/
inline CUDAC double3 cross(double3 a, double3 b) {
	return make_double3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x);
}

/**
 ** @brief get length of double3
 **
 **/
inline CUDAC double length(double3 v) {
	return sqrtf(dot(v, v));
}

/**
 ** @brief normalize of double3
 **
 **/
inline CUDAC double3 normalize(double3 v) {
	return v * rsqrtf(dot(v, v));
}
/*
 // floor
 inline __host__ __device__ double3 floor(const double3 v)
 {
 return make_float3(floor(v.x), floor(v.y), floor(v.z));
 }

 // reflect
 inline __host__ __device__ double3 reflect(double3 i, double3 n)
 {
 return i - 2.0f * n * dot(n,i);
 }
 */

/**
 ** @brief get absolute value of <i> double3 </i>
 **
 **/
inline CUDAC double3 fabs(double3 v) {
	return make_double3(::fabs(v.x), ::fabs(v.y), ::fabs(v.z));
}
