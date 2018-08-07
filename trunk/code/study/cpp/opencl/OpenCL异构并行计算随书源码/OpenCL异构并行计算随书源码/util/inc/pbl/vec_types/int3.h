#pragma once
#include "../CUtil.h"
/**
 ** @file int3.h
 ** @author yyfn
 ** @date 20101025
 **/
#ifndef __CUDACC__
typedef struct {
	int x;
	int y;
	int z;
} int3;

int3 make_int3(int x, int y, int z) {
	int3 r;

	r.x = x;
	r.y = y;
	r.z = z;

	return r;
}
#endif

/**
 ** @brief overload <i> negate </i> operator for <i> int3 </i>
 **
 **/
inline CUDAC int3 operator-(int3 &a) {
	return make_int3(-a.x, -a.y, -a.z);
}

/**
 ** @brief overload <i> += </i> operator for <i> int3 </i>
 **
 **/
inline CUDAC void operator+=(int3 &a, int3 b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
}

/**
 ** @brief overload <i> + </i> operator for <i> int3 </i>
 **
 **/
inline CUDAC int3 operator+(int3 a, int3 b) {
	a += b;
	return a; //make_int3(a.x+b.x, a.y+b.y, a.z+b.z);
}

/**
 ** @brief overload <i> -= </i> operator for <i> int3 </i>
 **
 **/
inline CUDAC void operator-=(int3 &a, int3 b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
}

/**
 ** @brief overload <i> - </i> operator for <i> int3 </i>
 **
 **/
inline CUDAC int3 operator-(int3 a, int3 b) {
	a -= b;
	return a; //make_int3(a.x-b.x, a.y-b.y, a.z-b.z);
}

/**
 ** @brief overload <i> *= </i> operator for <i>two int3 </i>
 **
 **/
inline CUDAC void operator*=(int3& a, int3 b) {
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
}

/**
 ** @brief overload <i> * </i> operator for <i> int3 </i>
 **
 **/
inline CUDAC int3 operator*(int3 a, int3 b) {
	a *= b;
	return a; //make_int3(a.x*b.x, a.y*b.y, a.z*b.z);
}

/**
 ** @brief overload <i> *= </i> operator for <i> int3 and int </i>
 **
 **/
inline CUDAC void operator*=(int3 &a, int s) {
	a.x *= s;
	a.y *= s;
	a.z *= s;
}

/**
 ** @brief overload <i> * </i> operator for <i> int3 and int </i>
 **
 **/
inline CUDAC int3 operator*(int3 a, int s) {
	a *= s;
	return a; //make_int3(a.x*s, a.y*s, a.z*s);
}

inline CUDAC int3 operator*(int s, int3 a) {
	a *= s;
	return a; //make_int3(a.x*s, a.y*s, a.z*s);
}

/**
 ** @brief overload <i> /= </i> operator for <i> int3 </i>
 **
 **/
inline CUDAC void operator/=(int3& a, int3 b) {
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
}

/**
 ** @brief overload <i> / </i> operator for <i> int3 </i>
 **
 **/
inline CUDAC int3 operator/(int3 a, int3 b) {
	a /= b;
	return a; //make_int3(a.x/b.x, a.y/b.y, a.z/b.z);
}

/**
 ** @brief overload <i> /= </i> operator for <i> int3 and int </i>
 **
 **/
inline CUDAC void operator/=(int3 &a, int s) {
	a.x /= s;
	a.y /= s;
	a.z /= s;
}
/**
 ** @brief overload <i> / </i> operator for <i> int3 and int </i>
 **
 **/
inline CUDAC int3 operator/(int3 a, int s) {
	a /= s;
	return a; //make_int3(a.x/s, a.y/s, a.z/s);
}

/**
 ** @brief get <i> abs </i> of <i> int3 </i>
 **
 **/
inline CUDAC int3 abs(int3 a) {
	return make_int3(::abs(a.x), ::abs(a.y), ::abs(a.z));
}

inline CUDAC int dot(int3 a, int3 b) {
	return (a.x*b.x + a.y*b.y + a.z*b.z);
}
/*
 inline
 #ifdef __CUDACC__
 __host__ __device__
 #endif
 int3 cross(int3 a, int3 b){
 return make_int3(a.x*b.y, a.y*b.x);
 }
 */
