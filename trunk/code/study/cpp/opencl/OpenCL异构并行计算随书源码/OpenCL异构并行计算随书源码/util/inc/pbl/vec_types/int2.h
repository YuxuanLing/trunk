#pragma once
#include "../CUtil.h"
#include <math.h>
/**
 ** @file int2.h
 ** @author yyfn
 ** @date 20101025
 **/

#ifndef __CUDACC__
typedef struct {
	int x;
	int y;
} int2;

inline int2 make_int2(int x, int y) {
	int2 r;

	r.x = x;
	r.y = y;

	return r;
}
#endif

/**
 ** @brief overload <i> negate </i> operator for <i> int2 </i>
 **
 **/
inline CUDAC int2 operator-(int2 &a) {
	return make_int2(-a.x, -a.y);
}

/**
 ** @brief overload <i> += </i> operator for <i> int2 </i>
 **
 **/
inline CUDAC void operator+=(int2 &a, int2 b) {
	a.x += b.x;
	a.y += b.y;
}

/**
 ** @brief overload <i> + </i> operator for <i> int2 </i>
 **
 **/
inline CUDAC int2 operator+(int2 a, int2 b) {
	a += b;
	return a; //make_int2(a.x + b.x, a.y + b.y);
}

/**
 ** @brief overload <i> -= </i> operator for <i> int2 </i>
 **
 **/
inline CUDAC void operator-=(int2 &a, int2 b) {
	a.x -= b.x;
	a.y -= b.y;
}

/**
 ** @brief overload <i> - </i> operator for <i> int2 </i>
 **
 **/
inline CUDAC int2 operator-(int2 a, int2 b) {
	a -= b;
	return a; //make_int2(a.x - b.x, a.y - b.y);
}

/**
 ** @brief overload <i> *= </i> operator for <i>two int2 </i>
 **
 **/
inline CUDAC void operator*=(int2& a, int2 b) {
	a.x *= b.x;
	a.y *= b.y;
}

/**
 ** @brief overload <i> * </i> operator for <i>two int2 </i>
 **
 **/
inline CUDAC int2 operator*(int2 a, int2 b) {
	a *= b;
	return a; //make_int2(a.x * b.x, a.y * b.y);
}

/**
 ** @brief overload <i> *= </i> operator for <i> int2 and int </i>
 **
 **/
inline CUDAC void operator*=(int2 &a, int s) {
	a.x *= s;
	a.y *= s;
}

/**
 ** @brief overload <i> * </i> operator for <i> int2 and int </i>
 **
 **/
inline CUDAC int2 operator*(int2 a, int s) {
	a *= s;
	return a; //make_int2(a.x * s, a.y * s);
}

inline CUDAC int2 operator*(int s, int2 a) {
	a *= s;
	return a; //make_int2(a.x * s, a.y * s);
}

/**
 ** @brief overload <i> /= </i> operator for <i> two int2</i>
 **
 **/
inline CUDAC void operator/=(int2& a, int2 b) {
	a.x /= b.x;
	a.y /= b.y;
}
/**
 ** @brief overload <i> / </i> operator for <i> two int2</i>
 **
 **/
inline CUDAC int2 operator/(int2 a, int2 b) {
	a /= b;
	return a;
}
/**
 ** @brief overload <i> /= </i> operator for <i> int2 and int </i>
 **
 **/
inline CUDAC void operator/=(int2& a, int s) {
	a.x /= s;
	a.y /= s;
}

inline CUDAC int2 operator/(int2 a, int s) {
	a /= s;
	return a; //make_int2(a.x * s, a.y * s);
}

/**
 ** @brief get <i> dot product </i> of <i> two int2 </i>
 **
 **/
inline CUDAC int dot(int2 a, int2 b) {
	return a.x * b.x + a.y * b.y;
}

/**
 ** @brief get <i> cross product </i> of <i> two int2 </i>
 **
 **/
inline CUDAC int2 cross(int2 a, int2 b) {
	return make_int2(a.x * b.y, a.y * b.x);
}

/**
 ** @brief get <i> abs </i> of <i> int2 </i>
 **
 **/
inline CUDAC int2 abs(int2 a) {
	return make_int2(::abs(a.x), ::abs(a.y));
}
