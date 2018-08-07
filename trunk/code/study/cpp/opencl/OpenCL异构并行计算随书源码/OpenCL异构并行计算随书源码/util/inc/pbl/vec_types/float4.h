#pragma once
#include "../CUtil.h"
/**
 ** @file float4.h
 ** @author yyfn
 ** @date 20101025
 **/

#ifndef __CUDACC__
typedef struct {
	float x;
	float y;
	float z;
	float w;
} float4;

inline float4 make_float4(float x, float y, float z, float w) {
	float4 r;

	r.x = x;
	r.y = y;
	r.z = z;
	r.w = w;

	return r;
}
#endif
/**
 ** @brief get a <i> float4 </i> by int4 value
 **
 **/
/*
 inline
 #ifdef __CUDACC__
 __host__ __device__
 #endif
 float4 make_float4(int4 a){
 return make_float4(float(a.x), float(a.y), float(a.z), float(a.w));
 }
 */
/**
 ** @brief overload <i> negate </i> operator for <i> float4 </i>
 **
 **/
inline CUDAC float4 operator-(float4 &a) {
	return make_float4(-a.x, -a.y, -a.z, -a.w);
}

/**
 ** @brief overload <i> += </i> operator for <i> float4 </i>
 **
 **/
inline CUDAC void operator+=(float4 &a, float4 b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
}

/**
 ** @brief overload <i> + </i> operator for <i> float4 </i>
 **
 **/
inline CUDAC float4 operator+(float4 a, float4 b) {
	a += b;
	return a; //make_float4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
}

/**
 ** @brief overload <i> -= </i> operator for <i> float4 </i>
 **
 **/
inline CUDAC void operator-=(float4 &a, float4 b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= b.w;
}

/**
 ** @brief overload <i> - </i> operator for <i> float4 </i>
 **
 **/
inline CUDAC float4 operator-(float4 a, float4 b) {
	a -= b;
	return a; //make_float4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
}

/**
 ** @brief overload <i> *= </i> operator for <i> float4 </i>
 **
 **/
inline CUDAC void operator*=(float4& a, float4 b) {
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	a.w *= b.w;
}
/**
 ** @brief overload <i> * </i> operator for <i> float4 </i>
 **
 **/
inline CUDAC float4 operator*(float4 a, float4 b) {
	a *= b;
	return a; //make_float4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);
}

/**
 ** @brief overload <i> *= </i> operator for <i> float4 and float </i>
 **
 **/
inline CUDAC void operator*=(float4 &a, float s) {
	a.x *= s;
	a.y *= s;
	a.z *= s;
	a.w *= s;
}

/**
 ** @brief overload <i> * </i> operator for <i> float4 and float </i>
 **
 **/
inline CUDAC float4 operator*(float4 a, float s) {
	a *= s;
	return a; //make_float4(a.x*s, a.y*s, a.z*s, a.w*s);
}

inline CUDAC float4 operator*(float s, float4 a) {
	a *= s;
	return a; //make_float4(a.x*s, a.y*s, a.z*s, a.w*s);
}

/**
 ** @brief overload <i> /= </i> operator for <i> float4 </i>
 **
 **/
inline CUDAC void operator/=(float4& a, float4 b) {
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	a.w /= b.w;
}

/**
 ** @brief overload <i> / </i> operator for <i> float4 </i>
 **
 **/
inline CUDAC float4 operator/(float4 a, float4 b) {
	a /= b;
	return a; //make_float4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);
}

/**
 ** @brief overload <i> / </i> operator for <i> float4 and float </i>
 **
 **/
inline CUDAC float4 operator/(float4 a, float s) {
	return a*(1.0f/s);
}

/**
 ** @brief overload <i> /= </i> operator for <i> float4 </i>
 **
 **/
inline CUDAC void operator/=(float4 &a, float s) {
	a *= 1.0f/s;
}

/**
 ** @brief get a value between <i> a </i> and <i> b </i>
 **
 ** @param t ratio
 **/
inline CUDAC float4 lerp(float4 a, float4 b, float t) {
	return a + t*(b-a);
}

/**
 ** @brief get dot product of <i> float4 </i>
 **
 **/
inline CUDAC float dot(float4 a, float4 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

/**
 ** @brief get length of <i> float4 </i>
 **
 **/
inline CUDAC float length(float4 v) {
	return sqrtf(dot(v, v));
}

/**
 ** @brief normalize of <i> float4 </i>
 **
 **/
inline CUDAC float4 normalize(float4 v) {
	return v*rsqrtf(dot(v, v));
}

/**
 ** @brief get absolute value of <i> float4 </i>
 **
 **/
inline CUDAC float4 fabs(float4 v) {
	return make_float4(::fabs(v.x), ::fabs(v.y), ::fabs(v.z), ::fabs(v.w));
}
