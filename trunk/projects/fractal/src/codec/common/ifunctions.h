#ifndef _IFUNCTIONS_H_
#define _IFUNCTIONS_H_


# if !(defined(WIN32) || defined(WIN64)) && (__STDC_VERSION__ < 199901L)
#define static
#define inline
#endif
#include <math.h>
#include <limits.h>


static inline short smin(short a, short b)
{
	return (short)(((a) < (b)) ? (a) : (b));
}

static inline short smax(short a, short b)
{
	return (short)(((a) > (b)) ? (a) : (b));
}

static inline int imin(int a, int b)
{
	return ((a) < (b)) ? (a) : (b);
}

static inline int imin3(int a, int b, int c)
{
	return ((a) < (b)) ? imin(a, c) : imin(b, c);
}

static inline int imax(int a, int b)
{
	return ((a) > (b)) ? (a) : (b);
}

static inline int imedian(int a, int b, int c)
{
	if (a > b) // a > b
	{
		if (b > c)
			return(b); // a > b > c
		else if (a > c)
			return(c); // a > c > b
		else
			return(a); // c > a > b
	}
	else // b > a
	{
		if (a > c)
			return(a); // b > a > c
		else if (b > c)
			return(c); // b > c > a
		else
			return(b);  // c > b > a
	}
}




#endif
