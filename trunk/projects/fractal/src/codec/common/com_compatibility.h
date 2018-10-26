#ifndef _COMPATIBILITY_H_
#define _COMPATIBILITY_H_

/* This header defines/redefines stuff needed for compatibility between
 * platforms and compilers. Standard types are not defined here, but in
 * taah264stdtypes.h. */

#ifdef _WIN32
# define _mm_free(a)       _aligned_free(a)
# define _mm_malloc(a, b)  _aligned_malloc(a, b)
#if _MSC_VER < 1900
# define snprintf _snprintf
#endif
#endif


/* We use Intel specific functions in the codec. Make sure it compiles with
 * other compilers as well. */
#ifdef __INTEL_COMPILER
# define TAA_H264_ALIGN(x) __declspec(align(x))
# define TAA_H264_NOINLINE __declspec(noinline)

#elif defined  _MSC_VER
#include<malloc.h>
#include<assert.h>
#include"taah264stdtypes.h"
# define TAA_H264_ALIGN(x) __declspec(align(x))
# define TAA_H264_NOINLINE __declspec(noinline)

//Returns the number of 1-bits in x.
uint32_t inline popcnt(uint32_t x)
{
	x -= ((x >> 1) & 0x55555555);
	x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
	x = (((x >> 4) + x) & 0x0f0f0f0f);
	x += (x >> 8);
	x += (x >> 16);
	return x & 0x0000003f;
}

//Returns the number of leading 0-bits in x, starting at the most significant bit position. If x is 0, the result is undefined.
uint32_t inline  clz(uint32_t x)
{
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return 32 - popcnt(x);
}

//Returns the number of trailing 0 - bits in x, starting at the least significant bit position.If x is 0, the result is undefined.
uint32_t inline  ctz(uint32_t x)
{
	return popcnt((x & -x) - 1);
}
//Returns one plus the index of the least significant 1-bit of x, or if x is zero, returns zero.
uint32_t inline  ffs(uint32_t x)
{
	unsigned char isNonzero;
	uint32_t index;
	isNonzero = _BitScanForward(&index, x);
	if (isNonzero)
	{
		return index + 1;
	}
	else
	{
		return 0;
	}
}


//return the pos of the first 1 from most significant (from right to left),  for example , 0x00000050 will return 6 
# define _bit_scan_reverse(a) (31 - clz(a))
//return the pos of the first 1 from least significant (from left to right),  for example , 0x00000050 will return 4 
# define _bit_scan_forward(a) (ffs(a) - 1)
# define _popcnt32(x) (popcnt(x))

//#define __assume_aligned(ptr, alignment)  assert(((uint64_t)ptr&(alignment - 1)) == 0)
#define __assume_aligned(ptr, alignment) 


#else
/* With GCC in mind... */
# define TAA_H264_ALIGN(x) __attribute__((aligned(x)))
# define __assume_aligned(ptr, alignment)
# define  __inline inline
# define TAA_H264_NOINLINE __attribute__((__noinline__))
# define _bit_scan_reverse(a) (31 - __builtin_clz (a))
# define _bit_scan_forward(a) (__builtin_ffs (a) - 1)
# define _popcnt32(x) (__builtin_popcount(x))

#include <mm_malloc.h>
//#include <malloc.h>

#endif

#define TAA_H264_MALLOC(size)           _mm_malloc(size, 64)
#define TAA_H264_MM_MALLOC(size, align) _mm_malloc(size, align)
#define TAA_H264_FREE(mem)              _mm_free(mem)
#define TAA_H264_MM_FREE(mem)           _mm_free(mem)

#endif
