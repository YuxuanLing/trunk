#pragma once
#include "../CUtil.h"
/**
 ** @file
 ** @author yyfn
 ** @date 20121025
 *  @brief wrap for 4 int struct
 **/

#ifndef __CUDACC__
typedef struct {
	int x;
	int y;
	int z;
	int w;
} int4;

int4 make_int4(int x, int y, int z, int w) {
	int4 r;

	r.x = x;
	r.y = y;
	r.z = z;
	r.w = w;

	return r;
}
#endif
/*
 // min
 inline __host__ __device__ uint3 min(uint3 a, uint3 b)
 {
 return make_uint3(min(a.x,b.x), min(a.y,b.y), min(a.z,b.z));
 }

 // max
 inline __host__ __device__ uint3 max(uint3 a, uint3 b)
 {
 return make_uint3(max(a.x,b.x), max(a.y,b.y), max(a.z,b.z));
 }
 */
// addition
inline CUDAC void operator+=(int4 &a, int4 b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
}

inline CUDAC int4 operator+(int4 a, int4 b) {
	a += b;
	return a; //make_int4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
}

// subtract
inline CUDAC void operator-=(int4 &a, int4 b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= b.w;
}

inline CUDAC int4 operator-(int4 a, int4 b) {
	a -= b;
	return a; //make_int4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
}

// multiply
inline CUDAC void operator*=(int4& a, int4 b) {
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	a.w *= b.w;
}
inline CUDAC int4 operator*(int4 a, int4 b) {
	a *= b;
	return a; //make_int4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);
}

inline CUDAC void operator*=(int4& a, int s) {
	a.x *= s;
	a.y *= s;
	a.z *= s;
	a.w *= s;
}

inline CUDAC int4 operator*(int4 a, int s) {
	a *= s;
	return a; //make_int4(a.x*s, a.y*s, a.z*s, a.w*s);
}

inline CUDAC int4 operator*(int s, int4 a) {
	return (a * s);
}

// divide
inline CUDAC void operator/=(int4& a, int4 b) {
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	a.w /= b.w;
}

inline CUDAC int4 operator/(int4 a, int4 b) {
	a /= b;
	return a; //make_int4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);
}

inline CUDAC void operator/=(int4 &a, int s) {
	a.x /= s;
	a.y /= s;
	a.z /= s;
	a.w /= s;
}

inline CUDAC int4 operator/(int4 a, int s) {
	a /= s;
	return a; //make_int4(a.x/s, a.y/s, a.z/s, a.w/s);
}

/**
 ** @brief get <i> abs </i> of <i> int4 </i>
 **
 **/
inline CUDAC int4 abs(int4 a) {
	return make_int4(::abs(a.x), ::abs(a.y), ::abs(a.z), ::abs(a.w));
}

inline CUDAC int dot(int4 a, int4 b) {
	return (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w);
}
/*
 // clamp
 inline __device__ __host__ uint clamp(uint f, uint a, uint b)
 {
 return max(a, min(f, b));
 }

 inline __device__ __host__ uint3 clamp(uint3 v, uint a, uint b)
 {
 return make_uint3(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b));
 }

 inline __device__ __host__ uint3 clamp(uint3 v, uint3 a, uint3 b)
 {
 return make_uint3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z));
 }
 */
