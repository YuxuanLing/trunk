#pragma once
#include "../CUtil.h"
/**
 ** @file double4.h
 ** @author yyfn
 ** @date 20101025
 **/

#ifndef __CUDACC__
typedef struct {
	double x;
	double y;
	double z;
	double w;
} double4;

inline double4 make_double4(double x, double y, double z, double w) {
	double4 r;

	r.x = x;
	r.y = y;
	r.z = z;
	r.w = w;

	return r;
}

/**
 ** @brief get a <i> double4 </i> by int4 value
 **
 **/
inline CUDAC double4 make_double4(float4 a) {
	return make_double4(double(a.x), double(a.y), double(a.z), double(a.w));
}
#endif
/**
 ** @brief overload <i> negate </i> operator for <i> double4 </i>
 **
 **/
inline CUDAC double4 operator-(double4 &a) {
	return make_double4(-a.x, -a.y, -a.z, -a.w);
}

/**
 ** @brief overload <i> += </i> operator for <i> double4 </i>
 **
 **/
inline CUDAC void operator+=(double4 &a, double4 b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
}

/**
 ** @brief overload <i> + </i> operator for <i> double4 </i>
 **
 **/
inline CUDAC double4 operator+(double4 a, double4 b) {
	a += b;
	return a; //make_double4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
}

/**
 ** @brief overload <i> -= </i> operator for <i> double4 </i>
 **
 **/
inline CUDAC void operator-=(double4 &a, double4 b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= b.w;
}

/**
 ** @brief overload <i> - </i> operator for <i> double4 </i>
 **
 **/
inline CUDAC double4 operator-(double4 a, double4 b) {
	a -= b;
	return a; //make_double4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
}

/**
 ** @brief overload <i> *= </i> operator for <i> double4 </i>
 **
 **/
inline CUDAC void operator*=(double4& a, double4 b) {
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	a.w *= b.w;
}
/**
 ** @brief overload <i> * </i> operator for <i> double4 </i>
 **
 **/
inline CUDAC double4 operator*(double4 a, double4 b) {
	a *= b;
	return a; //make_double4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);
}

/**
 ** @brief overload <i> *= </i> operator for <i> double4 and double </i>
 **
 **/
inline CUDAC void operator*=(double4 &a, double s) {
	a.x *= s;
	a.y *= s;
	a.z *= s;
	a.w *= s;
}

/**
 ** @brief overload <i> * </i> operator for <i> double4 and double </i>
 **
 **/
inline CUDAC double4 operator*(double4 a, double s) {
	a *= s;
	return a; //make_double4(a.x*s, a.y*s, a.z*s, a.w*s);
}

inline CUDAC double4 operator*(double s, double4 a) {
	a *= s;
	return a; //make_double4(a.x*s, a.y*s, a.z*s, a.w*s);
}

/**
 ** @brief overload <i> /= </i> operator for <i> double4 </i>
 **
 **/
inline CUDAC void operator/=(double4& a, double4 b) {
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	a.w /= b.w;
}

/**
 ** @brief overload <i> / </i> operator for <i> double4 </i>
 **
 **/
inline CUDAC double4 operator/(double4 a, double4 b) {
	a /= b;
	return a; //make_double4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);
}

/**
 ** @brief overload <i> / </i> operator for <i> double4 and double </i>
 **
 **/
inline CUDAC double4 operator/(double4 a, double s) {
	return a*(1.0f/s);
}

/**
 ** @brief overload <i> /= </i> operator for <i> double4 </i>
 **
 **/
inline CUDAC void operator/=(double4 &a, double s) {
	a *= 1.0f/s;
}

/**
 ** @brief get a value between <i> a </i> and <i> b </i>
 **
 ** @param t ratio
 **/
inline CUDAC double4 lerp(double4 a, double4 b, double t) {
	return a + t*(b-a);
}

/**
 ** @brief get dot product of <i> double4 </i>
 **
 **/
inline CUDAC double dot(double4 a, double4 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

/**
 ** @brief get length of <i> double4 </i>
 **
 **/
inline CUDAC double length(double4 v) {
	return sqrtf(dot(v, v));
}

/**
 ** @brief normalize of <i> double4 </i>
 **
 **/
inline CUDAC double4 normalize(double4 v) {
	return v*rsqrtf(dot(v, v));
}

/**
 ** @brief get absolute value of <i> double4 </i>
 **
 **/
inline CUDAC double4 fabs(double4 v) {
	return make_double4(::fabs(v.x), ::fabs(v.y), ::fabs(v.z), ::fabs(v.w));
}
