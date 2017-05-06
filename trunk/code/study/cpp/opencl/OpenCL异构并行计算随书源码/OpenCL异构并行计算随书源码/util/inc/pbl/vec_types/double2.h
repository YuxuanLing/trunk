#pragma once

#include "../CUtil.h"

/**
 ** @file double2.h
 ** @author yyfn
 ** @date 20101025
 **/

#ifndef __CUDACC__
typedef struct {
	double x;
	double y;
} double2;

inline double2 make_double2(double x, double y) {
	double2 r;

	r.x = x;
	r.y = y;

	return r;
}

/**
 ** @brief get a <i> double2 </i> by double2 value
 **
 **/
inline double2 make_double2(float2 a) {
	return make_double2(a.x, a.y);
}
#endif

/**
 ** @brief overload <i> negate </i> operator for <i> double2 </i>
 **
 **/
inline CUDAC double2 operator-(double2 &a) {
	return make_double2(-a.x, -a.y);
}

/**
 ** @brief overload <i> += </i> operator for <i> double2 </i>
 **
 **/
inline CUDAC void operator+=(double2 &a, double2 b) {
	a.x += b.x;
	a.y += b.y;
}

/**
 ** @brief overload <i> + </i> operator for <i> double2 </i>
 **
 **/
inline CUDAC double2 operator+(double2 a, double2 b) {
	a += b;
	return a; //make_float2(a.x+b.x, a.y+b.y);
}

/**
 ** @brief overload <i> -= </i> operator for <i> double2 </i>
 **
 **/
inline CUDAC void operator-=(double2 &a, double2 b) {
	a.x -= b.x;
	a.y -= b.y;
}

/**
 ** @brief overload <i> += </i> operator for <i> double2 </i>
 **
 **/
inline CUDAC double2 operator-(double2 a, double2 b) {
	a -= b;
	return a; //make_float2(a.x-b.x, a.y-b.y);
}

/**
 ** @brief overload <i> *= </i> operator for <i> double2 and double</i>
 **
 **/
inline CUDAC void operator*=(double2 &a, double s) {
	a.x *= s;
	a.y *= s;
}

/**
 ** @brief overload <i> * </i> operator for <i> double2 </i>
 **
 **/
inline CUDAC void operator*=(double2 &a, double2 &b) {
	a.x *= b.x;
	a.y *= b.y;
}

inline CUDAC double2 operator*(double2 a, double2 b) {
	a *= b;
	return a; //make_float2(a.x*b.x, a.y*b.y);
}

/**
 ** @brief overload <i> * </i> operator for <i> double2 and float </i>
 **
 **/
inline CUDAC double2 operator*(double2 a, double s) {
	a *= s;
	return a; //make_float2(a.x*s, a.y*s);
}

inline CUDAC double2 operator*(double s, double2 a) {
	a *= s;
	return a; //make_float2(a.x*s, a.y*s);
}

/**
 ** @brief overload <i> /= </i> operator for <i> double2 </i>
 **
 **/
inline CUDAC void operator/=(double2& a, double2 b) {
	a.x /= b.x;
	a.y /= b.y;
}

/**
 ** @brief overload <i> / </i> operator for <i> double2 </i>
 **
 **/
inline CUDAC double2 operator/(double2 a, double2 b) {
	a /= b;
	return a; //make_float2(a.x/b.x, a.y/b.y);
}

/**
 ** @brief overload <i> /= </i> operator for <i> double2 and double</i>
 **
 **/
inline CUDAC void operator/=(double2 &a, double s) {
	a *= 1.0f / s;
	;
}

/**
 ** @brief overload <i> / </i> operator for <i> double2 and double</i>
 **
 **/
inline CUDAC double2 operator/(double2 a, double s) {
	a /= s;
	return a;
}

/**
 ** @brief get a value between <i> a </i> and <i> b </i>
 **
 ** @param t ratio

 **/
inline CUDAC double2 lerp(double2 a, double2 b, double t) {
	return a + t * (b - a);
}

/**
 ** @brief get dot product of <i> double2 </i>
 **
 **/
inline CUDAC double dot(double2 a, double2 b) {
	return a.x * b.x + a.y * b.y;
}

/**
 **  @brief get length of <i> double2 </i>
 **
 **/
inline CUDAC double length(double2 v) {
	return sqrt(dot(v, v));
}

/**
 ** @brief normalize of <i> double2 </i>
 **
 **/
inline CUDAC double2 normalize(double2 v) {
	return v * rsqrtf(dot(v, v));
}
/*
 // floor
 inline __host__ __device__ double2 floor(const double2 v)
 {
 return make_float2(floor(v.x), floor(v.y));
 }

 // reflect
 inline __host__ __device__ double2 reflect(double2 i, double2 n)
 {
 return i - 2.0f * n * dot(n,i);
 }
 */
/**
 ** @brief get absolute value of <i> double2 </i>
 **
 **/
inline CUDAC double2 fabs(double2 v) {
	return make_double2(::fabs(v.x), ::fabs(v.y));
}
