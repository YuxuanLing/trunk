#ifndef _TAA_MATH_H_
#define _TAA_MATH_H_

#include "taah264stdtypes.h"
#include <stdlib.h>

#ifndef max
# define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
# define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#define TAA_H264_IS_IN_RANGE(a, low, high) ((a) >= (low) && (a) <= (high))
#define TAA_H264_CLIP(a, low, high) min(max((a), (low)), (high))
#define TAA_H264_DIV6(a) (((a) * 171) >> 10)

static inline int taa_h264_median (
  int a,
  int b,
  int c)
{
  int t1 = min (a, b);
  int t2 = (t1 == a) ? b : a;
  int t3 = max (t1, c);
  return (t1 == t3) ? t1 : min (t2, c);
}

#endif
