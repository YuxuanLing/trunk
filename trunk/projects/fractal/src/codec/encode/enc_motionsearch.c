/* ================================================================================= */
/*                                                                                   */
/*                               Copyright Notice                                    */
/*                                                                                   */
/*  All material herein: Copyright © 2009 TANDBERG Telecom AS. All Rights Reserved.  */
/*  This product is protected under US patents. Other patents pending.               */
/*  Further information can be found at http://www.tandberg.com/tandberg_pm.jsp.     */
/*                                                                                   */
/*  The source code is owned by TANDBERG Telecom AS and is protected by copyright    */
/*  laws and international copyright treaties, as well as other intellectual         */
/*  property laws and treaties. All right, title and interest in the source code     */
/*  are owned TANDBERG Telecom AS. Therefore, you must treat the source code like    */
/*  any other copyrighted material.                                                  */
/*                                                                                   */
/* ================================================================================= */
#include "taah264stdtypes.h"
#include "com_interpolation.h"
#include "com_compatibility.h"
#include "com_intmath.h"
#include "com_common.h"
#include "com_error.h"
#include "enc_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <emmintrin.h>

#if SANITY_INTERPOLATION
# include "com_interpolation_ref.h"
#endif

// left most bit detect and motion vector cost
#define _lmbd(a) (((a) == (0)) ? (32) : (31 - _bit_scan_reverse(a)))
#define MV_COST(lambda, dx, dy)        (((130 - 2 * (_lmbd(abs(dx)) + _lmbd(abs(dy)))) * lambda) >> LAMBDA_SHIFT)
#define MV_COST2(lambda, lmbd1, lmbd2) (((130 - 2 * (lmbd1          + lmbd2))          * lambda) >> LAMBDA_SHIFT)

#pragma warning (disable: 981)

#ifdef AVG_HPEL
/* Disable remarks about unused variables. We have a lot of unused
 * hpel and vpel variables in this case. */
# pragma warning (disable: 869)
#endif


static
void taa_h264_qp_sad16x16h2(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * hpel,
  const uint8_t * fpel,
  uint32_t        sad[2]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  __assume_aligned(curr, 16);
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 16; j++)
    {
#ifdef AVG_HPEL
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i* stride + j+0]+fpel[i * stride + j+1]+1)>>1) + fpel[i * stride + j] + 1) >> 1));     // left
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i* stride + j+0]+fpel[i * stride + j+1]+1)>>1) + fpel[i * stride + j + 1] + 1) >> 1)); // rigtht
#else
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((hpel[i * stride + j] + fpel[i * stride + j] + 1) >> 1));     // left
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((hpel[i * stride + j] + fpel[i * stride + j + 1] + 1) >> 1)); // rigtht
#endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
}

static
void taa_h264_qp_sad16x8h2(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * hpel,
  const uint8_t * fpel,
  uint32_t        sad[2]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  __assume_aligned(curr, 16);
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 16; j++)
    {
#ifdef AVG_HPEL
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i* stride + j+0]+fpel[i * stride + j+1]+1)>>1) + fpel[i * stride + j] + 1) >> 1));     // left
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i* stride + j+0]+fpel[i * stride + j+1]+1)>>1) + fpel[i * stride + j + 1] + 1) >> 1)); // rigtht
#else
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((hpel[i * stride + j] + fpel[i * stride + j] + 1) >> 1));     // left
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((hpel[i * stride + j] + fpel[i * stride + j + 1] + 1) >> 1)); // rigtht
#endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
}
static
void taa_h264_qp_sad8x16h2(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * hpel,
  const uint8_t * fpel,
  uint32_t        sad[2]
  )
{
#ifdef AVG_HPEL
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  //__assume_aligned(curr, 16);
  __assume_aligned(curr, 8);
  for (i = 0; i < 8; i++)
  {
    TAA_H264_ALIGN(64) uint8_t avg0[16], cur[16], fp0[16], fp1[16];
    __m128i xmm0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+8) * stride + 0])), *(((double *)(&fpel[i * stride + 0])))));
    __m128i xmm1 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+8) * stride + 1])), *(((double *)(&fpel[i * stride + 1])))));
    _mm_store_si128((__m128i *) avg0, _mm_avg_epu8(xmm0, xmm1));
    _mm_store_pd((double *) cur, (_mm_set_pd (*((double *)(&curr[(i+8) * MB_WIDTH_Y])), *(((double *)(&curr[i * MB_WIDTH_Y]))))));
    _mm_store_pd((double *) fp0, (_mm_set_pd (*((double *)(&fpel[(i+8) * stride + 0])), *(((double *)(&fpel[i * stride + 0]))))));
    _mm_store_pd((double *) fp1, (_mm_set_pd (*((double *)(&fpel[(i+8) * stride + 1])), *(((double *)(&fpel[i * stride + 1]))))));
    for (j = 0; j < 16; j++)
    {
      sad0 += abs(cur[j] - ((avg0[j] + fp0[j] + 1) >> 1)); // left
      sad1 += abs(cur[j] - ((avg0[j] + fp1[j] + 1) >> 1)); // rigtht
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
#else
  int i;
  __m128i sum  = _mm_setzero_si128();
  __m128i sum0 = _mm_setzero_si128();
  __m128i sum1 = _mm_setzero_si128();
  for(i = 0; i < 8; i++)
  {
    sum0 = _mm_add_epi32(sum0, _mm_sad_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&curr[MB_WIDTH_Y * (i + 0)])))), _mm_avg_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&hpel[stride * (i + 0)])))), _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[stride * (i + 0) + 1])), *(((double *)(&fpel[stride * (i + 0)]))))))));
    sum1 = _mm_add_epi32(sum1, _mm_sad_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&curr[MB_WIDTH_Y * (i + 8)])))), _mm_avg_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&hpel[stride * (i + 8)])))), _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[stride * (i + 8) + 1])), *(((double *)(&fpel[stride * (i + 8)]))))))));
  }
  sum = _mm_add_epi32(sum0, sum1);
  sad[0] = _mm_cvtsi128_si32(sum);
  sad[1] = _mm_cvtsi128_si32(_mm_srli_si128(sum, 8));
#endif
}

static
void taa_h264_qp_sad16x16v2(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * vpel,
  const uint8_t * fpel,
  uint32_t        sad[2]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  __assume_aligned(curr, 16);
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 16; j++)
    {
#ifdef AVG_HPEL
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i+0) * stride + j]+fpel[(i+1) * stride + j] + 1)>>1) + fpel[i * stride + j] + 1) >> 1));
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i+0) * stride + j]+fpel[(i+1) * stride + j] + 1)>>1) + fpel[(i + 1) * stride + j] + 1) >> 1));
#else
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + fpel[i * stride + j] + 1) >> 1));
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + fpel[(i + 1) * stride + j] + 1) >> 1));
#endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
}

#ifdef AVG_HPEL
/* This is unfortunate, but we don't want all these remark's about of
 * unused half pel variables (used only when using exact interpolation
 * in mv-search). */
# pragma warning (disable:869)
#endif

#if HPEL_DIAG_ENABLED
static
void taa_h264_qp_sad16x16d4(
  const int       stride,
  const uint8_t * cur,
  const uint8_t * fpel,
  const uint8_t * hpel,
  const uint8_t * vpel,
  const uint8_t * dpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(cur, 16);
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 16; j++)
    {
# ifdef AVG_HPEL
      sad0 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i*stride + j] + fpel[(i+1)*stride + j + 1] + 1) >> 1) + ((fpel[(i+0)*stride + j + 0] + fpel[(i+0)*stride + j + 1] + 1) >> 1) + 1) >> 1));
      sad1 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i*stride + j] + fpel[(i+1)*stride + j + 1] + 1) >> 1) + ((fpel[(i+0)*stride + j + 0] + fpel[(i+1)*stride + j + 0] + 1) >> 1) + 1) >> 1));
      sad2 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i*stride + j] + fpel[(i+1)*stride + j + 1] + 1) >> 1) + ((fpel[(i+0)*stride + j + 1] + fpel[(i+1)*stride + j + 1] + 1) >> 1) + 1) >> 1));
      sad3 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i*stride + j] + fpel[(i+1)*stride + j + 1] + 1) >> 1) + ((fpel[(i+1)*stride + j + 0] + fpel[(i+1)*stride + j + 1] + 1) >> 1) + 1) >> 1));
# else
      sad0 += abs(cur[i * MB_WIDTH_Y + j] - ((dpel[i*stride + j] + hpel[(i + 0) * stride + j + 0] + 1) >> 1));
      sad1 += abs(cur[i * MB_WIDTH_Y + j] - ((dpel[i*stride + j] + vpel[(i + 0) * stride + j + 0] + 1) >> 1));
      sad2 += abs(cur[i * MB_WIDTH_Y + j] - ((dpel[i*stride + j] + vpel[(i + 0) * stride + j + 1] + 1) >> 1));
      sad3 += abs(cur[i * MB_WIDTH_Y + j] - ((dpel[i*stride + j] + hpel[(i + 1) * stride + j + 0] + 1) >> 1));
# endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
}

static
void taa_h264_qp_sad16x16x4(
  const int       stride,
  const uint8_t * cur,
  const uint8_t * fpel,
  const uint8_t * hpel,
  const uint8_t * vpel,
  const uint8_t * dpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(cur, 16);
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 16; j++)
    {
# ifdef AVG_HPEL
      sad0 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i * stride + j + 0] + fpel[(i + 1) * stride + j + 0] + 1) >> 1) + ((fpel[(i + 0) * stride + j] + fpel[(i + 0) * stride + j + 1] + 1) >> 1) + 1) >> 1));
      sad1 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i * stride + j + 1] + fpel[(i + 1) * stride + j + 1] + 1) >> 1) + ((fpel[(i + 0) * stride + j] + fpel[(i + 0) * stride + j + 1] + 1) >> 1) + 1) >> 1));
      sad2 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i * stride + j + 0] + fpel[(i + 1) * stride + j + 0] + 1) >> 1) + ((fpel[(i + 1) * stride + j] + fpel[(i + 1) * stride + j + 1] + 1) >> 1) + 1) >> 1));
      sad3 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i * stride + j + 1] + fpel[(i + 1) * stride + j + 1] + 1) >> 1) + ((fpel[(i + 1) * stride + j] + fpel[(i + 1) * stride + j + 1] + 1) >> 1) + 1) >> 1));
# else
      sad0 += abs(cur[i * MB_WIDTH_Y + j] - ((vpel[(i + 0) * stride + j + 0] + hpel[(i + 0) * stride + j + 0] + 1) >> 1));
      sad1 += abs(cur[i * MB_WIDTH_Y + j] - ((vpel[(i + 0) * stride + j + 1] + hpel[(i + 0) * stride + j + 0] + 1) >> 1));
      sad2 += abs(cur[i * MB_WIDTH_Y + j] - ((vpel[(i + 0) * stride + j + 0] + hpel[(i + 1) * stride + j + 0] + 1) >> 1));
      sad3 += abs(cur[i * MB_WIDTH_Y + j] - ((vpel[(i + 0) * stride + j + 1] + hpel[(i + 1) * stride + j + 0] + 1) >> 1));
# endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
}


static
void taa_h264_qp_sad16x8d4(
  const int       stride,
  const uint8_t * cur,
  const uint8_t * fpel,
  const uint8_t * hpel,
  const uint8_t * vpel,
  const uint8_t * dpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(cur, 16);
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 16; j++)
    {
# ifdef AVG_HPEL
      sad0 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i*stride + j] + fpel[(i+1)*stride + j + 1] + 1) >> 1) + ((fpel[(i+0)*stride + j + 0] + fpel[(i+0)*stride + j + 1] + 1) >> 1) + 1) >> 1));
      sad1 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i*stride + j] + fpel[(i+1)*stride + j + 1] + 1) >> 1) + ((fpel[(i+0)*stride + j + 0] + fpel[(i+1)*stride + j + 0] + 1) >> 1) + 1) >> 1));
      sad2 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i*stride + j] + fpel[(i+1)*stride + j + 1] + 1) >> 1) + ((fpel[(i+0)*stride + j + 1] + fpel[(i+1)*stride + j + 1] + 1) >> 1) + 1) >> 1));
      sad3 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i*stride + j] + fpel[(i+1)*stride + j + 1] + 1) >> 1) + ((fpel[(i+1)*stride + j + 0] + fpel[(i+1)*stride + j + 1] + 1) >> 1) + 1) >> 1));
# else
      sad0 += abs(cur[i * MB_WIDTH_Y + j] - ((dpel[i*stride + j] + hpel[(i + 0) * stride + j + 0] + 1) >> 1));
      sad1 += abs(cur[i * MB_WIDTH_Y + j] - ((dpel[i*stride + j] + vpel[(i + 0) * stride + j + 0] + 1) >> 1));
      sad2 += abs(cur[i * MB_WIDTH_Y + j] - ((dpel[i*stride + j] + vpel[(i + 0) * stride + j + 1] + 1) >> 1));
      sad3 += abs(cur[i * MB_WIDTH_Y + j] - ((dpel[i*stride + j] + hpel[(i + 1) * stride + j + 0] + 1) >> 1));
# endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
}

static
void taa_h264_qp_sad16x8x4(
  const int       stride,
  const uint8_t * cur,
  const uint8_t * fpel,
  const uint8_t * hpel,
  const uint8_t * vpel,
  const uint8_t * dpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(cur, 16);
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 16; j++)
    {
# ifdef AVG_HPEL
      sad0 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i * stride + j + 0] + fpel[(i + 1) * stride + j + 0] + 1) >> 1) + ((fpel[(i + 0) * stride + j] + fpel[(i + 0) * stride + j + 1] + 1) >> 1) + 1) >> 1));
      sad1 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i * stride + j + 1] + fpel[(i + 1) * stride + j + 1] + 1) >> 1) + ((fpel[(i + 0) * stride + j] + fpel[(i + 0) * stride + j + 1] + 1) >> 1) + 1) >> 1));
      sad2 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i * stride + j + 0] + fpel[(i + 1) * stride + j + 0] + 1) >> 1) + ((fpel[(i + 1) * stride + j] + fpel[(i + 1) * stride + j + 1] + 1) >> 1) + 1) >> 1));
      sad3 += abs(cur[i * MB_WIDTH_Y + j] - ((((fpel[i * stride + j + 1] + fpel[(i + 1) * stride + j + 1] + 1) >> 1) + ((fpel[(i + 1) * stride + j] + fpel[(i + 1) * stride + j + 1] + 1) >> 1) + 1) >> 1));
# else
      sad0 += abs(cur[i * MB_WIDTH_Y + j] - ((vpel[(i + 0) * stride + j + 0] + hpel[(i + 0) * stride + j + 0] + 1) >> 1));
      sad1 += abs(cur[i * MB_WIDTH_Y + j] - ((vpel[(i + 0) * stride + j + 1] + hpel[(i + 0) * stride + j + 0] + 1) >> 1));
      sad2 += abs(cur[i * MB_WIDTH_Y + j] - ((vpel[(i + 0) * stride + j + 0] + hpel[(i + 1) * stride + j + 0] + 1) >> 1));
      sad3 += abs(cur[i * MB_WIDTH_Y + j] - ((vpel[(i + 0) * stride + j + 1] + hpel[(i + 1) * stride + j + 0] + 1) >> 1));
# endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
}


static
void taa_h264_qp_sad8x16d4(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * fpel,
  const uint8_t * hpel,
  const uint8_t * vpel,
  const uint8_t * dpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(curr, 16);
# ifdef AVG_HPEL
  for (i = 0; i < 8; i++)
  {
    TAA_H264_ALIGN(64) uint8_t cur[16], fp0[16], fp1[16], fp2[16], fp3[16];
    _mm_store_pd((double *) cur, (_mm_set_pd (*((double *)(&curr[(i+8) * MB_WIDTH_Y])), *(((double *)(&curr[i * MB_WIDTH_Y]))))));
    _mm_store_pd((double *) fp0, (_mm_set_pd (*((double *)(&fpel[(i+8) * stride + 0])), *(((double *)(&fpel[(i+0) * stride + 0]))))));
    _mm_store_pd((double *) fp1, (_mm_set_pd (*((double *)(&fpel[(i+9) * stride + 1])), *(((double *)(&fpel[(i+1) * stride + 1]))))));
    _mm_store_pd((double *) fp2, (_mm_set_pd (*((double *)(&fpel[(i+8) * stride + 1])), *(((double *)(&fpel[(i+0) * stride + 1]))))));
    _mm_store_pd((double *) fp3, (_mm_set_pd (*((double *)(&fpel[(i+9) * stride + 0])), *(((double *)(&fpel[(i+1) * stride + 0]))))));

    for (j = 0; j < 16; j++)
    {
      sad0 += abs(cur[j] - ((((fp0[j] + fp1[j] + 1) >> 1) + ((fp0[j] + fp2[j] + 1) >> 1) + 1) >> 1));
      sad1 += abs(cur[j] - ((((fp0[j] + fp1[j] + 1) >> 1) + ((fp0[j] + fp3[j] + 1) >> 1) + 1) >> 1));
      sad2 += abs(cur[j] - ((((fp0[j] + fp1[j] + 1) >> 1) + ((fp2[j] + fp1[j] + 1) >> 1) + 1) >> 1));
      sad3 += abs(cur[j] - ((((fp0[j] + fp1[j] + 1) >> 1) + ((fp3[j] + fp1[j] + 1) >> 1) + 1) >> 1));
    }
  }
# else
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 8; j++)
    {
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((dpel[i*stride + j] + hpel[(i + 0) * stride + j + 0] + 1) >> 1));
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((dpel[i*stride + j] + vpel[(i + 0) * stride + j + 0] + 1) >> 1));
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((dpel[i*stride + j] + vpel[(i + 0) * stride + j + 1] + 1) >> 1));
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((dpel[i*stride + j] + hpel[(i + 1) * stride + j + 0] + 1) >> 1));
    }
  }
# endif
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
}

static
void taa_h264_qp_sad8x16x4(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * fpel,
  const uint8_t * hpel,
  const uint8_t * vpel,
  const uint8_t * dpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(curr, 16);
# ifdef AVG_HPEL
  for (i = 0; i < 8; i++)
  {
    TAA_H264_ALIGN(64) uint8_t cur[16], fp0[16], fp1[16], fp2[16], fp3[16];
    _mm_store_pd((double *) cur, (_mm_set_pd (*((double *)(&curr[(i+8) * MB_WIDTH_Y])), *(((double *)(&curr[i * MB_WIDTH_Y]))))));
    _mm_store_pd((double *) fp0, (_mm_set_pd (*((double *)(&fpel[(i+8) * stride + 0])), *(((double *)(&fpel[(i+0) * stride + 0]))))));
    _mm_store_pd((double *) fp1, (_mm_set_pd (*((double *)(&fpel[(i+9) * stride + 1])), *(((double *)(&fpel[(i+1) * stride + 1]))))));
    _mm_store_pd((double *) fp2, (_mm_set_pd (*((double *)(&fpel[(i+8) * stride + 1])), *(((double *)(&fpel[(i+0) * stride + 1]))))));
    _mm_store_pd((double *) fp3, (_mm_set_pd (*((double *)(&fpel[(i+9) * stride + 0])), *(((double *)(&fpel[(i+1) * stride + 0]))))));

    for (j = 0; j < 16; j++)
    {
      sad0 += abs(cur[j] - ((((fp0[j] + fp3[j] + 1) >> 1) + ((fp0[j] + fp2[j] + 1) >> 1) + 1) >> 1));
      sad1 += abs(cur[j] - ((((fp2[j] + fp1[j] + 1) >> 1) + ((fp0[j] + fp2[j] + 1) >> 1) + 1) >> 1));
      sad2 += abs(cur[j] - ((((fp0[j] + fp3[j] + 1) >> 1) + ((fp3[j] + fp1[j] + 1) >> 1) + 1) >> 1));
      sad3 += abs(cur[j] - ((((fp2[j] + fp1[j] + 1) >> 1) + ((fp3[j] + fp1[j] + 1) >> 1) + 1) >> 1));
    }
  }
# else
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 8; j++)
    {
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[(i + 0) * stride + j + 0] + hpel[(i + 0) * stride + j + 0] + 1) >> 1));
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[(i + 0) * stride + j + 1] + hpel[(i + 0) * stride + j + 0] + 1) >> 1));
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[(i + 0) * stride + j + 0] + hpel[(i + 1) * stride + j + 0] + 1) >> 1));
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[(i + 0) * stride + j + 1] + hpel[(i + 1) * stride + j + 0] + 1) >> 1));
    }
  }
# endif
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
}
#endif

static
void taa_h264_qp_sad16x8v2(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * vpel,
  const uint8_t * fpel,
  uint32_t        sad[2]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  __assume_aligned(curr, 16);
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 16; j++)
    {
#ifdef AVG_HPEL
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i+0) * stride + j]+fpel[(i+1) * stride + j] + 1)>>1) + fpel[i * stride + j] + 1) >> 1));
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i+0) * stride + j]+fpel[(i+1) * stride + j] + 1)>>1) + fpel[(i + 1) * stride + j] + 1) >> 1));
#else
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + fpel[i * stride + j] + 1) >> 1));
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + fpel[(i + 1) * stride + j] + 1) >> 1));
#endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
}

static
void taa_h264_qp_sad8x16v2(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * vpel,
  const uint8_t * fpel,
  uint32_t        sad[2]
  )
{
#ifdef AVG_HPEL
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  //__assume_aligned(curr, 16);
  __assume_aligned(curr, 8);
  for (i = 0; i < 8; i++)
  {
    TAA_H264_ALIGN(64) uint8_t avg0[16], cur[16], fp0[16], fp1[16];
    __m128i xmm0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+8) * stride])), *(((double *)(&fpel[(i+0) * stride])))));
    __m128i xmm1 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+9) * stride])), *(((double *)(&fpel[(i+1) * stride])))));
    _mm_store_si128((__m128i *) avg0, _mm_avg_epu8(xmm0, xmm1));
    _mm_store_pd((double *) cur, (_mm_set_pd (*((double *)(&curr[(i+8) * MB_WIDTH_Y])), *(((double *)(&curr[i * MB_WIDTH_Y]))))));
    _mm_store_pd((double *) fp0, (_mm_set_pd (*((double *)(&fpel[(i+8) * stride])), *(((double *)(&fpel[(i+0) * stride]))))));
    _mm_store_pd((double *) fp1, (_mm_set_pd (*((double *)(&fpel[(i+9) * stride])), *(((double *)(&fpel[(i+1) * stride]))))));
    for (j = 0; j < 16; j++)
    {
      sad0 += abs(cur[j] - ((avg0[j] + fp0[j] + 1) >> 1));
      sad1 += abs(cur[j] - ((avg0[j] + fp1[j] + 1) >> 1));
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
#else
  int i;
  __m128i sum  = _mm_setzero_si128();
  __m128i sum0 = _mm_setzero_si128();
  __m128i sum1 = _mm_setzero_si128();
  for(i = 0; i < 8; i++)
  {
    sum0 = _mm_add_epi32(sum0, _mm_sad_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&curr[MB_WIDTH_Y * (i + 0)])))), _mm_avg_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&vpel[stride * (i + 0)])))), _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[stride * (i + 1)])), *(((double *)(&fpel[stride * (i + 0)]))))))));
    sum1 = _mm_add_epi32(sum1, _mm_sad_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&curr[MB_WIDTH_Y * (i + 8)])))), _mm_avg_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&vpel[stride * (i + 8)])))), _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[stride * (i + 9)])), *(((double *)(&fpel[stride * (i + 8)]))))))));
  }
  sum = _mm_add_epi32(sum0, sum1);
  sad[0] = _mm_cvtsi128_si32(sum);
  sad[1] = _mm_cvtsi128_si32(_mm_srli_si128(sum, 8));
#endif

}

static
void taa_h264_qp_sad16x16h4(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * fpel,
  const uint8_t * hpel,
  const uint8_t * vpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(curr, 16);
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 16; j++)
    {
#ifdef AVG_HPEL
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i+0) * stride + j + 0]+fpel[(i+1) * stride + j + 0]+1)>>1) + ((fpel[(i+0) * stride + j - 1]+fpel[(i+0) * stride + j + 0] + 1)>>1) + 1) >> 1));           // up-left
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i+0) * stride + j + 0]+fpel[(i+1) * stride + j + 0]+1)>>1) + ((fpel[(i+1) * stride + j - 1]+fpel[(i+1) * stride + j + 0] + 1)>>1) + 1) >> 1));     // down-left
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i+0) * stride + j + 0]+fpel[(i+1) * stride + j + 0]+1)>>1) + ((fpel[(i+0) * stride + j + 0]+fpel[(i+0) * stride + j + 1] + 1)>>1) + 1) >> 1));       // up-right
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i+0) * stride + j + 0]+fpel[(i+1) * stride + j + 0]+1)>>1) + ((fpel[(i+1) * stride + j + 0]+fpel[(i+1) * stride + j + 1] + 1)>>1) + 1) >> 1)); // down-right
#else
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + hpel[i * stride + j] + 1) >> 1));           // up-left
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + hpel[(i + 1) * stride + j] + 1) >> 1));     // down-left
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + hpel[i * stride + j + 1] + 1) >> 1));       // up-right
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + hpel[(i + 1) * stride + j + 1] + 1) >> 1)); // down-right
#endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
}
static
void taa_h264_qp_sad16x8h4(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * fpel,
  const uint8_t * hpel,
  const uint8_t * vpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(curr, 16);
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 16; j++)
    {
#ifdef AVG_HPEL
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i+0) * stride + j + 0]+fpel[(i+1) * stride + j + 0]+1)>>1) + ((fpel[(i+0) * stride + j - 1]+fpel[(i+0) * stride + j + 0] + 1)>>1) + 1) >> 1));           // up-left
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i+0) * stride + j + 0]+fpel[(i+1) * stride + j + 0]+1)>>1) + ((fpel[(i+1) * stride + j - 1]+fpel[(i+1) * stride + j + 0] + 1)>>1) + 1) >> 1));     // down-left
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i+0) * stride + j + 0]+fpel[(i+1) * stride + j + 0]+1)>>1) + ((fpel[(i+0) * stride + j + 0]+fpel[(i+0) * stride + j + 1] + 1)>>1) + 1) >> 1));       // up-right
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i+0) * stride + j + 0]+fpel[(i+1) * stride + j + 0]+1)>>1) + ((fpel[(i+1) * stride + j + 0]+fpel[(i+1) * stride + j + 1] + 1)>>1) + 1) >> 1)); // down-right
#else
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + hpel[i * stride + j] + 1) >> 1));           // up-left
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + hpel[(i + 1) * stride + j] + 1) >> 1));     // down-left
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + hpel[i * stride + j + 1] + 1) >> 1));       // up-right
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + hpel[(i + 1) * stride + j + 1] + 1) >> 1)); // down-right
#endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
}

static
void taa_h264_qp_sad8x16h4(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * fpel,
  const uint8_t * hpel,
  const uint8_t * vpel,
  uint32_t        sad[4]
  )
{
#ifdef AVG_HPEL
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  //__assume_aligned(curr, 16);
  __assume_aligned(curr, 8);
  for (i = 0; i < 8; i++)
  {
    TAA_H264_ALIGN(64) uint8_t avg0[16], avg1[16], avg2[16], avg3[16],  avg4[16], cur[16];
    __m128i xmm0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+8) * stride + 0])), *(((double *)(&fpel[(i+0) * stride + 0])))));
    __m128i xmm1 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+9) * stride + 0])), *(((double *)(&fpel[(i+1) * stride + 0])))));
    __m128i xmm2 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+8) * stride - 1])), *(((double *)(&fpel[(i+0) * stride - 1])))));
    __m128i xmm3 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+9) * stride - 1])), *(((double *)(&fpel[(i+1) * stride - 1])))));
    __m128i xmm4 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+8) * stride + 1])), *(((double *)(&fpel[(i+0) * stride + 1])))));
    __m128i xmm5 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+9) * stride + 1])), *(((double *)(&fpel[(i+1) * stride + 1])))));

    _mm_store_si128((__m128i *) avg0, _mm_avg_epu8(xmm0, xmm1));
    _mm_store_si128((__m128i *) avg1, _mm_avg_epu8(xmm2, xmm0));
    _mm_store_si128((__m128i *) avg2, _mm_avg_epu8(xmm3, xmm1));
    _mm_store_si128((__m128i *) avg3, _mm_avg_epu8(xmm0, xmm4));
    _mm_store_si128((__m128i *) avg4, _mm_avg_epu8(xmm1, xmm5));

    _mm_store_pd((double *) cur, (_mm_set_pd (*((double *)(&curr[(i+8) * MB_WIDTH_Y])), *(((double *)(&curr[i * MB_WIDTH_Y]))))));

    for (j = 0; j < 16; j++)
    {
      sad0 += abs(cur[j] - ((avg0[j] + avg1[j] + 1) >> 1));    // up-left
      sad1 += abs(cur[j] - ((avg0[j] + avg2[j] + 1) >> 1));    // down-left
      sad2 += abs(cur[j] - ((avg0[j] + avg3[j] + 1) >> 1));    // up-right
      sad3 += abs(cur[j] - ((avg0[j] + avg4[j] + 1) >> 1));    // down-right
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
#else
  int i;
  __m128i sum0 = _mm_setzero_si128();
  __m128i sum1 = _mm_setzero_si128();
  __m128i sum2 = _mm_setzero_si128();
  __m128i sum3 = _mm_setzero_si128();
  for(i = 0; i < 8; i++)
  {
    sum0 = _mm_add_epi32(sum0, _mm_sad_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&curr[MB_WIDTH_Y * (i + 0)])))), _mm_avg_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&vpel[stride * (i + 0)])))), _mm_castpd_si128(_mm_set_pd (*((double *)(&hpel[stride * (i + 0) + 1])), *(((double *)(&hpel[stride * (i + 0)]))))))));
    sum1 = _mm_add_epi32(sum1, _mm_sad_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&curr[MB_WIDTH_Y * (i + 8)])))), _mm_avg_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&vpel[stride * (i + 8)])))), _mm_castpd_si128(_mm_set_pd (*((double *)(&hpel[stride * (i + 8) + 1])), *(((double *)(&hpel[stride * (i + 8)]))))))));
    sum2 = _mm_add_epi32(sum2, _mm_sad_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&curr[MB_WIDTH_Y * (i + 0)])))), _mm_avg_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&vpel[stride * (i + 0)])))), _mm_castpd_si128(_mm_set_pd (*((double *)(&hpel[stride * (i + 1) + 1])), *(((double *)(&hpel[stride * (i + 1)]))))))));
    sum3 = _mm_add_epi32(sum3, _mm_sad_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&curr[MB_WIDTH_Y * (i + 8)])))), _mm_avg_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&vpel[stride * (i + 8)])))), _mm_castpd_si128(_mm_set_pd (*((double *)(&hpel[stride * (i + 9) + 1])), *(((double *)(&hpel[stride * (i + 9)]))))))));
  }
  sum0 = _mm_add_epi32(sum0, sum1);
  sum2 = _mm_add_epi32(sum2, sum3);
  sum1 = _mm_srli_si128(sum0, 8);
  sum3 = _mm_srli_si128(sum2, 8);

  sad[0] = _mm_cvtsi128_si32(sum0);
  sad[1] = _mm_cvtsi128_si32(sum2);
  sad[2] = _mm_cvtsi128_si32(sum1);
  sad[3] = _mm_cvtsi128_si32(sum3);
#endif
}

static
void taa_h264_qp_sad16x16x4d4(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * fpel,
  const uint8_t * vpel,
  const uint8_t * hpel,
  uint32_t        sad[8]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  uint32_t sad4 = 0;
  uint32_t sad5 = 0;
  uint32_t sad6 = 0;
  uint32_t sad7 = 0;
  __assume_aligned(curr, 16);
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 16; j++)
    {
#ifdef AVG_HPEL
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + ((fpel[(i-1) * stride + j ] + fpel[i * stride + j ] +1)>>1) + 1) >> 1));   // up
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + ((fpel[i * stride + j ] + fpel[(i+1) * stride + j ] +1)>>1) + 1) >> 1));   // down
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + ((fpel[i * stride + j - 1] + fpel[i * stride + j ] +1)>>1) + 1) >> 1));   // left
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + ((fpel[i * stride + j ] + fpel[i * stride + j + 1] +1)>>1) + 1) >> 1));   // right
      sad4 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i-1) * stride + j]+fpel[i * stride + j]+1)>>1) + ((fpel[i* stride + j-1]+fpel[i * stride + j]+1)>>1) + 1) >> 1));   // up-left
      sad5 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i-1) * stride + j]+fpel[i * stride + j]+1)>>1) + ((fpel[i* stride + j]+fpel[i * stride + j+1]+1)>>1) + 1) >> 1));   // up-right
      sad6 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i * stride + j]+fpel[(i+1) * stride + j]+1)>>1) + ((fpel[i* stride + j-1]+fpel[i * stride + j]+1)>>1) + 1) >> 1));   // down-left
      sad7 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i * stride + j]+fpel[(i+1) * stride + j]+1)>>1) + ((fpel[i* stride + j]+fpel[i * stride + j+1]+1)>>1) + 1) >> 1));   // down-right
#else
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + vpel[i * stride + j] + 1) >> 1));       // up
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + vpel[(i + 1) * stride + j] + 1) >> 1)); // down
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + hpel[i * stride + j] + 1) >> 1));       // left
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + hpel[i * stride + j + 1] + 1) >> 1));   // right
      sad4 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + hpel[i * stride + j] + 1) >> 1));           // up-left
      sad5 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + hpel[i * stride + j + 1] + 1) >> 1));       // up-right
      sad6 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[(i + 1) * stride + j] + hpel[i * stride + j] + 1) >> 1));     // down-left
      sad7 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[(i + 1) * stride + j] + hpel[i * stride + j + 1] + 1) >> 1)); // down-right
#endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
  sad[4] = sad4;
  sad[5] = sad5;
  sad[6] = sad6;
  sad[7] = sad7;
}

static
void taa_h264_qp_sad16x8x4d4(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * fpel,
  const uint8_t * vpel,
  const uint8_t * hpel,
  uint32_t        sad[8]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  uint32_t sad4 = 0;
  uint32_t sad5 = 0;
  uint32_t sad6 = 0;
  uint32_t sad7 = 0;
  __assume_aligned(curr, 16);
#ifdef AVG_HPEL
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 16; j++)
    {
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + ((fpel[(i-1) * stride + j ] + fpel[i * stride + j ] +1)>>1) + 1) >> 1));   // up
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + ((fpel[i * stride + j ] + fpel[(i+1) * stride + j ] +1)>>1) + 1) >> 1));   // down
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + ((fpel[i * stride + j - 1] + fpel[i * stride + j ] +1)>>1) + 1) >> 1));   // left
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + ((fpel[i * stride + j ] + fpel[i * stride + j + 1] +1)>>1) + 1) >> 1));   // right
      sad4 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i-1) * stride + j]+fpel[i * stride + j]+1)>>1) + ((fpel[i* stride + j-1]+fpel[i * stride + j]+1)>>1) + 1) >> 1));   // up-left
      sad5 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[(i-1) * stride + j]+fpel[i * stride + j]+1)>>1) + ((fpel[i* stride + j]+fpel[i * stride + j+1]+1)>>1) + 1) >> 1));   // up-right
      sad6 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i * stride + j]+fpel[(i+1) * stride + j]+1)>>1) + ((fpel[i* stride + j-1]+fpel[i * stride + j]+1)>>1) + 1) >> 1));   // down-left
      sad7 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i * stride + j]+fpel[(i+1) * stride + j]+1)>>1) + ((fpel[i* stride + j]+fpel[i * stride + j+1]+1)>>1) + 1) >> 1));   // down-right
    }
  }
#else
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 16; j++)
    {
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + vpel[i * stride + j] + 1) >> 1));       // up
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + vpel[(i + 1) * stride + j] + 1) >> 1)); // down
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + hpel[i * stride + j] + 1) >> 1));       // left
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + hpel[i * stride + j + 1] + 1) >> 1));   // right
      sad4 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + hpel[i * stride + j] + 1) >> 1));           // up-left
      sad5 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + hpel[i * stride + j + 1] + 1) >> 1));       // up-right
      sad6 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[(i + 1) * stride + j] + hpel[i * stride + j] + 1) >> 1));     // down-left
      sad7 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[(i + 1) * stride + j] + hpel[i * stride + j + 1] + 1) >> 1)); // down-right
    }
  }
#endif
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
  sad[4] = sad4;
  sad[5] = sad5;
  sad[6] = sad6;
  sad[7] = sad7;
}

static
void taa_h264_qp_sad8x16x4d4(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * fpel,
  const uint8_t * vpel,
  const uint8_t * hpel,
  uint32_t        sad[8]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  uint32_t sad4 = 0;
  uint32_t sad5 = 0;
  uint32_t sad6 = 0;
  uint32_t sad7 = 0;
  //__assume_aligned(curr, 16);
  __assume_aligned(curr, 8);
#ifdef AVG_HPEL
  for (i = 0; i < 8; i++)
  {
    TAA_H264_ALIGN(64) uint8_t avg0[16], avg1[16], avg2[16], avg3[16], cur[16], full[16];
    __m128i xmm0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+8-1) * stride+0])), *(((double *)(&fpel[(i-1) * stride+0])))));
    __m128i xmm1 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+8+0) * stride+0])), *(((double *)(&fpel[(i+0) * stride+0])))));
    __m128i xmm2 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+8+0) * stride-1])), *(((double *)(&fpel[(i+0) * stride-1])))));
    __m128i xmm3 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+8+1) * stride+0])), *(((double *)(&fpel[(i+1) * stride+0])))));
    __m128i xmm4 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+8+0) * stride+1])), *(((double *)(&fpel[(i+0) * stride+1])))));

    _mm_store_si128((__m128i *) avg0, _mm_avg_epu8(xmm0, xmm1));
    _mm_store_si128((__m128i *) avg1, _mm_avg_epu8(xmm2, xmm1));
    _mm_store_si128((__m128i *) avg2, _mm_avg_epu8(xmm3, xmm1));
    _mm_store_si128((__m128i *) avg3, _mm_avg_epu8(xmm4, xmm1));

    _mm_store_pd((double *) cur, (_mm_set_pd (*((double *)(&curr[(i+8) * MB_WIDTH_Y])), *(((double *)(&curr[i * MB_WIDTH_Y]))))));
    _mm_store_pd((double *) full, _mm_castsi128_pd(xmm1));

    for (j = 0; j < 16; j++)
    {
      sad4 += abs(cur[j] - ((avg0[j] + avg1[j] + 1) >> 1));   // up-left
      sad5 += abs(cur[j] - ((avg0[j] + avg3[j] + 1) >> 1));   // up-right
      sad6 += abs(cur[j] - ((avg2[j] + avg1[j] + 1) >> 1));   // down-left
      sad7 += abs(cur[j] - ((avg2[j] + avg3[j] + 1) >> 1));   // down-right
    }
    for (j = 0; j < 16; j++)
    {
      sad0 += abs(cur[j] - ((full[j] + avg0[j] + 1) >> 1));   // up
      sad1 += abs(cur[j] - ((full[j] + avg2[j] + 1) >> 1));   // down
      sad2 += abs(cur[j] - ((full[j] + avg1[j] + 1) >> 1));   // left
      sad3 += abs(cur[j] - ((full[j] + avg3[j] + 1) >> 1));   // right
    }
  }
#else
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 8; j++)
    {
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + vpel[i * stride + j] + 1) >> 1));       // up
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + vpel[(i + 1) * stride + j] + 1) >> 1)); // down
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + hpel[i * stride + j] + 1) >> 1));       // left
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[i * stride + j] + hpel[i * stride + j + 1] + 1) >> 1));   // right
      sad4 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + hpel[i * stride + j] + 1) >> 1));           // up-left
      sad5 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[i * stride + j] + hpel[i * stride + j + 1] + 1) >> 1));       // up-right
      sad6 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[(i + 1) * stride + j] + hpel[i * stride + j] + 1) >> 1));     // down-left
      sad7 += abs(curr[i * MB_WIDTH_Y + j] - ((vpel[(i + 1) * stride + j] + hpel[i * stride + j + 1] + 1) >> 1)); // down-right
    }
  }
#endif
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
  sad[4] = sad4;
  sad[5] = sad5;
  sad[6] = sad6;
  sad[7] = sad7;
}

static
void taa_h264_qp_sad16x16v4(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * fpel,
  const uint8_t * hpel,
  const uint8_t * vpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(curr, 16);
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 16; j++)
    {
#ifdef AVG_HPEL
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i* stride + j]+fpel[i * stride + j + 1]+1)>>1) + ((fpel[(i-1) * stride + j + 0]+fpel[(i+0) * stride + j + 0] + 1)>>1) + 1) >> 1)); // up-left
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i* stride + j]+fpel[i * stride + j + 1]+1)>>1) + ((fpel[(i-1) * stride + j + 1]+fpel[(i+0) * stride + j + 1] + 1)>>1) + 1) >> 1)); // up-right
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i* stride + j]+fpel[i * stride + j + 1]+1)>>1) + ((fpel[(i+0) * stride + j + 0]+fpel[(i+1) * stride + j + 0] + 1)>>1) + 1) >> 1)); // down-left
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i* stride + j]+fpel[i * stride + j + 1]+1)>>1) + ((fpel[(i+0) * stride + j + 1]+fpel[(i+1) * stride + j + 1] + 1)>>1) + 1) >> 1)); // down-right
#else
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((hpel[i * stride + j] + vpel[i * stride + j] + 1) >> 1));           // up-left
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((hpel[i * stride + j] + vpel[i * stride + j + 1] + 1) >> 1));       // up-right
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((hpel[i * stride + j] + vpel[(i + 1) * stride + j] + 1) >> 1));     // down-left
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((hpel[i * stride + j] + vpel[(i + 1) * stride + j + 1] + 1) >> 1)); // down-right
#endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
}

static
void taa_h264_qp_sad16x8v4(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * fpel,
  const uint8_t * hpel,
  const uint8_t * vpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(curr, 16);
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 16; j++)
    {
#ifdef AVG_HPEL
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i* stride + j]+fpel[i * stride + j + 1]+1)>>1) + ((fpel[(i-1) * stride + j + 0]+fpel[(i+0) * stride + j + 0] + 1)>>1) + 1) >> 1)); // up-left
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i* stride + j]+fpel[i * stride + j + 1]+1)>>1) + ((fpel[(i-1) * stride + j + 1]+fpel[(i+0) * stride + j + 1] + 1)>>1) + 1) >> 1)); // up-right
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i* stride + j]+fpel[i * stride + j + 1]+1)>>1) + ((fpel[(i+0) * stride + j + 0]+fpel[(i+1) * stride + j + 0] + 1)>>1) + 1) >> 1)); // down-left
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((((fpel[i* stride + j]+fpel[i * stride + j + 1]+1)>>1) + ((fpel[(i+0) * stride + j + 1]+fpel[(i+1) * stride + j + 1] + 1)>>1) + 1) >> 1)); // down-right
#else
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((hpel[i * stride + j] + vpel[i * stride + j] + 1) >> 1));           // up-left
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((hpel[i * stride + j] + vpel[i * stride + j + 1] + 1) >> 1));       // up-right
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((hpel[i * stride + j] + vpel[(i + 1) * stride + j] + 1) >> 1));     // down-left
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((hpel[i * stride + j] + vpel[(i + 1) * stride + j + 1] + 1) >> 1)); // down-right
#endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
}

static
void taa_h264_qp_sad8x16v4(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * fpel,
  const uint8_t * hpel,
  const uint8_t * vpel,
  uint32_t        sad[4]
  )
{
#ifdef AVG_HPEL
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  //__assume_aligned(curr, 16);
  __assume_aligned(curr, 8);
  for (i = 0; i < 8; i++)
  {
    TAA_H264_ALIGN(64) uint8_t avg0[16], avg1[16], avg2[16], avg3[16], avg4[16], cur[16];
    __m128i xmm0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+8) * stride + 0])), *(((double *)(&fpel[(i+0) * stride + 0])))));
    __m128i xmm1 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+8) * stride + 1])), *(((double *)(&fpel[(i+0) * stride + 1])))));
    __m128i xmm2 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+7) * stride + 0])), *(((double *)(&fpel[(i-1) * stride + 0])))));
    __m128i xmm4 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+7) * stride + 1])), *(((double *)(&fpel[(i-1) * stride + 1])))));
    __m128i xmm7 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+9) * stride + 0])), *(((double *)(&fpel[(i+1) * stride + 0])))));
    __m128i xmm9 = _mm_castpd_si128(_mm_set_pd (*((double *)(&fpel[(i+9) * stride + 1])), *(((double *)(&fpel[(i+1) * stride + 1])))));
    _mm_store_si128((__m128i *) avg0, _mm_avg_epu8(xmm0, xmm1));
    _mm_store_si128((__m128i *) avg1, _mm_avg_epu8(xmm2, xmm0));
    _mm_store_si128((__m128i *) avg2, _mm_avg_epu8(xmm4, xmm1));
    _mm_store_si128((__m128i *) avg3, _mm_avg_epu8(xmm0, xmm7));
    _mm_store_si128((__m128i *) avg4, _mm_avg_epu8(xmm1, xmm9));
    _mm_store_pd((double *) cur, (_mm_set_pd (*((double *)(&curr[(i+8) * MB_WIDTH_Y])), *(((double *)(&curr[i * MB_WIDTH_Y]))))));

    for (j = 0; j < 16; j++)
    {
      sad0 += abs(cur[j] - ((avg0[j] + avg1[j] + 1) >> 1)); // up-left
      sad1 += abs(cur[j] - ((avg0[j] + avg2[j] + 1) >> 1)); // up-right
      sad2 += abs(cur[j] - ((avg0[j] + avg3[j] + 1) >> 1)); // down-left
      sad3 += abs(cur[j] - ((avg0[j] + avg4[j] + 1) >> 1)); // down-right
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
#else
  int i;
  __m128i sum0 = _mm_setzero_si128();
  __m128i sum1 = _mm_setzero_si128();
  __m128i sum2 = _mm_setzero_si128();
  __m128i sum3 = _mm_setzero_si128();
  for(i = 0; i < 8; i++)
  {
    sum0 = _mm_add_epi32(sum0, _mm_sad_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&curr[MB_WIDTH_Y * (i + 0)])))), _mm_avg_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&hpel[stride * (i + 0)])))), _mm_castpd_si128(_mm_set_pd (*((double *)(&vpel[stride * (i + 0) + 1])), *(((double *)(&vpel[stride * (i + 0)]))))))));
    sum1 = _mm_add_epi32(sum1, _mm_sad_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&curr[MB_WIDTH_Y * (i + 8)])))), _mm_avg_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&hpel[stride * (i + 8)])))), _mm_castpd_si128(_mm_set_pd (*((double *)(&vpel[stride * (i + 8) + 1])), *(((double *)(&vpel[stride * (i + 8)]))))))));
    sum2 = _mm_add_epi32(sum2, _mm_sad_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&curr[MB_WIDTH_Y * (i + 0)])))), _mm_avg_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&hpel[stride * (i + 0)])))), _mm_castpd_si128(_mm_set_pd (*((double *)(&vpel[stride * (i + 1) + 1])), *(((double *)(&vpel[stride * (i + 1)]))))))));
    sum3 = _mm_add_epi32(sum3, _mm_sad_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&curr[MB_WIDTH_Y * (i + 8)])))), _mm_avg_epu8(_mm_castpd_si128(_mm_set1_pd (*((double *)(&hpel[stride * (i + 8)])))), _mm_castpd_si128(_mm_set_pd (*((double *)(&vpel[stride * (i + 9) + 1])), *(((double *)(&vpel[stride * (i + 9)]))))))));

  }
  sum0 = _mm_add_epi32(sum0, sum1);
  sum2 = _mm_add_epi32(sum2, sum3);
  sum1 = _mm_srli_si128(sum0, 8);
  sum3 = _mm_srli_si128(sum2, 8);

  sad[0] = _mm_cvtsi128_si32(sum0);
  sad[1] = _mm_cvtsi128_si32(sum1);
  sad[2] = _mm_cvtsi128_si32(sum2);
  sad[3] = _mm_cvtsi128_si32(sum3);
#endif
}

#if HPEL_DIAG_ENABLED

static
void taa_h264_square_sad16x16(
  const int       stride,
  const uint8_t * cur,
  const uint8_t * fpel,
  const uint8_t * dpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(cur, 16);
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 16; j++)
    {
# ifdef AVG_HPEL
      sad0 += abs(cur[i * MB_WIDTH_Y + j] - ((fpel[(i - 1) * stride + j - 1] + fpel[(i + 0) * stride + j + 0] + 1) >> 1));
      sad1 += abs(cur[i * MB_WIDTH_Y + j] - ((fpel[(i - 1) * stride + j + 0] + fpel[(i + 0) * stride + j + 1] + 1) >> 1));
      sad2 += abs(cur[i * MB_WIDTH_Y + j] - ((fpel[(i + 0) * stride + j - 1] + fpel[(i + 1) * stride + j + 0] + 1) >> 1));
      sad3 += abs(cur[i * MB_WIDTH_Y + j] - ((fpel[(i + 0) * stride + j + 0] + fpel[(i + 1) * stride + j + 1] + 1) >> 1));
# else
      sad0 += abs(cur[i * MB_WIDTH_Y + j] - dpel[(i - 1) * stride + j - 1]);
      sad1 += abs(cur[i * MB_WIDTH_Y + j] - dpel[(i - 1) * stride + j + 0]);
      sad2 += abs(cur[i * MB_WIDTH_Y + j] - dpel[(i + 0) * stride + j - 1]);
      sad3 += abs(cur[i * MB_WIDTH_Y + j] - dpel[(i + 0) * stride + j + 0]);
# endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
}

static
void taa_h264_square_sad16x8(
  const int       stride,
  const uint8_t * cur,
  const uint8_t * fpel,
  const uint8_t * dpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(cur, 16);
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 16; j++)
    {
# ifdef AVG_HPEL
      sad0 += abs(cur[i * MB_WIDTH_Y + j] - ((fpel[(i - 1) * stride + j - 1] + fpel[(i + 0) * stride + j + 0] + 1) >> 1));
      sad1 += abs(cur[i * MB_WIDTH_Y + j] - ((fpel[(i - 1) * stride + j + 0] + fpel[(i + 0) * stride + j + 1] + 1) >> 1));
      sad2 += abs(cur[i * MB_WIDTH_Y + j] - ((fpel[(i + 0) * stride + j - 1] + fpel[(i + 1) * stride + j + 0] + 1) >> 1));
      sad3 += abs(cur[i * MB_WIDTH_Y + j] - ((fpel[(i + 0) * stride + j + 0] + fpel[(i + 1) * stride + j + 1] + 1) >> 1));
# else
      sad0 += abs(cur[i * MB_WIDTH_Y + j] - dpel[(i - 1) * stride + j - 1]);
      sad1 += abs(cur[i * MB_WIDTH_Y + j] - dpel[(i - 1) * stride + j + 0]);
      sad2 += abs(cur[i * MB_WIDTH_Y + j] - dpel[(i + 0) * stride + j - 1]);
      sad3 += abs(cur[i * MB_WIDTH_Y + j] - dpel[(i + 0) * stride + j + 0]);
# endif
    }
  }
  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
}

static
void taa_h264_square_sad8x16(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * fpel,
  const uint8_t * dpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(curr, 16);
# ifdef AVG_HPEL
  for (i = 0; i < 8; i++)
  {
    TAA_H264_ALIGN(64) uint8_t cur[16], fp0[16], fp1[16], fp2[16], fp3[16], fp4[16], fp5[16], fp6[16];
    _mm_store_pd((double *) cur, (_mm_set_pd (*((double *)(&curr[(i+8) * MB_WIDTH_Y])), *(((double *)(&curr[i * MB_WIDTH_Y]))))));
    _mm_store_pd((double *) fp0, (_mm_set_pd (*((double *)(&fpel[(i+8) * stride + 0])), *(((double *)(&fpel[(i+0) * stride + 0]))))));
    _mm_store_pd((double *) fp1, (_mm_set_pd (*((double *)(&fpel[(i+9) * stride + 1])), *(((double *)(&fpel[(i+1) * stride + 1]))))));
    _mm_store_pd((double *) fp2, (_mm_set_pd (*((double *)(&fpel[(i+8) * stride + 1])), *(((double *)(&fpel[(i+0) * stride + 1]))))));
    _mm_store_pd((double *) fp3, (_mm_set_pd (*((double *)(&fpel[(i+9) * stride + 0])), *(((double *)(&fpel[(i+1) * stride + 0]))))));
    _mm_store_pd((double *) fp4, (_mm_set_pd (*((double *)(&fpel[(i+7) * stride - 1])), *(((double *)(&fpel[(i-1) * stride - 1]))))));
    _mm_store_pd((double *) fp5, (_mm_set_pd (*((double *)(&fpel[(i+7) * stride + 0])), *(((double *)(&fpel[(i-1) * stride + 0]))))));
    _mm_store_pd((double *) fp6, (_mm_set_pd (*((double *)(&fpel[(i+8) * stride - 1])), *(((double *)(&fpel[(i+0) * stride - 1]))))));

    for (j = 0; j < 16; j++)
    {
      sad0 += abs(cur[j] - ((fp4[j] + fp0[j] + 1) >> 1));
      sad1 += abs(cur[j] - ((fp5[j] + fp2[j] + 1) >> 1));
      sad2 += abs(cur[j] - ((fp6[j] + fp3[j] + 1) >> 1));
      sad3 += abs(cur[j] - ((fp0[j] + fp1[j] + 1) >> 1));
    }
  }
# else
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 8; j++)
    {
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - dpel[(i - 1) * stride + j - 1]);
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - dpel[(i - 1) * stride + j + 0]);
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - dpel[(i + 0) * stride + j - 1]);
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - dpel[(i + 0) * stride + j + 0]);
    }
  }
# endif

  sad[0] = sad0;
  sad[1] = sad1;
  sad[2] = sad2;
  sad[3] = sad3;
}
#endif // HPEL_DIAG_ENABLED


static
void taa_h264_diamond_sad16x16(
  const int       stride,
  const uint8_t * cur,
  const uint8_t * fpel,
  const uint8_t * vpel,
  const uint8_t * hpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(cur, 16);
  for (i = 0; i < 16; i++)
  {
    for (j = 0; j < 16; j++)
    {
#ifdef AVG_HPEL
      sad0 += abs(cur[i * MB_WIDTH_Y + j] - ((fpel[(i+0) * stride + j - 1] + fpel[(i+0) * stride + j + 0] + 1) >> 1));
      sad1 += abs(cur[i * MB_WIDTH_Y + j] - ((fpel[(i-1) * stride + j + 0] + fpel[(i+0) * stride + j + 0] + 1) >> 1));
      sad2 += abs(cur[i * MB_WIDTH_Y + j] - ((fpel[(i+0) * stride + j + 0] + fpel[(i+0) * stride + j + 1] + 1) >> 1));
      sad3 += abs(cur[i * MB_WIDTH_Y + j] - ((fpel[(i+0) * stride + j + 0] + fpel[(i+1) * stride + j + 0] + 1) >> 1));
#else
      sad0 += abs(cur[i * MB_WIDTH_Y + j] - hpel[i * stride + j]);
      sad1 += abs(cur[i * MB_WIDTH_Y + j] - vpel[i * stride + j]);
      sad2 += abs(cur[i * MB_WIDTH_Y + j] - hpel[i * stride + j + 1]);
      sad3 += abs(cur[i * MB_WIDTH_Y + j] - vpel[i * stride + j + stride]);
#endif
    }
  }
  sad[2] = sad0;
  sad[0] = sad1;
  sad[3] = sad2;
  sad[1] = sad3;
}
static
void taa_h264_diamond_sad16x8(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * fpel,
  const uint8_t * vpel,
  const uint8_t * hpel,
  uint32_t        sad[4]
  )
{
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  __assume_aligned(curr, 16);
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 16; j++)
    {
#ifdef AVG_HPEL
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[(i+0) * stride + j - 1] + fpel[(i+0) * stride + j + 0] + 1) >> 1));
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[(i-1) * stride + j + 0] + fpel[(i+0) * stride + j + 0] + 1) >> 1));
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[(i+0) * stride + j + 0] + fpel[(i+0) * stride + j + 1] + 1) >> 1));
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - ((fpel[(i+0) * stride + j + 0] + fpel[(i+1) * stride + j + 0] + 1) >> 1));
#else
      sad0 += abs(curr[i * MB_WIDTH_Y + j] - hpel[i * stride + j]);
      sad1 += abs(curr[i * MB_WIDTH_Y + j] - vpel[i * stride + j]);
      sad2 += abs(curr[i * MB_WIDTH_Y + j] - hpel[i * stride + j + 1]);
      sad3 += abs(curr[i * MB_WIDTH_Y + j] - vpel[i * stride + j + stride]);
#endif
    }
  }
  sad[2] = sad0;
  sad[0] = sad1;
  sad[3] = sad2;
  sad[1] = sad3;
}

static
void taa_h264_diamond_sad8x16(
  const int       stride,
  const uint8_t * curr,
  const uint8_t * fpel,
  const uint8_t * vpel,
  const uint8_t * hpel,
  uint32_t        sad[4]
  )
{
#ifdef AVG_HPEL
  int i, j;
  uint32_t sad0 = 0;
  uint32_t sad1 = 0;
  uint32_t sad2 = 0;
  uint32_t sad3 = 0;
  //__assume_aligned(curr, 16);
  __assume_aligned(curr, 8);

  for(i = 0; i < 8; i++)
  {
    TAA_H264_ALIGN(64) uint8_t mem0[16], mem1[16], mem2[16], mem3[16], mem4[16], cur[16];

    _mm_store_pd((double *) mem0, _mm_set_pd (*((double *)(&fpel[stride     * (i+8) - 1])), *(((double *)(&fpel[stride     * (i + 0) - 1])))));
    _mm_store_pd((double *) mem1, _mm_set_pd (*((double *)(&fpel[stride     * (i+7) + 0])), *(((double *)(&fpel[stride     * (i - 1) + 0])))));
    _mm_store_pd((double *) mem2, _mm_set_pd (*((double *)(&fpel[stride     * (i+8) + 0])), *(((double *)(&fpel[stride     * (i + 0) + 0])))));
    _mm_store_pd((double *) mem3, _mm_set_pd (*((double *)(&fpel[stride     * (i+8) + 1])), *(((double *)(&fpel[stride     * (i + 0) + 1])))));
    _mm_store_pd((double *) mem4, _mm_set_pd (*((double *)(&fpel[stride     * (i+9) + 0])), *(((double *)(&fpel[stride     * (i + 1) + 0])))));
    _mm_store_pd((double *) cur,  _mm_set_pd (*((double *)(&curr[MB_WIDTH_Y * (i+8) + 0])), *(((double *)(&curr[MB_WIDTH_Y * (i + 0) + 0])))));

    for (j = 0; j < 16; j++)
    {
      sad0 += abs(cur[j] - ((mem0[j]+mem2[j]+1)>>1));
      sad1 += abs(cur[j] - ((mem1[j]+mem2[j]+1)>>1));
      sad2 += abs(cur[j] - ((mem2[j]+mem3[j]+1)>>1));
      sad3 += abs(cur[j] - ((mem2[j]+mem4[j]+1)>>1));
    }
  }
  sad[2] = sad0;
  sad[0] = sad1;
  sad[3] = sad2;
  sad[1] = sad3;
#else
  int i;
  __m128i sum0 = _mm_setzero_si128();
  __m128i sum1 = _mm_setzero_si128();
  __m128i sum2 = _mm_setzero_si128();
  __m128i sum3 = _mm_setzero_si128();
  for(i = 0; i < 8; i++)
  {
    __m128i xmm1 = _mm_castpd_si128(_mm_set_pd (*((double *)(&curr[MB_WIDTH_Y * (i + 8)])), *((double *)(&curr[MB_WIDTH_Y * (i + 0)]))));
    sum0 = _mm_add_epi32(sum0, _mm_sad_epu8(xmm1, _mm_castpd_si128(_mm_set_pd (*((double *)(&hpel[stride * (i + 8) + 0])), *(((double *)(&hpel[stride * (i + 0) + 0])))))));
    sum1 = _mm_add_epi32(sum1, _mm_sad_epu8(xmm1, _mm_castpd_si128(_mm_set_pd (*((double *)(&vpel[stride * (i + 8) + 0])), *(((double *)(&vpel[stride * (i + 0) + 0])))))));
    sum2 = _mm_add_epi32(sum2, _mm_sad_epu8(xmm1, _mm_castpd_si128(_mm_set_pd (*((double *)(&hpel[stride * (i + 8) + 1])), *(((double *)(&hpel[stride * (i + 0) + 1])))))));
    sum3 = _mm_add_epi32(sum3, _mm_sad_epu8(xmm1, _mm_castpd_si128(_mm_set_pd (*((double *)(&vpel[stride * (i + 9) + 0])), *(((double *)(&vpel[stride * (i + 1) + 0])))))));
  }
  sum0 = _mm_add_epi32(sum0, _mm_srli_si128(sum0, 8));
  sum1 = _mm_add_epi32(sum1, _mm_srli_si128(sum1, 8));
  sum2 = _mm_add_epi32(sum2, _mm_srli_si128(sum2, 8));
  sum3 = _mm_add_epi32(sum3, _mm_srli_si128(sum3, 8));

  sad[2] = _mm_cvtsi128_si32(sum0);
  sad[0] = _mm_cvtsi128_si32(sum1);
  sad[3] = _mm_cvtsi128_si32(sum2);
  sad[1] = _mm_cvtsi128_si32(sum3);
#endif
}

#ifdef AVG_HPEL
# pragma warning (default:869)
#endif



static uint32_t taa_h264_split_sad_4x8x8(
  const int       stride,
  const uint8_t * cur,
  const uint8_t * ref,
  uint16_t        sad[4]
  )
{
  __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
  __m128i sum0, sum1;
  sum0 = _mm_setzero_si128();
  sum1 = _mm_setzero_si128();

  xmm0 = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[0 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[0 * stride])));
  xmm1 = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[1 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[1 * stride])));
  xmm2 = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[2 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[2 * stride])));
  xmm3 = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[3 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[3 * stride])));
  xmm4 = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[4 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[4 * stride])));
  xmm5 = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[5 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[5 * stride])));
  xmm6 = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[6 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[6 * stride])));
  xmm7 = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[7 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[7 * stride])));

  sum0 = _mm_add_epi16(sum0, xmm0);
  sum0 = _mm_add_epi16(sum0, xmm1);
  sum0 = _mm_add_epi16(sum0, xmm2);
  sum0 = _mm_add_epi16(sum0, xmm3);
  sum0 = _mm_add_epi16(sum0, xmm4);
  sum0 = _mm_add_epi16(sum0, xmm5);
  sum0 = _mm_add_epi16(sum0, xmm6);
  sum0 = _mm_add_epi16(sum0, xmm7);

  xmm8  = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[ 8 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[ 8 * stride])));
  xmm9  = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[ 9 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[ 9 * stride])));
  xmm10 = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[10 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[10 * stride])));
  xmm11 = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[11 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[11 * stride])));
  xmm12 = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[12 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[12 * stride])));
  xmm13 = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[13 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[13 * stride])));
  xmm14 = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[14 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[14 * stride])));
  xmm15 = _mm_sad_epu8(_mm_load_si128((__m128i *)(&cur[15 * MB_WIDTH_Y])), _mm_loadu_si128((__m128i *)(&ref[15 * stride])));

  sum1 = _mm_add_epi16(sum1, xmm8);
  sum1 = _mm_add_epi16(sum1, xmm9);
  sum1 = _mm_add_epi16(sum1, xmm10);
  sum1 = _mm_add_epi16(sum1, xmm11);
  sum1 = _mm_add_epi16(sum1, xmm12);
  sum1 = _mm_add_epi16(sum1, xmm13);
  sum1 = _mm_add_epi16(sum1, xmm14);
  sum1 = _mm_add_epi16(sum1, xmm15);

  sum0 = _mm_packs_epi32(sum0, sum1);
  sum0 = _mm_packs_epi32(sum0, sum0);
  _mm_storel_epi64((__m128i *) sad, sum0);
  return(sad[3] + sad[2] + sad[1] + sad[0]);
}

static void taa_h264_split_sad_aligned_4x8x8(
  const uint8_t * cur,
  const uint8_t * ref,
  uint16_t        sad[4]
  )
{
  __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
  __m128i sum0, sum1;
  sum0 = _mm_setzero_si128();
  sum1 = _mm_setzero_si128();

  xmm0 = _mm_sad_epu8(*(__m128i *)(&cur[0 * MB_WIDTH_Y]), *(__m128i *)(&ref[0 * MB_WIDTH_Y]));
  xmm1 = _mm_sad_epu8(*(__m128i *)(&cur[1 * MB_WIDTH_Y]), *(__m128i *)(&ref[1 * MB_WIDTH_Y]));
  xmm2 = _mm_sad_epu8(*(__m128i *)(&cur[2 * MB_WIDTH_Y]), *(__m128i *)(&ref[2 * MB_WIDTH_Y]));
  xmm3 = _mm_sad_epu8(*(__m128i *)(&cur[3 * MB_WIDTH_Y]), *(__m128i *)(&ref[3 * MB_WIDTH_Y]));
  xmm4 = _mm_sad_epu8(*(__m128i *)(&cur[4 * MB_WIDTH_Y]), *(__m128i *)(&ref[4 * MB_WIDTH_Y]));
  xmm5 = _mm_sad_epu8(*(__m128i *)(&cur[5 * MB_WIDTH_Y]), *(__m128i *)(&ref[5 * MB_WIDTH_Y]));
  xmm6 = _mm_sad_epu8(*(__m128i *)(&cur[6 * MB_WIDTH_Y]), *(__m128i *)(&ref[6 * MB_WIDTH_Y]));
  xmm7 = _mm_sad_epu8(*(__m128i *)(&cur[7 * MB_WIDTH_Y]), *(__m128i *)(&ref[7 * MB_WIDTH_Y]));

  sum0 = _mm_add_epi16(sum0, xmm0);
  sum0 = _mm_add_epi16(sum0, xmm1);
  sum0 = _mm_add_epi16(sum0, xmm2);
  sum0 = _mm_add_epi16(sum0, xmm3);
  sum0 = _mm_add_epi16(sum0, xmm4);
  sum0 = _mm_add_epi16(sum0, xmm5);
  sum0 = _mm_add_epi16(sum0, xmm6);
  sum0 = _mm_add_epi16(sum0, xmm7);

  xmm8  = _mm_sad_epu8(*(__m128i *)(&cur[ 8 * MB_WIDTH_Y]), *(__m128i *)(&ref[ 8 * MB_WIDTH_Y]));
  xmm9  = _mm_sad_epu8(*(__m128i *)(&cur[ 9 * MB_WIDTH_Y]), *(__m128i *)(&ref[ 9 * MB_WIDTH_Y]));
  xmm10 = _mm_sad_epu8(*(__m128i *)(&cur[10 * MB_WIDTH_Y]), *(__m128i *)(&ref[10 * MB_WIDTH_Y]));
  xmm11 = _mm_sad_epu8(*(__m128i *)(&cur[11 * MB_WIDTH_Y]), *(__m128i *)(&ref[11 * MB_WIDTH_Y]));
  xmm12 = _mm_sad_epu8(*(__m128i *)(&cur[12 * MB_WIDTH_Y]), *(__m128i *)(&ref[12 * MB_WIDTH_Y]));
  xmm13 = _mm_sad_epu8(*(__m128i *)(&cur[13 * MB_WIDTH_Y]), *(__m128i *)(&ref[13 * MB_WIDTH_Y]));
  xmm14 = _mm_sad_epu8(*(__m128i *)(&cur[14 * MB_WIDTH_Y]), *(__m128i *)(&ref[14 * MB_WIDTH_Y]));
  xmm15 = _mm_sad_epu8(*(__m128i *)(&cur[15 * MB_WIDTH_Y]), *(__m128i *)(&ref[15 * MB_WIDTH_Y]));

  sum1 = _mm_add_epi16(sum1, xmm8);
  sum1 = _mm_add_epi16(sum1, xmm9);
  sum1 = _mm_add_epi16(sum1, xmm10);
  sum1 = _mm_add_epi16(sum1, xmm11);
  sum1 = _mm_add_epi16(sum1, xmm12);
  sum1 = _mm_add_epi16(sum1, xmm13);
  sum1 = _mm_add_epi16(sum1, xmm14);
  sum1 = _mm_add_epi16(sum1, xmm15);

  sum0 = _mm_packs_epi32(sum0, sum1);
  sum0 = _mm_packs_epi32(sum0, sum0);
  _mm_storel_epi64((__m128i *) sad, sum0);
}


static
void taa_h264_diamond_sad16x8x8(
  const int       stride,
  const uint8_t * cur,
  const uint8_t * ref,
  const int       pred_mvx,
  const int       pred_mvy,
  const int       curr_mvx,
  const int       curr_mvy,
  const uint32_t  lambda,
  uint32_t        mvcost[4],
  uint32_t        sad[4],
  unsigned short  sad_matrix[16])
{
  int i, bit_x0, bit_y0, bit_x1, bit_y1, bit_x2, bit_y2;
  __m128i xmm0, sum0, sum1, sum2, sum3, sum4, sum5, sum6, sum7;
  sum0 = _mm_setzero_si128();
  sum1 = _mm_setzero_si128();
  sum2 = _mm_setzero_si128();
  sum3 = _mm_setzero_si128();
  for(i = 0; i < 8; i++)
  {
    xmm0 = _mm_load_si128((__m128i *)(&cur[i * MB_WIDTH_Y]));
    sum0 = _mm_add_epi16(sum0, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 0) * stride - 1]))));
    sum1 = _mm_add_epi16(sum1, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i - 1) * stride + 0]))));
    sum2 = _mm_add_epi16(sum2, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 0) * stride + 1]))));
    sum3 = _mm_add_epi16(sum3, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 1) * stride + 0]))));
  }
  sum4 = _mm_setzero_si128();
  sum5 = _mm_setzero_si128();
  sum6 = _mm_setzero_si128();
  sum7 = _mm_setzero_si128();
  for(i = 0; i < 8; i++)
  {
    xmm0 = _mm_load_si128((__m128i *)(&cur[(i + 8) * MB_WIDTH_Y]));
    sum4 = _mm_add_epi16(sum4, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 8) * stride - 1]))));
    sum5 = _mm_add_epi16(sum5, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 7) * stride + 0]))));
    sum6 = _mm_add_epi16(sum6, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 8) * stride + 1]))));
    sum7 = _mm_add_epi16(sum7, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 9) * stride + 0]))));
  }
  sum0 = _mm_packs_epi32(sum0, sum4);
  sum0 = _mm_packs_epi32(sum0, sum0);
  sum1 = _mm_packs_epi32(sum1, sum5);
  sum1 = _mm_packs_epi32(sum1, sum1);
  sum2 = _mm_packs_epi32(sum2, sum6);
  sum2 = _mm_packs_epi32(sum2, sum2);
  sum3 = _mm_packs_epi32(sum3, sum7);
  sum3 = _mm_packs_epi32(sum3, sum3);

  _mm_storel_epi64((__m128i *) &sad_matrix[ 8], sum0);
  _mm_storel_epi64((__m128i *) &sad_matrix[ 0], sum1);
  _mm_storel_epi64((__m128i *) &sad_matrix[12], sum2);
  _mm_storel_epi64((__m128i *) &sad_matrix[ 4], sum3);

  bit_x0 = _lmbd(abs((curr_mvx + 0) - pred_mvx));
  bit_y0 = _lmbd(abs((curr_mvy + 0) - pred_mvy));
  bit_x1 = _lmbd(abs((curr_mvx - 4) - pred_mvx));
  bit_y1 = _lmbd(abs((curr_mvy - 4) - pred_mvy));
  bit_x2 = _lmbd(abs((curr_mvx + 4) - pred_mvx));
  bit_y2 = _lmbd(abs((curr_mvy + 4) - pred_mvy));

  sad[0] = sad_matrix[ 3] + sad_matrix[ 2] + sad_matrix[ 1] + sad_matrix[ 0];
  sad[1] = sad_matrix[ 7] + sad_matrix[ 6] + sad_matrix[ 5] + sad_matrix[ 4];
  sad[2] = sad_matrix[11] + sad_matrix[10] + sad_matrix[ 9] + sad_matrix[ 8];
  sad[3] = sad_matrix[15] + sad_matrix[14] + sad_matrix[13] + sad_matrix[12];

  mvcost[0] = MV_COST2 (lambda, bit_x0, bit_y1);
  mvcost[1] = MV_COST2 (lambda, bit_x0, bit_y2);
  mvcost[2] = MV_COST2 (lambda, bit_x1, bit_y0);
  mvcost[3] = MV_COST2 (lambda, bit_x2, bit_y0);
}

#if 0
static
void taa_h264_diamond_arrow_sad16x8x8 (
  const int       stride,
  const int       step_up,
  const int       step_down,
  const int       step_left,
  const int       step_right,
  const uint8_t * cur,
  const uint8_t * ref,
  const int       pred_mvx,
  const int       pred_mvy,
  const int       curr_mvx,
  const int       curr_mvy,
  const uint32_t  lambda,
  uint32_t        mvcost[4],
  uint32_t        sad[4],
  unsigned short  sad_matrix[16])
{
  __m128i xmm0, sum0, sum1, sum2, sum3, sum4, sum5, sum6, sum7;
  sum0 = _mm_setzero_si128();
  sum1 = _mm_setzero_si128();
  sum2 = _mm_setzero_si128();
  sum3 = _mm_setzero_si128();
  for(int i = 0; i < 8; i++)
  {
    xmm0 = _mm_load_si128((__m128i *)(&cur[i * MB_WIDTH_Y]));
    xmm0 = _mm_load_si128((__m128i *)(&cur[i * MB_WIDTH_Y]));
    sum0 = _mm_add_epi16(sum0, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i +   step_up) * stride +           0])))); // up
    sum1 = _mm_add_epi16(sum1, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + step_down) * stride +           0])))); // down
    sum2 = _mm_add_epi16(sum2, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i +          0) * stride +  step_left])))); // left
    sum3 = _mm_add_epi16(sum3, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i +          0) * stride + step_right])))); // right
  }
  sum6 = _mm_setzero_si128();
  sum4 = _mm_setzero_si128();
  sum7 = _mm_setzero_si128();
  sum5 = _mm_setzero_si128();
  for(int i = 0; i < 8; i++)
  {
    xmm0 = _mm_load_si128((__m128i *)(&cur[(i + 8) * MB_WIDTH_Y]));
    sum4 = _mm_add_epi16(sum4, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 8 +   step_up) * stride +           0])))); // up
    sum5 = _mm_add_epi16(sum5, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 8 + step_down) * stride +           0])))); // down
    sum6 = _mm_add_epi16(sum6, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 8 +          0) * stride +  step_left])))); // left
    sum7 = _mm_add_epi16(sum7, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 8 +          0) * stride + step_right])))); // right
  }
  sum2 = _mm_packs_epi32(sum2, sum6);
  sum2 = _mm_packs_epi32(sum2, sum2);
  sum0 = _mm_packs_epi32(sum0, sum4);
  sum0 = _mm_packs_epi32(sum0, sum0);
  sum3 = _mm_packs_epi32(sum3, sum7);
  sum3 = _mm_packs_epi32(sum3, sum3);
  sum1 = _mm_packs_epi32(sum1, sum5);
  sum1 = _mm_packs_epi32(sum1, sum1);

  _mm_storel_epi64((__m128i *) &sad_matrix[ 0], sum0);  // up
  _mm_storel_epi64((__m128i *) &sad_matrix[ 4], sum1);  // down
  _mm_storel_epi64((__m128i *) &sad_matrix[ 8], sum2);  // left
  _mm_storel_epi64((__m128i *) &sad_matrix[12], sum3);  // right

  sad[0] = sad_matrix[ 3] + sad_matrix[ 2] + sad_matrix[ 1] + sad_matrix[ 0];
  sad[1] = sad_matrix[ 7] + sad_matrix[ 6] + sad_matrix[ 5] + sad_matrix[ 4];
  sad[2] = sad_matrix[11] + sad_matrix[10] + sad_matrix[ 9] + sad_matrix[ 8];
  sad[3] = sad_matrix[15] + sad_matrix[14] + sad_matrix[13] + sad_matrix[12];

  mvcost[0] = MV_COST (lambda, curr_mvx - pred_mvx +            0, curr_mvy - pred_mvy +   4*step_up); // up
  mvcost[1] = MV_COST (lambda, curr_mvx - pred_mvx +            0, curr_mvy - pred_mvy + 4*step_down); // down
  mvcost[2] = MV_COST (lambda, curr_mvx - pred_mvx +  4*step_left, curr_mvy - pred_mvy +           0); // left
  mvcost[3] = MV_COST (lambda, curr_mvx - pred_mvx + 4*step_right, curr_mvy - pred_mvy +           0); // right
}
#endif

static
void taa_h264_diamond_crude_sad16x8x8(
  const int       stride,
  const int       step,
  const uint8_t * cur,
  const uint8_t * ref,
  const int       pred_mvx,
  const int       pred_mvy,
  const int       curr_mvx,
  const int       curr_mvy,
  const uint32_t  lambda,
  uint32_t        mvcost[4],
  uint32_t        sad[4],
  unsigned short  sad_matrix[16])
{
  int i, bit_x0, bit_y0, bit_x1, bit_y1, bit_x2, bit_y2;
  __m128i xmm0, sum0, sum1, sum2, sum3, sum4, sum5, sum6, sum7;
  sum0 = _mm_setzero_si128();
  sum1 = _mm_setzero_si128();
  sum2 = _mm_setzero_si128();
  sum3 = _mm_setzero_si128();
  for(i = 0; i < 8; i++)
  {
    xmm0 = _mm_load_si128((__m128i *)(&cur[i * MB_WIDTH_Y]));
    xmm0 = _mm_load_si128((__m128i *)(&cur[i * MB_WIDTH_Y]));
    sum0 = _mm_add_epi16(sum0, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i - step) * stride +    0])))); // up
    sum1 = _mm_add_epi16(sum1, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + step) * stride +    0])))); // down
    sum2 = _mm_add_epi16(sum2, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i +    0) * stride - step])))); // left
    sum3 = _mm_add_epi16(sum3, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i +    0) * stride + step])))); // right
  }
  sum6 = _mm_setzero_si128();
  sum4 = _mm_setzero_si128();
  sum7 = _mm_setzero_si128();
  sum5 = _mm_setzero_si128();
  for(i = 0; i < 8; i++)
  {
    xmm0 = _mm_load_si128((__m128i *)(&cur[(i + 8) * MB_WIDTH_Y]));
    sum4 = _mm_add_epi16(sum4, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 8 - step) * stride +    0])))); // up
    sum5 = _mm_add_epi16(sum5, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 8 + step) * stride +    0])))); // down
    sum6 = _mm_add_epi16(sum6, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 8 +    0) * stride - step])))); // left
    sum7 = _mm_add_epi16(sum7, _mm_sad_epu8(xmm0, _mm_loadu_si128((__m128i *)(&ref[(i + 8 +    0) * stride + step])))); // right
  }
  sum2 = _mm_packs_epi32(sum2, sum6);
  sum2 = _mm_packs_epi32(sum2, sum2);
  sum0 = _mm_packs_epi32(sum0, sum4);
  sum0 = _mm_packs_epi32(sum0, sum0);
  sum3 = _mm_packs_epi32(sum3, sum7);
  sum3 = _mm_packs_epi32(sum3, sum3);
  sum1 = _mm_packs_epi32(sum1, sum5);
  sum1 = _mm_packs_epi32(sum1, sum1);

  _mm_storel_epi64((__m128i *) &sad_matrix[ 0], sum0);  // up
  _mm_storel_epi64((__m128i *) &sad_matrix[ 4], sum1);  // down
  _mm_storel_epi64((__m128i *) &sad_matrix[ 8], sum2);  // left
  _mm_storel_epi64((__m128i *) &sad_matrix[12], sum3);  // right

  bit_x0 = _lmbd(abs((curr_mvx         ) - pred_mvx));
  bit_y0 = _lmbd(abs((curr_mvy         ) - pred_mvy));
  bit_x1 = _lmbd(abs((curr_mvx - 4*step) - pred_mvx));
  bit_y1 = _lmbd(abs((curr_mvy - 4*step) - pred_mvy));
  bit_x2 = _lmbd(abs((curr_mvx + 4*step) - pred_mvx));
  bit_y2 = _lmbd(abs((curr_mvy + 4*step) - pred_mvy));

  sad[0] = sad_matrix[ 3] + sad_matrix[ 2] + sad_matrix[ 1] + sad_matrix[ 0];
  sad[1] = sad_matrix[ 7] + sad_matrix[ 6] + sad_matrix[ 5] + sad_matrix[ 4];
  sad[2] = sad_matrix[11] + sad_matrix[10] + sad_matrix[ 9] + sad_matrix[ 8];
  sad[3] = sad_matrix[15] + sad_matrix[14] + sad_matrix[13] + sad_matrix[12];

  mvcost[0] = MV_COST2 (lambda, bit_x0, bit_y1);
  mvcost[1] = MV_COST2 (lambda, bit_x0, bit_y2);
  mvcost[2] = MV_COST2 (lambda, bit_x1, bit_y0);
  mvcost[3] = MV_COST2 (lambda, bit_x2, bit_y0);
}

static int taa_h264_half_search_16x16 (
  const int       stride,
  const uint8_t * mblock,
  const uint8_t * fpel,
  const uint8_t * half_hor,
  const uint8_t * half_vert,
  uint32_t        min_cost,
  int *           ydelta,
  int *           xdelta,
  const int       mvdy,
  const int       mvdx,
  const int       lambda)
{
  /* Search in 4 half pel positions around the min_sad position:
   *      1
   *    3   4
   *      2
   *
   * Since only horizonal and vertical half pels are implemented we skip the
   * diagonal positions. If they were to included, the number of positions
   * to check would have been 8.
   */
  uint32_t sad[4];
  taa_h264_diamond_sad16x16(
    stride,
    mblock,
    fpel,
    &half_vert[-stride],
    &half_hor[-1],
    sad);

  const int bit_x0 = _lmbd(abs(mvdx+0));
  const int bit_x1 = _lmbd(abs(mvdx-2));
  const int bit_x2 = _lmbd(abs(mvdx+2));
  const int bit_y0 = _lmbd(abs(mvdy+0));
  const int bit_y1 = _lmbd(abs(mvdy-2));
  const int bit_y2 = _lmbd(abs(mvdy+2));

  const uint32_t cost[5] =
  {
    min_cost,
    sad[0] + MV_COST2 (lambda, bit_x0, bit_y1),
    sad[1] + MV_COST2 (lambda, bit_x0, bit_y2),
    sad[2] + MV_COST2 (lambda, bit_x1, bit_y0),
    sad[3] + MV_COST2 (lambda, bit_x2, bit_y0)
  };

  int j = 0;
  for(int i = 1; i < 5; i++)
    if (cost[i] < cost[j] )
      j = i;
  const int dx[5] = {*xdelta, 0, 0, -2, 2};
  const int dy[5] = {*ydelta, -2, 2, 0, 0};

  min_cost = cost[j];
  *xdelta = dx[j];
  *ydelta = dy[j];

  return min_cost;
}

#if HPEL_DIAG_ENABLED

static int taa_h264_diag_half_search_16x16 (
  const int       stride,
  const uint8_t * mblock,
  const uint8_t * fpel,
  const uint8_t * half_diag,
  uint32_t        min_cost,
  int *           ydelta,
  int *           xdelta,
  const int       mvdy,
  const int       mvdx,
  const int       lambda)
{
  uint32_t sad[4];
  taa_h264_square_sad16x16(
    stride,
    mblock,
    fpel,
    half_diag,
    sad);

  const int bit_x0 = _lmbd(abs(mvdx-2));
  const int bit_x1 = _lmbd(abs(mvdx+2));
  const int bit_y0 = _lmbd(abs(mvdy-2));
  const int bit_y1 = _lmbd(abs(mvdy+2));

  const uint32_t cost[5] =
  {
    min_cost,
    sad[0] + MV_COST2 (lambda, bit_x0, bit_y0),
    sad[1] + MV_COST2 (lambda, bit_x1, bit_y0),
    sad[2] + MV_COST2 (lambda, bit_x0, bit_y1),
    sad[3] + MV_COST2 (lambda, bit_x1, bit_y1)
  };

  int j = 0;
  for(int i = 1; i < 5; i++)
    if (cost[i] < cost[j] )
      j = i;

  const int dx[5] = {*xdelta, -2,  2, -2,  2};
  const int dy[5] = {*ydelta, -2, -2,  2,  2};

  min_cost = cost[j];
  *xdelta = dx[j];
  *ydelta = dy[j];

  return min_cost;
}

static int taa_h264_diag_half_search_16x8 (
  const int       stride,
  const uint8_t * mblock,
  const uint8_t * fpel,
  const uint8_t * half_diag,
  uint32_t        min_cost,
  int *           ydelta,
  int *           xdelta,
  const int       mvdy,
  const int       mvdx,
  const int       lambda)
{
  uint32_t sad[4];
  taa_h264_square_sad16x8(
    stride,
    mblock,
    fpel,
    half_diag,
    sad);

  const int bit_x0 = _lmbd(abs(mvdx-2));
  const int bit_x1 = _lmbd(abs(mvdx+2));
  const int bit_y0 = _lmbd(abs(mvdy-2));
  const int bit_y1 = _lmbd(abs(mvdy+2));

  const uint32_t cost[5] =
  {
    min_cost,
    sad[0] + MV_COST2 (lambda, bit_x0, bit_y0),
    sad[1] + MV_COST2 (lambda, bit_x1, bit_y0),
    sad[2] + MV_COST2 (lambda, bit_x0, bit_y1),
    sad[3] + MV_COST2 (lambda, bit_x1, bit_y1)
  };

  int j = 0;
  for(int i = 1; i < 5; i++)
    if (cost[i] < cost[j] )
      j = i;

  const int dx[5] = {*xdelta, -2,  2, -2,  2};
  const int dy[5] = {*ydelta, -2, -2,  2,  2};

  min_cost = cost[j];
  *xdelta = dx[j];
  *ydelta = dy[j];

  return min_cost;
}


static int taa_h264_diag_half_search_8x16 (
  const int       stride,
  const uint8_t * mblock,
  const uint8_t * fpel,
  const uint8_t * half_diag,
  uint32_t        min_cost,
  int *           ydelta,
  int *           xdelta,
  const int       mvdy,
  const int       mvdx,
  const int       lambda)
{
  uint32_t sad[4];
  taa_h264_square_sad8x16(
    stride,
    mblock,
    fpel,
    half_diag,
    sad);

  const int bit_x0 = _lmbd(abs(mvdx-2));
  const int bit_x1 = _lmbd(abs(mvdx+2));
  const int bit_y0 = _lmbd(abs(mvdy-2));
  const int bit_y1 = _lmbd(abs(mvdy+2));

  const uint32_t cost[5] =
  {
    min_cost,
    sad[0] + MV_COST2 (lambda, bit_x0, bit_y0),
    sad[1] + MV_COST2 (lambda, bit_x1, bit_y0),
    sad[2] + MV_COST2 (lambda, bit_x0, bit_y1),
    sad[3] + MV_COST2 (lambda, bit_x1, bit_y1)
  };

  int j = 0;
  for(int i = 1; i < 5; i++)
    if (cost[i] < cost[j] )
      j = i;

  const int dx[5] = {*xdelta, -2,  2, -2,  2};
  const int dy[5] = {*ydelta, -2, -2,  2,  2};

  min_cost = cost[j];
  *xdelta = dx[j];
  *ydelta = dy[j];

  return min_cost;
}

#endif // HPEL_DIAG_ENABLED


static int taa_h264_half_search_16x8 (
  const int       stride,
  const uint8_t * mblock,
  const uint8_t * fpel,
  const uint8_t * half_hor,
  const uint8_t * half_vert,
  uint32_t        min_cost,
  int *           ydelta,
  int *           xdelta,
  const int       mvdy,
  const int       mvdx,
  const int       lambda)
{
  /* Search in 4 half pel positions around the min_sad position:
   *      1
   *    3   4
   *      2
   *
   * Since only horizonal and vertical half pels are implemented we skip the
   * diagonal positions. If they were to included, the number of positions
   * to check would have been 8.
   */
  uint32_t sad[4];
  taa_h264_diamond_sad16x8(
    stride,
    mblock,
    fpel,
    &half_vert[-stride],
    &half_hor[-1],
    sad);

  const int bit_x0 = _lmbd(abs(mvdx+0));
  const int bit_x1 = _lmbd(abs(mvdx-2));
  const int bit_x2 = _lmbd(abs(mvdx+2));
  const int bit_y0 = _lmbd(abs(mvdy+0));
  const int bit_y1 = _lmbd(abs(mvdy-2));
  const int bit_y2 = _lmbd(abs(mvdy+2));

  const uint32_t cost[5] =
  {
    min_cost,
    sad[0] + MV_COST2 (lambda, bit_x0, bit_y1),
    sad[1] + MV_COST2 (lambda, bit_x0, bit_y2),
    sad[2] + MV_COST2 (lambda, bit_x1, bit_y0),
    sad[3] + MV_COST2 (lambda, bit_x2, bit_y0)
  };

  int j = 0;
  for(int i = 1; i < 5; i++)
    if (cost[i] < cost[j] )
      j = i;
  const int dx[5] = {*xdelta, 0, 0, -2, 2};
  const int dy[5] = {*ydelta, -2, 2, 0, 0};

  min_cost = cost[j];
  *xdelta = dx[j];
  *ydelta = dy[j];

  return min_cost;
}

static int taa_h264_half_search_8x16(
  const int       stride,
  const uint8_t * mblock,
  const uint8_t * fpel,
  const uint8_t * half_hor,
  const uint8_t * half_vert,
  uint32_t        min_cost,
  int *           ydelta,
  int *           xdelta,
  const int       mvdy,
  const int       mvdx,
  const int       lambda)
{
  /* Search in 4 half pel positions around the min_sad position:
   *      1
   *    3   4
   *      2
   *
   * Since only horizonal and vertical half pels are implemented we skip the
   * diagonal positions. If they were to included, the number of positions
   * to check would have been 8.
   */
  uint32_t sad[4];
  taa_h264_diamond_sad8x16(
    stride,
    mblock,
    fpel,
    &half_vert[-stride],
    &half_hor[-1],
    sad);

  const int bit_x0 = _lmbd(abs(mvdx+0));
  const int bit_x1 = _lmbd(abs(mvdx-2));
  const int bit_x2 = _lmbd(abs(mvdx+2));
  const int bit_y0 = _lmbd(abs(mvdy+0));
  const int bit_y1 = _lmbd(abs(mvdy-2));
  const int bit_y2 = _lmbd(abs(mvdy+2));

  const uint32_t cost[5] =
  {
    min_cost,
    sad[0] + MV_COST2 (lambda, bit_x0, bit_y1),
    sad[1] + MV_COST2 (lambda, bit_x0, bit_y2),
    sad[2] + MV_COST2 (lambda, bit_x1, bit_y0),
    sad[3] + MV_COST2 (lambda, bit_x2, bit_y0)
  };

  int j = 0;
  for(int i = 1; i < 5; i++)
    if (cost[i] < cost[j] )
      j = i;
  const int dx[5] = {*xdelta, 0, 0, -2, 2};
  const int dy[5] = {*ydelta, -2, 2, 0, 0};

  min_cost = cost[j];
  *xdelta = dx[j];
  *ydelta = dy[j];

  return min_cost;
}



static int taa_h264_quarter_search_16x16 (
  const int       stride,
  const uint8_t * mblock,                                       /* Input macroblock */
  const uint8_t * full_y,                                       /* Full pels */
  const uint8_t * half_h,                                       /* Horizontal half pels */
  const uint8_t * half_v,                                       /* Vertical half pels */
#if HPEL_DIAG_ENABLED
  const uint8_t * half_d,                                       /* Diagonal half pels */
#endif
  uint32_t        min_cost,                                     /* The minimal cost so far */
  int *           ydelta,                                       /* Out: Quarter pels to best position */
  int *           xdelta,                                       /* Out: Quarter pels to best position */
  const int       mvdy,                                         /* The MV of the best full_y pel position */
  const int       mvdx,                                         /* The MV of the best full_y pel position */
  const int       lambda)                                       /* Lagrange multiplier */
{
  const int zfrac = 4 * (*ydelta & 3) + (*xdelta & 3);

  if (zfrac == 0) // fullpel pos 0
  {
    uint32_t sad[8];
    // taa_h264_qp_sad16x16d4(stride, mblock, full_y, &half_v[-stride], &half_h[-1], &sad[0]); // qpel diamond: up, down, left, right
    taa_h264_qp_sad16x16x4d4(stride, mblock, full_y, &half_v[-stride], &half_h[-1], &sad[0]); // qpel x-cross: up-left, up-right, down-left, down-right

    const int bit_x0 = _lmbd(abs(mvdx + 0));
    const int bit_y0 = _lmbd(abs(mvdy + 0));
    const int bit_x1 = _lmbd(abs(mvdx - 1));
    const int bit_y1 = _lmbd(abs(mvdy - 1));
    const int bit_x2 = _lmbd(abs(mvdx + 1));
    const int bit_y2 = _lmbd(abs(mvdy + 1));

    const uint32_t cost[9] =
    {
      min_cost,
      sad[0] + MV_COST2 (lambda, bit_x0, bit_y1),
      sad[1] + MV_COST2 (lambda, bit_x0, bit_y2),
      sad[2] + MV_COST2 (lambda, bit_x1, bit_y0),
      sad[3] + MV_COST2 (lambda, bit_x2, bit_y0),
      sad[4] + MV_COST2 (lambda, bit_x1, bit_y1),
      sad[5] + MV_COST2 (lambda, bit_x2, bit_y1),
      sad[6] + MV_COST2 (lambda, bit_x1, bit_y2),
      sad[7] + MV_COST2 (lambda, bit_x2, bit_y2)
    };

    int j = 0;
    for(int i = 1; i < 9; i++)
      if (cost[i] < cost[j] )
        j = i;
    const int dx[9] = { *xdelta, 0, 0, -1, 1,  -1, 1, -1, 1};
    const int dy[9] = { *ydelta, -1, 1, 0, 0,  -1, -1, 1, 1};

    min_cost = cost[j];
    *xdelta = dx[j];
    *ydelta = dy[j];
  }
  else if (zfrac == 2) // halfpel pos 2
  {
    uint32_t sad[6];
    const int a = *xdelta + 2;
    const int b = (*xdelta - 2) >> 2;
    taa_h264_qp_sad16x16h2(stride, mblock, &half_h[b], &full_y[b],          &sad[0]); // horizontal 2 positions
    taa_h264_qp_sad16x16v4(stride, mblock, &full_y[b], &half_h[b], &half_v[b - stride], &sad[2]); // vertical 4 positions

    const int bit_x0 = _lmbd(abs(mvdx + a-3));
    const int bit_x1 = _lmbd(abs(mvdx + a-1));
    const int bit_y0 = _lmbd(abs(mvdy +   0));
    const int bit_y1 = _lmbd(abs(mvdy -   1));
    const int bit_y2 = _lmbd(abs(mvdy +   1));

    const uint32_t cost[7] =
    {
      min_cost,
      sad[0] + MV_COST2 (lambda, bit_x0, bit_y0),
      sad[1] + MV_COST2 (lambda, bit_x1, bit_y0),
      sad[2] + MV_COST2 (lambda, bit_x0, bit_y1),
      sad[3] + MV_COST2 (lambda, bit_x1, bit_y1),
      sad[4] + MV_COST2 (lambda, bit_x0, bit_y2),
      sad[5] + MV_COST2 (lambda, bit_x1, bit_y2)
    };

    int j = 0;
    for(int i = 1; i < 7; i++)
      if (cost[i] < cost[j] )
        j = i;
    const int dx[7] = { *xdelta, a-3, a-1, a-3, a-1, a-3, a-1};
    const int dy[7] = { *ydelta,   0,   0,  -1,  -1,   1,   1};

    min_cost = cost[j];
    *xdelta = dx[j];
    *ydelta = dy[j];
  }
  else if (zfrac == 8) // halfpel pos 8
  {
    uint32_t sad[6];
    const int a = *ydelta + 2;
    const int b = (*ydelta - 2) >> 2;
    taa_h264_qp_sad16x16h4(stride, mblock, &full_y[b * stride], &half_h[b * stride - 1], &half_v[b * stride], &sad[0]); // vertical 4 positions
    taa_h264_qp_sad16x16v2(stride, mblock, &half_v[b * stride],     &full_y[b * stride], &sad[4]); // horizontal 2 positions

    const int bit_x0 = _lmbd(abs(mvdx -   1));
    const int bit_x1 = _lmbd(abs(mvdx +   1));
    const int bit_x2 = _lmbd(abs(mvdx +   0));
    const int bit_y0 = _lmbd(abs(mvdy + a-3));
    const int bit_y1 = _lmbd(abs(mvdy + a-1));

    const uint32_t cost[7] =
    {
      min_cost,
      sad[0] + MV_COST2 (lambda, bit_x0, bit_y0),
      sad[1] + MV_COST2 (lambda, bit_x0, bit_y1),
      sad[2] + MV_COST2 (lambda, bit_x1, bit_y0),
      sad[3] + MV_COST2 (lambda, bit_x1, bit_y1),
      sad[4] + MV_COST2 (lambda, bit_x2, bit_y0),
      sad[5] + MV_COST2 (lambda, bit_x2, bit_y1)
    };
    int j = 0;
    for(int i = 1; i < 7; i++)
      if (cost[i] < cost[j] )
        j = i;
    const int dx[7] = { *xdelta,  -1,  -1,   1,   1,   0,   0};
    const int dy[7] = { *ydelta, a-3, a-1, a-3, a-1, a-3, a-1};
    min_cost = cost[j];
    *xdelta = dx[j];
    *ydelta = dy[j];
  }
#if HPEL_DIAG_ENABLED
  else // halfpel pos 10
  {
    uint32_t sad[8];
    taa_h264_qp_sad16x16d4(stride, mblock, full_y, half_h, half_v, half_d, &sad[0]); // diamond around diagonal hpel
    taa_h264_qp_sad16x16x4(stride, mblock, full_y, half_h, half_v, half_d, &sad[4]); // square around diagonal hpel

    const int bit_x1 = _lmbd(abs(mvdx + 1));
    const int bit_x2 = _lmbd(abs(mvdx + 2));
    const int bit_x3 = _lmbd(abs(mvdx + 3));
    const int bit_y1 = _lmbd(abs(mvdy + 1));
    const int bit_y2 = _lmbd(abs(mvdy + 2));
    const int bit_y3 = _lmbd(abs(mvdy + 3));

    const uint32_t cost[9] =
    {
      min_cost,
      sad[0] + MV_COST2 (lambda, bit_x2, bit_y1),
      sad[1] + MV_COST2 (lambda, bit_x1, bit_y2),
      sad[2] + MV_COST2 (lambda, bit_x3, bit_y2),
      sad[3] + MV_COST2 (lambda, bit_x2, bit_y3),
      sad[4] + MV_COST2 (lambda, bit_x1, bit_y1),
      sad[5] + MV_COST2 (lambda, bit_x3, bit_y1),
      sad[6] + MV_COST2 (lambda, bit_x1, bit_y3),
      sad[7] + MV_COST2 (lambda, bit_x3, bit_y3)
    };
    int j = 0;
    for(int i = 1; i < 9; i++)
      if (cost[i] < cost[j])
        j = i;
    const int dx[9] = { *xdelta, 2, 1, 3, 2, 1, 3, 1, 3};
    const int dy[9] = { *ydelta, 1, 2, 2, 3, 1, 1, 3, 3};
    min_cost = cost[j];
    *xdelta = dx[j];
    *ydelta = dy[j];
  }
#endif

#if 0

  const int dmvx[9] = { 0, 0, 0, -1, 0, -1, 0, -1, 0};
  const int dmvy[9] = { 0, -1, 0, 0, 0, -1, -1, 0, 0};
  int const offset_x = dmvx[j];
  int const offset_y = dmvy[j];
  __assume_aligned(recon_y, 64);
  if (j == 0)
  {
    for (int y = 0; y < 16; y++)
      for (int x = 0; x < 16; x++)
        recon_y[y * MB_WIDTH_Y + x] = full_y[y * stride + x];
  }
  else if (j < 3)
  {
    for (int y = 0; y < 16; y++)
      for (int x = 0; x < 16; x++)
        recon_y[y * MB_WIDTH_Y + x] = (uint8_t)((full_y[y * stride + x] + half_v[(y + offset_y)* stride + x] + 1) >> 1);
  }
  else if (j < 5)
  {
    for (int y = 0; y < 16; y++)
      for (int x = 0; x < 16; x++)
        recon_y[y * MB_WIDTH_Y + x] = (uint8_t)((full_y[y * stride + x] + half_h[y * stride + x + offset_x] + 1) >> 1);
  }
  else
  {
    for (int y = 0; y < 16; y++)
      for (int x = 0; x < 16; x++)
        recon_y[y * MB_WIDTH_Y + x] = (uint8_t)((half_v[(y + offset_y)* stride + x] + half_h[y * stride + x + offset_x] + 1) >> 1);
  }
# ifndef NDEBUG
  //============================================
  //    Check that recon_y is really best mb
  //============================================
  uint32_t sad_y = 0;
  for (int y = 0; y < 16; y++)
    for (int x = 0; x < 16; x++)
      sad_y += abs(recon_y[y * MB_WIDTH_Y + x] - mblock[y * MB_WIDTH_Y + x]);
  int min_cost_y = sad_y + MV_COST (lambda, mvdx + *xdelta, mvdy + *ydelta);
  TAA_h264_DEBUG_ASSERT(min_cost_y == min_cost);
# endif

#endif

  return min_cost;
}

static int taa_h264_quarter_search_16x8 (
  const int       stride,
  const uint8_t * mblock,                                       /* Input macroblock */
  const uint8_t * full_y,                                         /* Full pels */
  const uint8_t * half_h,                                     /* Horizontal half pels */
  const uint8_t * half_v,                                    /* Vertical half pels */
#if HPEL_DIAG_ENABLED
  const uint8_t * half_d,                                    /* Vertical half pels */
#endif
  uint32_t        min_cost,                                     /* The minimal cost so far */
  int *           ydelta,                                       /* Out: Quarter pels to best position */
  int *           xdelta,                                       /* Out: Quarter pels to best position */
  const int       mvdy,                                         /* The MV of the best full_y pel position */
  const int       mvdx,                                         /* The MV of the best full_y pel position */
  const int       lambda)                                       /* Lagrange multiplier */
{
  /* Search all quarter pels around the best full_y pel */

  const int zfrac = 4 * (*ydelta & 3) + (*xdelta & 3);

  if (zfrac == 0) // fullpel pos 0
  {
    uint32_t sad[8];
    // taa_h264_qp_sad16x8d4(stride, mblock, full_y, &half_v[-stride], &half_h[-1], &sad[0]); // qpel diamond: up, down, left, right
    taa_h264_qp_sad16x8x4d4(stride, mblock, full_y, &half_v[-stride], &half_h[-1], &sad[0]); // qpel x-cross: up-left, up-right, down-left, down-right

    const int bit_x0 = _lmbd(abs(mvdx + 0));
    const int bit_y0 = _lmbd(abs(mvdy + 0));
    const int bit_x1 = _lmbd(abs(mvdx - 1));
    const int bit_y1 = _lmbd(abs(mvdy - 1));
    const int bit_x2 = _lmbd(abs(mvdx + 1));
    const int bit_y2 = _lmbd(abs(mvdy + 1));

    const uint32_t cost[9] =
    {
      min_cost,
      sad[0] + MV_COST2 (lambda, bit_x0, bit_y1),
      sad[1] + MV_COST2 (lambda, bit_x0, bit_y2),
      sad[2] + MV_COST2 (lambda, bit_x1, bit_y0),
      sad[3] + MV_COST2 (lambda, bit_x2, bit_y0),
      sad[4] + MV_COST2 (lambda, bit_x1, bit_y1),
      sad[5] + MV_COST2 (lambda, bit_x2, bit_y1),
      sad[6] + MV_COST2 (lambda, bit_x1, bit_y2),
      sad[7] + MV_COST2 (lambda, bit_x2, bit_y2)
    };

    int j = 0;
    for(int i = 1; i < 9; i++)
      if (cost[i] < cost[j] )
        j = i;

    const int dx[9] = { *xdelta, 0, 0, -1, 1,  -1, 1, -1, 1};
    const int dy[9] = { *ydelta, -1, 1, 0, 0,  -1, -1, 1, 1};

    min_cost = cost[j];
    *xdelta = dx[j];
    *ydelta = dy[j];

  }
  else if (zfrac == 2) // halfpel pos 2
  {
    uint32_t sad[6];
    const int a = *xdelta + 2;
    const int b = (*xdelta - 2) >> 2;

    taa_h264_qp_sad16x8h2(stride, mblock, &half_h[b], &full_y[b],          &sad[0]); // horizontal 2 positions
    taa_h264_qp_sad16x8v4(stride, mblock, &full_y[b], &half_h[b], &half_v[b - stride], &sad[2]); // vertical 4 positions

    const int bit_x0 = _lmbd(abs(mvdx + a-3));
    const int bit_x1 = _lmbd(abs(mvdx + a-1));
    const int bit_y0 = _lmbd(abs(mvdy +   0));
    const int bit_y1 = _lmbd(abs(mvdy -   1));
    const int bit_y2 = _lmbd(abs(mvdy +   1));

    const uint32_t cost[7] =
    {
      min_cost,
      sad[0] + MV_COST2 (lambda, bit_x0, bit_y0),
      sad[1] + MV_COST2 (lambda, bit_x1, bit_y0),
      sad[2] + MV_COST2 (lambda, bit_x0, bit_y1),
      sad[3] + MV_COST2 (lambda, bit_x1, bit_y1),
      sad[4] + MV_COST2 (lambda, bit_x0, bit_y2),
      sad[5] + MV_COST2 (lambda, bit_x1, bit_y2),
    };

    int j = 0;
    for(int i = 1; i < 7; i++)
      if (cost[i] < cost[j] )
        j = i;
    const int dx[7] = { *xdelta, a-3, a-1, a-3, a-1, a-3, a-1};
    const int dy[7] = { *ydelta,   0,   0,  -1,  -1,   1,   1};

    min_cost = cost[j];
    *xdelta = dx[j];
    *ydelta = dy[j];

  }
  else if (zfrac == 8) // halfpel pos 8
  {
    uint32_t sad[6];
    const int a = *ydelta + 2;
    const int b = (*ydelta - 2) >> 2;

    taa_h264_qp_sad16x8h4(stride, mblock, &full_y[b * stride], &half_h[b * stride - 1], &half_v[b * stride], &sad[0]); // vertical 4 positions
    taa_h264_qp_sad16x8v2(stride, mblock, &half_v[b * stride],     &full_y[b * stride], &sad[4]); // horizontal 2 positions

    const int bit_x0 = _lmbd(abs(mvdx -   1));
    const int bit_x1 = _lmbd(abs(mvdx +   1));
    const int bit_x2 = _lmbd(abs(mvdx +   0));
    const int bit_y0 = _lmbd(abs(mvdy + a-3));
    const int bit_y1 = _lmbd(abs(mvdy + a-1));

    const uint32_t cost[7] =
    {
      min_cost,
      sad[0] + MV_COST2 (lambda, bit_x0, bit_y0),
      sad[1] + MV_COST2 (lambda, bit_x0, bit_y1),
      sad[2] + MV_COST2 (lambda, bit_x1, bit_y0),
      sad[3] + MV_COST2 (lambda, bit_x1, bit_y1),
      sad[4] + MV_COST2 (lambda, bit_x2, bit_y0),
      sad[5] + MV_COST2 (lambda, bit_x2, bit_y1)
    };
    int j = 0;
    for(int i = 1; i < 7; i++)
      if (cost[i] < cost[j] )
        j = i;

    const int dx[7] = { *xdelta,  -1,  -1,   1,   1,   0,   0};
    const int dy[7] = { *ydelta, a-3, a-1, a-3, a-1, a-3, a-1};

    min_cost = cost[j];
    *xdelta = dx[j];
    *ydelta = dy[j];
  }
#if HPEL_DIAG_ENABLED
  else // halfpel pos 10
  {
    uint32_t sad[8];

    taa_h264_qp_sad16x8d4(stride, mblock, full_y, half_h, half_v, half_d, &sad[0]); // diamond around diagonal hpel
    taa_h264_qp_sad16x8x4(stride, mblock, full_y, half_h, half_v, half_d, &sad[4]); // square around diagonal hpel

    const int bit_x1 = _lmbd(abs(mvdx + 1));
    const int bit_x2 = _lmbd(abs(mvdx + 2));
    const int bit_x3 = _lmbd(abs(mvdx + 3));
    const int bit_y1 = _lmbd(abs(mvdy + 1));
    const int bit_y2 = _lmbd(abs(mvdy + 2));
    const int bit_y3 = _lmbd(abs(mvdy + 3));

    const uint32_t cost[9] =
    {
      min_cost,
      sad[0] + MV_COST2 (lambda, bit_x2, bit_y1),
      sad[1] + MV_COST2 (lambda, bit_x1, bit_y2),
      sad[2] + MV_COST2 (lambda, bit_x3, bit_y2),
      sad[3] + MV_COST2 (lambda, bit_x2, bit_y3),
      sad[4] + MV_COST2 (lambda, bit_x1, bit_y1),
      sad[5] + MV_COST2 (lambda, bit_x3, bit_y1),
      sad[6] + MV_COST2 (lambda, bit_x1, bit_y3),
      sad[7] + MV_COST2 (lambda, bit_x3, bit_y3)
    };
    int j = 0;
    for(int i = 1; i < 9; i++)
      if (cost[i] < cost[j])
        j = i;
    const int dx[9] = { *xdelta, 2, 1, 3, 2, 1, 3, 1, 3};
    const int dy[9] = { *ydelta, 1, 2, 2, 3, 1, 1, 3, 3};
    min_cost = cost[j];
    *xdelta = dx[j];
    *ydelta = dy[j];
  }
#endif
  return min_cost;
}


static int taa_h264_quarter_search_8x16 (
  const int       stride,
  const uint8_t * mblock,                                       /* Input macroblock */
  const uint8_t * full_y,                                         /* Full pels */
  const uint8_t * half_h,                                     /* Horizontal half pels */
  const uint8_t * half_v,                                    /* Vertical half pels */
#if HPEL_DIAG_ENABLED
  const uint8_t * half_d,                                    /* Vertical half pels */
#endif
  uint32_t        min_cost,                                     /* The minimal cost so far */
  int *           ydelta,                                       /* Out: Quarter pels to best position */
  int *           xdelta,                                       /* Out: Quarter pels to best position */
  const int       mvdy,                                         /* The MV of the best full_y pel position */
  const int       mvdx,                                         /* The MV of the best full_y pel position */
  const int       lambda)                                       /* Lagrange multiplier */
{
  /* Search all quarter pels around the best full_y pel */
  const int zfrac = 4 * (*ydelta & 3) + (*xdelta & 3);

  if (zfrac == 0) // fullpel pos 0
  {
    uint32_t sad[8];
    //taa_h264_qp_sad8x16d4(stride, mblock, &full_y[0],       &half_v[-stride], &half_h[-1], &sad[0]);  // qpel diamond: up, down, left, right
    taa_h264_qp_sad8x16x4d4(stride, mblock, full_y, &half_v[-stride], &half_h[-1], &sad[0]);  // qpel x-cross: up-left, up-right, down-left, down-right

    const int bit_x0 = _lmbd(abs(mvdx + 0));
    const int bit_y0 = _lmbd(abs(mvdy + 0));
    const int bit_x1 = _lmbd(abs(mvdx - 1));
    const int bit_y1 = _lmbd(abs(mvdy - 1));
    const int bit_x2 = _lmbd(abs(mvdx + 1));
    const int bit_y2 = _lmbd(abs(mvdy + 1));

    const uint32_t cost[9] =
    {
      min_cost,
      sad[0] + MV_COST2 (lambda, bit_x0, bit_y1),
      sad[1] + MV_COST2 (lambda, bit_x0, bit_y2),
      sad[2] + MV_COST2 (lambda, bit_x1, bit_y0),
      sad[3] + MV_COST2 (lambda, bit_x2, bit_y0),
      sad[4] + MV_COST2 (lambda, bit_x1, bit_y1),
      sad[5] + MV_COST2 (lambda, bit_x2, bit_y1),
      sad[6] + MV_COST2 (lambda, bit_x1, bit_y2),
      sad[7] + MV_COST2 (lambda, bit_x2, bit_y2)
    };

    int j = 0;
    for(int i = 1; i < 9; i++)
      if (cost[i] < cost[j] )
        j = i;

    const int dx[9] = { *xdelta, 0, 0, -1, 1,  -1, 1, -1, 1};
    const int dy[9] = { *ydelta, -1, 1, 0, 0,  -1, -1, 1, 1};

    min_cost = cost[j];
    *xdelta = dx[j];
    *ydelta = dy[j];

  }
  else if (zfrac == 2) // halfpel pos 2
  {
    uint32_t sad[6];
    const int a = *xdelta + 2;
    const int b = (*xdelta - 2) >> 2;

    taa_h264_qp_sad8x16h2(stride, mblock, &half_h[b], &full_y[b],          &sad[0]); // horizontal 2 positions
    taa_h264_qp_sad8x16v4(stride, mblock, &full_y[b], &half_h[b], &half_v[b - stride], &sad[2]); // vertical 4 positions

    const int bit_x0 = _lmbd(abs(mvdx + a-3));
    const int bit_x1 = _lmbd(abs(mvdx + a-1));
    const int bit_y0 = _lmbd(abs(mvdy +   0));
    const int bit_y1 = _lmbd(abs(mvdy -   1));
    const int bit_y2 = _lmbd(abs(mvdy +   1));

    const uint32_t cost[7] =
    {
      min_cost,
      sad[0] + MV_COST2 (lambda, bit_x0, bit_y0),
      sad[1] + MV_COST2 (lambda, bit_x1, bit_y0),
      sad[2] + MV_COST2 (lambda, bit_x0, bit_y1),
      sad[3] + MV_COST2 (lambda, bit_x1, bit_y1),
      sad[4] + MV_COST2 (lambda, bit_x0, bit_y2),
      sad[5] + MV_COST2 (lambda, bit_x1, bit_y2),
    };

    int j = 0;
    for(int i = 1; i < 7; i++)
      if (cost[i] < cost[j] )
        j = i;
    const int dx[7] = { *xdelta, a-3, a-1, a-3, a-1, a-3, a-1};
    const int dy[7] = { *ydelta,   0,   0,  -1,  -1,   1,   1};

    min_cost = cost[j];
    *xdelta = dx[j];
    *ydelta = dy[j];
  }
  else if (zfrac == 8) // halfpel pos 8
  {
    uint32_t sad[6];
    const int a = *ydelta + 2;
    const int b = (*ydelta - 2) >> 2;

    taa_h264_qp_sad8x16h4(stride, mblock, &full_y[b * stride], &half_h [b * stride - 1], &half_v[b * stride], &sad[0]); // vertical 4 positions
    taa_h264_qp_sad8x16v2(stride, mblock, &half_v[b * stride    ],  &full_y[b * stride], &sad[4]); // horizontal 2 positions

    const int bit_x0 = _lmbd(abs(mvdx -   1));
    const int bit_x1 = _lmbd(abs(mvdx +   1));
    const int bit_x2 = _lmbd(abs(mvdx +   0));
    const int bit_y0 = _lmbd(abs(mvdy + a-3));
    const int bit_y1 = _lmbd(abs(mvdy + a-1));

    const uint32_t cost[7] =
    {
      min_cost,
      sad[0] + MV_COST2 (lambda, bit_x0, bit_y0),
      sad[1] + MV_COST2 (lambda, bit_x0, bit_y1),
      sad[2] + MV_COST2 (lambda, bit_x1, bit_y0),
      sad[3] + MV_COST2 (lambda, bit_x1, bit_y1),
      sad[4] + MV_COST2 (lambda, bit_x2, bit_y0),
      sad[5] + MV_COST2 (lambda, bit_x2, bit_y1)
    };
    int j = 0;
    for(int i = 1; i < 7; i++)
      if (cost[i] < cost[j] )
        j = i;

    const int dx[7] = { *xdelta,  -1,  -1,   1,   1,   0,   0};
    const int dy[7] = { *ydelta, a-3, a-1, a-3, a-1, a-3, a-1};

    min_cost = cost[j];
    *xdelta = dx[j];
    *ydelta = dy[j];
  }
#if HPEL_DIAG_ENABLED
  else // halfpel pos 10
  {
    uint32_t sad[8];
    taa_h264_qp_sad8x16d4(stride, mblock, full_y, half_h, half_v, half_d, &sad[0]); // diamond around diagonal hpel
    taa_h264_qp_sad8x16x4(stride, mblock, full_y, half_h, half_v, half_d, &sad[4]); // square around diagonal hpel

    const int bit_x1 = _lmbd(abs(mvdx + 1));
    const int bit_x2 = _lmbd(abs(mvdx + 2));
    const int bit_x3 = _lmbd(abs(mvdx + 3));
    const int bit_y1 = _lmbd(abs(mvdy + 1));
    const int bit_y2 = _lmbd(abs(mvdy + 2));
    const int bit_y3 = _lmbd(abs(mvdy + 3));

    const uint32_t cost[9] =
    {
      min_cost,
      sad[0] + MV_COST2 (lambda, bit_x2, bit_y1),
      sad[1] + MV_COST2 (lambda, bit_x1, bit_y2),
      sad[2] + MV_COST2 (lambda, bit_x3, bit_y2),
      sad[3] + MV_COST2 (lambda, bit_x2, bit_y3),
      sad[4] + MV_COST2 (lambda, bit_x1, bit_y1),
      sad[5] + MV_COST2 (lambda, bit_x3, bit_y1),
      sad[6] + MV_COST2 (lambda, bit_x1, bit_y3),
      sad[7] + MV_COST2 (lambda, bit_x3, bit_y3)
    };
    int j = 0;
    for(int i = 1; i < 9; i++)
      if (cost[i] < cost[j])
        j = i;
    const int dx[9] = { *xdelta, 2, 1, 3, 2, 1, 3, 1, 3};
    const int dy[9] = { *ydelta, 1, 2, 2, 3, 1, 1, 3, 3};
    min_cost = cost[j];
    *xdelta = dx[j];
    *ydelta = dy[j];
  }
#endif
  return min_cost;
}



// ======================================================================================================
//
// ======================================================================================================
#include "enc_motionsearch.h"
#define MAX_SAD_VALUE               65535
#define SEARCH_RANGE_16x16          32

typedef union
{ 
  uint16_t u16[4]; 
  uint64_t u64;
} sad_t;

typedef struct
{
  int x;
  int y;
  uint32_t sad;
  uint32_t cost; // sad + mvcost
} taa_mvsad_t;

typedef struct
{
  int x;
  int y;
  uint32_t mvcost16x16;
  sad_t sad8x8;
} taa_trace_t;

typedef struct
{
  int8_t x;
  int8_t y;
} mvpos_t;


typedef enum {
  ALL,
  UP,
  DOWN,
  LEFT, RIGHT
} partition;

typedef enum {
  DIR_UP,
  DIR_DOWN,
  DIR_LEFT,
  DIR_RIGHT,
  DIR_NONE
} direction;


#if 0
TAA_H264_NO
void taa_h264_prefetch_searchregion(
  const int       stride,
  const uint8_t * frame_full_y,
  const uint8_t * frame_half_h,
  const uint8_t * frame_half_v,
  uint8_t         full_y[32 * 64],
  uint8_t         half_h[32 * 64],
  uint8_t         half_v[32 * 64]
  )
{
  __assume_aligned(full_y, 64);
  for(int j = 0; j < 32; j++)
  {
    _mm_prefetch((const char *)(&frame_full_y[(j + 2) * stride]), _MM_HINT_T0);
# pragma unroll(4)
    for(int i = 0; i < 64; i++)
      full_y[i + j * 64] = frame_full_y[i + j * stride];
  }
  __assume_aligned(half_h, 64);
  for(int j = 0; j < 32; j++)
  {
    _mm_prefetch((const char *)(&frame_half_h[(j + 2) * stride]), _MM_HINT_T0);
# pragma unroll(4)
    for(int i = 0; i < 64; i++)
      half_h[i + j * 64] = frame_half_h[i + j * stride];
  }
  __assume_aligned(half_v, 64);
  for(int j = 0; j < 32; j++)
  {
    _mm_prefetch((const char *)(&frame_half_v[(j + 2) * stride]), _MM_HINT_T0);
# pragma unroll(4)
    for(int i = 0; i < 64; i++)
      half_v[i + j * 64] = frame_half_v[i + j * stride];
  }
}
#endif

static void taa_h264_motion_estimate_mb (
  const int          stride,
  const int          cstride,
  const int          qp,
  const int          lambda,
  const int          thr,
  meinfo_t *         me,
  const int          xref,
  const int          yref,
  const uint8_t *    mblock,
  const mv_t         cand[4],
  mvpos              pred,
  const uint8_t *    full_y,
  const uint8_t *    full_u,
  const uint8_t *    full_v,
#if HPEL_DIAG_ENABLED
  const uint8_t *    half_d,
#endif
  const uint8_t *    half_h,
  const uint8_t *    half_v,
  uint8_t *  recon_y,
  uint8_t *  inter_uv
  )
{
  const int c_xmin = -me->xpos - 19;
  const int c_ymin = -me->ypos - 19;
  const int c_xmax = me->width - me->xpos - MB_WIDTH_Y + 18;
  const int c_ymax = me->height - me->ypos - MB_HEIGHT_Y + 18;

  //====================================================================
  //                          Search window
  //====================================================================
#if 0
  TAA_H264_ALIGN (64) uint8_t full_y[32 * 64];
  TAA_H264_ALIGN (64) uint8_t half_h[32 * 64];
  TAA_H264_ALIGN (64) uint8_t half_v[32 * 64];

  taa_h264_prefetch_searchregion(
    stride,
    frame_full_y, frame_half_h, frame_half_v,
    full_y, half_h, half_v);
#endif
  //====================================================================

  taa_mvsad_t best_all, best_up, best_down, best_left, best_right;
  taa_trace_t trace[SEARCH_RANGE_16x16 * 4 + 8];

  int trace_count = 0;
  /*
   * 1. Select a candidate vector for the whole macroblock.
   * 2. Do an integer-pixel search in a diamond pattern from the candidate vector position:
   * a) First one step out
   * b) Second step out
   * c) A double step
   * d) A number of quadrouple steps, until max vector or edge
   * e) In a-d stop if no improvement over two iterations
   * 3. Half-pixel search around the best integer position
   * 4. Quarter-pixel search around the best half-pixel position
   */
  int cost_min = 132000;     /* 2*256*256 = 131072 */
  int cost_all, cost_up, cost_down, cost_left, cost_right;
  mbtype_t mode;
  mvpos_t c[4];

  c[0].x = (int8_t)((cand[0].x + 2) >> 2);
  c[0].y = (int8_t)((cand[0].y + 2) >> 2);
  c[1].x = (int8_t)((cand[1].x + 2) >> 2);
  c[1].y = (int8_t)((cand[1].y + 2) >> 2);
  c[2].x = (int8_t)((cand[2].x + 2) >> 2);
  c[2].y = (int8_t)((cand[2].y + 2) >> 2);
  c[3].x = (int8_t)((cand[3].x + 2) >> 2);
  c[3].y = (int8_t)((cand[3].y + 2) >> 2);

  uint32_t sad[4];
  uint32_t cost[4];
  uint32_t mvcost[4];

  //==================================================================
  //                         Always check (0,0)
  //==================================================================
  {
    sad_t sad8x8;
    const int vdx = -pred.x;
    const int vdy = -pred.y;
    const uint32_t mvcost0 = MV_COST (lambda, vdx, vdy);
    const int sad0 = taa_h264_split_sad_4x8x8(stride, mblock, &full_y[0], sad8x8.u16);

    const int cost0 = mvcost0 + sad0;
    // save trace
    trace[trace_count].x = 0;
    trace[trace_count].y = 0;
    trace[trace_count].mvcost16x16 = mvcost0;
    trace[trace_count].sad8x8.u64 = sad8x8.u64;
    trace_count++;

    best_all.cost = cost0;
    best_all.sad = sad0;
    best_all.x = 0;
    best_all.y = 0;
  }

  //==================================================================
  // If first 4 candidates is a diamond go directly to diamond search
  // Statistically this is slightly more common than the opposite
  //==================================================================
  if (*(int64_t *) c == 0x000100ff0100ff00)
  {
    sad_t sad_matrix[4];
    const int curr_x   = best_all.x;
    const int curr_y   = best_all.y;
    const int curr_mvx = 4*curr_x;
    const int curr_mvy = 4*curr_y;
    taa_h264_diamond_sad16x8x8(
      stride,
      mblock,
      full_y + (curr_y * stride + curr_x),
      pred.x,
      pred.y,
      curr_mvx,
      curr_mvy,
      lambda,
      mvcost,
      sad,
      sad_matrix[0].u16);

    bool dead_end = true;
    for(int j = 0; j<4; j++)
    {
      trace[trace_count].mvcost16x16 = mvcost[j];
      cost[j] = mvcost[j] + sad[j];
      trace[trace_count].sad8x8.u64 = sad_matrix[j].u64;
      trace[trace_count].x = c[j].x;
      trace[trace_count].y = c[j].y;
      trace_count++;

      if (cost[j] < best_all.cost)
      {
        best_all.cost = cost[j];
        best_all.sad = sad[j];
        best_all.x = c[j].x;
        best_all.y = c[j].y;
        dead_end = false;
      }
    }

    if (dead_end)
    {
      const int max_sad_8x8 = max(max(
        trace[0].sad8x8.u16[0],
        trace[0].sad8x8.u16[1]), max(
        trace[0].sad8x8.u16[2],
        trace[0].sad8x8.u16[3]));

      // If none of the diamond positions returns a better result than (0,0) and
      // if (0,0) has a low enough cost, we choose that one without checking
      // half and quarter positions.
      if (max_sad_8x8 < thr)
      {
        taa_h264_save_best_16x16(
          stride,
          0,
          full_y,
          recon_y);
        taa_h264_save_best_chroma_8x8(
          0,
          0,
          cstride,
          full_u,
          full_v,
          inter_uv);

        me->best_cost = best_all.cost;
        me->sad8x8[0] = trace[0].sad8x8.u16[0];
        me->sad8x8[1] = trace[0].sad8x8.u16[1];
        me->sad8x8[2] = trace[0].sad8x8.u16[2];
        me->sad8x8[3] = trace[0].sad8x8.u16[3];
        me->best_mv[0].x = 0;
        me->best_mv[0].y = 0;
        me->best_mv[1].x = 0;
        me->best_mv[1].y = 0;
        me->best_mode = P_16x16;

        return;
      }
    }
  }
  else
  {
    sad_t sad8x8;
    // Cand 0
    {
      const int vdx = cand[0].x - pred.x;
      const int vdy = cand[0].y - pred.y;
      mvcost[0] = MV_COST (lambda, vdx, vdy);
      sad[0] = taa_h264_split_sad_4x8x8(stride, mblock, &full_y[0+c[0].y * stride + c[0].x], sad8x8.u16);
      cost[0] = mvcost[0] + sad[0];
      // save trace
      trace[trace_count].x = c[0].x;
      trace[trace_count].y = c[0].y;
      trace[trace_count].mvcost16x16 = mvcost[0];
      trace[trace_count].sad8x8.u64 = sad8x8.u64;
      trace_count++;
    }

    // Cand 1
    {
      const int vdx = cand[1].x - pred.x;
      const int vdy = cand[1].y - pred.y;
      mvcost[1] = MV_COST (lambda, vdx, vdy);
      sad[1] = taa_h264_split_sad_4x8x8(stride, mblock, &full_y[0+c[1].y * stride + c[1].x], sad8x8.u16);
      cost[1] = mvcost[1] + sad[1];
      // save trace
      trace[trace_count].x = c[1].x;
      trace[trace_count].y = c[1].y;
      trace[trace_count].mvcost16x16 = mvcost[1];
      trace[trace_count].sad8x8.u64 = sad8x8.u64;
      trace_count++;
    }

    // Cand 2
    {
      const int vdx = cand[2].x - pred.x;
      const int vdy = cand[2].y - pred.y;
      mvcost[2] = MV_COST (lambda, vdx, vdy);
      sad[2] = taa_h264_split_sad_4x8x8(stride, mblock, &full_y[0+c[2].y * stride + c[2].x], (uint16_t *) &sad8x8);
      cost[2] = mvcost[2] + sad[2];
      // save trace
      trace[trace_count].x = c[2].x;
      trace[trace_count].y = c[2].y;
      trace[trace_count].mvcost16x16 = mvcost[2];
      trace[trace_count].sad8x8.u64 = sad8x8.u64;
      trace_count++;
    }

    // Cand 3
    {
      const int vdx = cand[3].x - pred.x;
      const int vdy = cand[3].y - pred.y;
      mvcost[3] = MV_COST (lambda, vdx, vdy);
      sad[3] = taa_h264_split_sad_4x8x8(stride, mblock, &full_y[0+c[3].y * stride + c[3].x], (uint16_t *) &sad8x8);
      cost[3] = mvcost[3] + sad[3];
      // save trace
      trace[trace_count].x = c[3].x;
      trace[trace_count].y = c[3].y;
      trace[trace_count].mvcost16x16 = mvcost[3];
      trace[trace_count].sad8x8.u64 = sad8x8.u64;
      trace_count++;
    }

    for(int i = 0; i < 4; i++)
    {
      if (cost[i] < best_all.cost)
      {
        best_all.cost = cost[i];
        best_all.sad = sad[i];
        best_all.x = c[i].x;
        best_all.y = c[i].y;
      }
    }
  }
  //==================================================================
  //              16x16, 16x8, 8x16 hybrid diamond search
  //==================================================================
  {
    sad_t sad_matrix[4];
    const int best_mvx = 4*best_all.x;
    const int best_mvy = 4*best_all.y;
    const int dist = (pred.x - best_mvx) * (pred.x - best_mvx) + (pred.y - best_mvy) * (pred.y - best_mvy);
    int step = 1;
    if (dist>(4*4*2*2)) step = 2;
    if (dist>(4*4*4*4)) step = 4;
    if (dist>(4*4*8*8)) step = 8;
    //if (dist>(4*4*16*16)) step = 16;

    int centre_x = best_all.x;
    int centre_y = best_all.y;

    for(int i = 0; i<SEARCH_RANGE_16x16; i++)
    {
      taa_h264_diamond_crude_sad16x8x8(
        stride,
        step,
        mblock,
        full_y + (centre_y * stride + centre_x),
        pred.x,
        pred.y,
        4*centre_x,
        4*centre_y,
        lambda,
        mvcost,
        sad,
        sad_matrix[0].u16);

      const int c_x[4] = {centre_x, centre_x, centre_x - step, centre_x + step};
      const int c_y[4] = {centre_y - step, centre_y + step, centre_y, centre_y};

      bool dead_end = true;
      for(int j = 0; j<4; j++)
      {
        if ((c_x[j] >= c_xmin) && (c_x[j] <= c_xmax)
            && (c_y[j] >= c_ymin) && (c_y[j] <= c_ymax))
        {
          trace[trace_count].mvcost16x16 = mvcost[j];
          cost[j] = mvcost[j] + sad[j];
          trace[trace_count].sad8x8.u64 = sad_matrix[j].u64;
          trace[trace_count].x = c_x[j];
          trace[trace_count].y = c_y[j];
          trace_count++;
          if(cost[j] < best_all.cost)
          {
            best_all.cost = cost[j];
            best_all.sad  = sad[j];
            best_all.x    = centre_x = c_x[j];
            best_all.y    = centre_y = c_y[j];
            dead_end = false;
          }
        }
      }
      if (dead_end)
      {
        if (step==1) break;  // Terminate
        step = step >> 1;
      }
    }
  }

  //==============================================================
  //                 8x16 and 16x8 TRACE SEARCH
  //==============================================================
  {

    int point;                  /* index of search trace */
    int point0 = 0;
    int point1 = 0;
    int point2 = 0;
    int point3 = 0;

    int sadtop_best;
    int sadbottom_best;
    int sadleft_best;
    int sadright_best;

    int pointcounter = 0;

    sadtop_best = sadbottom_best = sadleft_best = sadright_best  = 1000000;


    /*  for half-pel approximation, we need to save the sads of the last diamond           */
    /*  so, for whichever blocks we are summing, we have to save the sums                  */
    /*  and then go back and look at the diamond around our best position.  this requires  */
    /*  that we do (at least) a single diamond around the best sad, if it was not          */
    /*  already used as a center                                                           */

    /* scan all of the searched points to find the minimum sad */


    // 8x8 SAD stored this way
    //
    //        0  1
    //        2  3
    //
    for (point = 0; point < trace_count; point++)
    {
      const int sad0 = trace[pointcounter].sad8x8.u16[0] + trace[pointcounter].sad8x8.u16[1]; //UP 16x8
      const int sad1 = trace[pointcounter].sad8x8.u16[2] + trace[pointcounter].sad8x8.u16[3]; //DOWN 16x8
      const int sad2 = trace[pointcounter].sad8x8.u16[0] + trace[pointcounter].sad8x8.u16[2]; //LEFT 8x16
      const int sad3 = trace[pointcounter].sad8x8.u16[1] + trace[pointcounter].sad8x8.u16[3]; //RIGHT 8x16

      const int sadtop    = sad0 + trace[pointcounter].mvcost16x16; //UP 16x8
      const int sadbottom = sad1 + trace[pointcounter].mvcost16x16; //DOWN 16x8
      const int sadleft   = sad2 + trace[pointcounter].mvcost16x16; //LEFT 8x16
      const int sadright  = sad3 + trace[pointcounter].mvcost16x16; //RIGHT 8x16

      if (sadtop < sadtop_best) //UP 16x8
      {
        sadtop_best = sadtop;
        point0 = pointcounter;
      }
      if (sadbottom < sadbottom_best) //DOWN 16x8
      {
        sadbottom_best = sadbottom;
        point1 = pointcounter;
      }
      if (sadleft < sadleft_best) //LEFT 8x16
      {
        sadleft_best = sadleft;
        point2 = pointcounter;
      }
      if (sadright < sadright_best) //RIGHT 8x16
      {
        sadright_best = sadright;
        point3 = pointcounter;
      }
      pointcounter++;
    }

    best_up.x    = trace[point0].x; //UP 16x8
    best_up.y    = trace[point0].y;
    best_up.cost = sadtop_best;
    best_up.sad  = sadtop_best - trace[point0].mvcost16x16;

    best_down.x    = trace[point1].x; //DOWN 16x8
    best_down.y    = trace[point1].y;
    best_down.cost = sadbottom_best;
    best_down.sad  = sadbottom_best - trace[point1].mvcost16x16;

    best_left.x    = trace[point2].x; //LEFT 8x16
    best_left.y    = trace[point2].y;
    best_left.cost = sadleft_best;
    best_left.sad  = sadleft_best - trace[point2].mvcost16x16;

    best_right.x    = trace[point3].x; //RIGHT 8x16
    best_right.y    = trace[point3].y;
    best_right.cost = sadright_best;
    best_right.sad  = sadright_best - trace[point3].mvcost16x16;

    //==============================================================
    //                     HALF PEL SEARCH
    //==============================================================
    me->best_mv[0].x = me->best_mv[1].x = best_all.x - xref;
    me->best_mv[0].y = me->best_mv[1].y = best_all.y - yref;

    //===============================================
    int xdelta_all = 0;
    int ydelta_all = 0;
    int xdelta_up = 0;
    int ydelta_up = 0;
    int xdelta_down = 0;
    int ydelta_down = 0;
    int xdelta_left = 0;
    int ydelta_left = 0;
    int xdelta_right = 0;
    int ydelta_right = 0;

    cost_all = best_all.cost;
    cost_up = best_up.cost;
    cost_down = best_down.cost;
    cost_left = best_left.cost;
    cost_right = best_right.cost;


#if HPEL_DIAG_ENABLED
    /* Half-pel search diag 16x16 ALL */
    {
      cost_all = taa_h264_diag_half_search_16x16 (
        stride,
        mblock,
        &full_y[0+best_all.y*stride + best_all.x],
        &half_d[0+best_all.y*stride + best_all.x],
        cost_all,
        &ydelta_all,
        &xdelta_all,
        4 * best_all.y - pred.y,
        4 * best_all.x - pred.x,
        lambda);
    }

    /* DIAG Half-pel search 16x8 UP */
    {
      cost_up = taa_h264_diag_half_search_16x8 (
        stride,
        mblock,
        &full_y[0+best_up.y*stride + best_up.x],
        &half_d[0+best_up.y*stride + best_up.x],
        cost_up,
        &ydelta_up,
        &xdelta_up,
        4 * best_up.y - pred.y,
        4 * best_up.x - pred.x,
        lambda);
    }

    /* DIAG Half-pel search 16x8 DOWN */
    {
      cost_down = taa_h264_diag_half_search_16x8 (
        stride,
        &mblock[8 * MB_WIDTH_Y],
        &full_y[0+(best_down.y+8)*stride + best_down.x],
        &half_d[0+(best_down.y+8)*stride + best_down.x],
        cost_down,
        &ydelta_down,
        &xdelta_down,
        4 * best_down.y - pred.y,
        4 * best_down.x - pred.x,
        lambda);
    }

    /* DIAG Half-pel search 8x16 LEFT */
    {
      cost_left = taa_h264_diag_half_search_8x16 (
        stride,
        mblock,
        &full_y[0+best_left.y*stride + best_left.x],
        &half_d[0+best_left.y*stride + best_left.x],
        cost_left,
        &ydelta_left,
        &xdelta_left,
        4 * best_left.y - pred.y,
        4 * best_left.x - pred.x,
        lambda);
    }

    /* DIAG Half-pel search 8x16 RIGHT */
    {
      cost_right = taa_h264_diag_half_search_8x16 (
        stride,
        &mblock[8],
        &full_y[0+8 + best_right.y*stride + best_right.x],
        &half_d[0+8 + best_right.y*stride + best_right.x],
        cost_right,
        &ydelta_right,
        &xdelta_right,
        4 * best_right.y - pred.y,
        4 * best_right.x - pred.x,
        lambda);
    }
#endif


#if 1
    /* Half-pel search 16x16 ALL */
    {
      cost_all = taa_h264_half_search_16x16 (
        stride,
        mblock,
        &full_y[0+best_all.y*stride + best_all.x],
        &half_h[0+best_all.y*stride + best_all.x],
        &half_v[0+best_all.y*stride + best_all.x],
        cost_all,
        &ydelta_all,
        &xdelta_all,
        4 * best_all.y - pred.y,
        4 * best_all.x - pred.x,
        lambda);
    }

    /* Half-pel search 16x8 UP */
    {
      cost_up = taa_h264_half_search_16x8 (
        stride,
        mblock,
        &full_y[0+best_up.y*stride + best_up.x],
        &half_h[0+best_up.y*stride + best_up.x],
        &half_v[0+best_up.y*stride + best_up.x],
        cost_up,
        &ydelta_up,
        &xdelta_up,
        4 * best_up.y - pred.y,
        4 * best_up.x - pred.x,
        lambda);
    }

    /* Half-pel search 16x8 DOWN */
    {
      cost_down = taa_h264_half_search_16x8 (
        stride,
        &mblock[8 * MB_WIDTH_Y],
        &full_y[0+(best_down.y+8)*stride + best_down.x],
        &half_h[0+(best_down.y+8)*stride + best_down.x],
        &half_v[0+(best_down.y+8)*stride + best_down.x],
        cost_down,
        &ydelta_down,
        &xdelta_down,
        4 * best_down.y - pred.y,
        4 * best_down.x - pred.x,
        lambda);
    }

    /* Half-pel search 8x16 LEFT */
    {
      cost_left = taa_h264_half_search_8x16 (
        stride,
        mblock,
        &full_y[0+best_left.y*stride + best_left.x],
        &half_h[0+best_left.y*stride + best_left.x],
        &half_v[0+best_left.y*stride + best_left.x],
        cost_left,
        &ydelta_left,
        &xdelta_left,
        4 * best_left.y - pred.y,
        4 * best_left.x - pred.x,
        lambda);
    }

    /* Half-pel search 8x16 RIGHT */
    {
      cost_right = taa_h264_half_search_8x16 (
        stride,
        &mblock[8],
        &full_y[0+8 + best_right.y*stride + best_right.x],
        &half_h[0+8 + best_right.y*stride + best_right.x],
        &half_v[0+8 + best_right.y*stride + best_right.x],
        cost_right,
        &ydelta_right,
        &xdelta_right,
        4 * best_right.y - pred.y,
        4 * best_right.x - pred.x,
        lambda);
    }
#endif
    mode = P_16x16;
    cost_min = cost_all;

    me->best_mv[0].x = me->best_mv[1].x = 4 * best_all.x;
    me->best_mv[0].y = me->best_mv[1].y = 4 * best_all.y;
    me->best_cost = cost_min;

    if ((cost_up + cost_down) < cost_min)
    {
      mode = P_16x8;
      cost_min = cost_up + cost_down;
      me->best_mv[0].x = 4 * best_up.x;
      me->best_mv[0].y = 4 * best_up.y;
      me->best_mv[1].x = 4 * best_down.x;
      me->best_mv[1].y = 4 * best_down.y;
      me->best_cost = cost_min;
    }
    if ((cost_left + cost_right) < cost_min)
    {
      mode = P_8x16;
      cost_min = cost_left + cost_right;
      me->best_mv[0].x = 4 * best_left.x;
      me->best_mv[0].y = 4 * best_left.y;
      me->best_mv[1].x = 4 * best_right.x;
      me->best_mv[1].y = 4 * best_right.y;
      me->best_cost = cost_min;
    }
#if 1
    //==============================================================
    //                    QUARTER PEL SEARCH
    //==============================================================
    if (mode == P_16x16)
    {
      // 16x16
      {
        cost_min = taa_h264_quarter_search_16x16 (
          stride,
          mblock,
          &full_y[0+best_all.y*stride + best_all.x],
          &half_h[0+best_all.y*stride + best_all.x],
          &half_v[0+best_all.y*stride + best_all.x],
#if HPEL_DIAG_ENABLED
          &half_d[0+best_all.y*stride + best_all.x],
#endif
          cost_min,
          &ydelta_all,
          &xdelta_all,
          me->best_mv[0].y - pred.y,
          me->best_mv[0].x - pred.x,
          lambda);
      }
      // Save 16x16
      {
        int const xint = xdelta_all >> 2;
        int const yint = ydelta_all >> 2;
        int const cxx  = (best_all.x + xint) >> 1;
        int const cyy  = (best_all.y + yint) >> 1;
        int const zfrac = 4 * (ydelta_all & 3) + (xdelta_all & 3);
        int const xfrac = (me->best_mv[0].x + xdelta_all) & 7;
        int const yfrac = (me->best_mv[0].y + ydelta_all) & 7;

        taa_h264_save_best_16x16(
          stride,
          zfrac,
          &full_y[0 + (best_all.y + yint) * stride  + best_all.x + xint],
          recon_y);
        taa_h264_save_best_chroma_8x8(
          xfrac,
          yfrac,
          cstride,
          &full_u[cyy*cstride + cxx],
          &full_v[cyy*cstride + cxx],
          inter_uv);
      }
      // Check 16x16
      uint16_t sad8x8[4];
      taa_h264_split_sad_aligned_4x8x8(recon_y, mblock, sad8x8);
      const int sad_check = sad8x8[0] + sad8x8[1] + sad8x8[2] + sad8x8[3];
      const int cost_check = sad_check + MV_COST (
        lambda, me->best_mv[0].x - pred.x + xdelta_all,
        me->best_mv[0].y - pred.y + ydelta_all);

# if SANITY_MV_SEARCH
      if (cost_check != cost_min)
      {
        fprintf(stderr, "error 16x16 cost check: check=%d != min=%d\n", cost_check, cost_min);
        fflush(stderr);
        TAA_H264_DEBUG_ASSERT(cost_check == cost_min);
      }
# endif

      //================================================
      me->sad8x8[0] = sad8x8[0];
      me->sad8x8[1] = sad8x8[1];
      me->sad8x8[2] = sad8x8[2];
      me->sad8x8[3] = sad8x8[3];
      me->best_cost = cost_check;
      me->best_mv[0].x += xdelta_all;
      me->best_mv[0].y += ydelta_all;
      me->best_mv[1].x = me->best_mv[0].x;
      me->best_mv[1].y = me->best_mv[0].y;

    }
    else if (mode == P_16x8)
    {
      // 16x8 up
      {
        cost_up = taa_h264_quarter_search_16x8 (
          stride,
          mblock,
          &full_y[0+best_up.y*stride + best_up.x],
          &half_h[0+best_up.y*stride + best_up.x],
          &half_v[0+best_up.y*stride + best_up.x],
#if HPEL_DIAG_ENABLED
          &half_d[0+best_up.y*stride + best_up.x],
#endif
          cost_up,
          &ydelta_up,
          &xdelta_up,
          me->best_mv[0].y - pred.y,
          me->best_mv[0].x - pred.x,
          lambda);
        TAA_H264_DEBUG_ASSERT (ydelta_up > -4 && ydelta_up < 4 && xdelta_up > -4 && xdelta_up < 4);
      }
      // Save 16x8 up
      {
        int const xint  = xdelta_up >> 2;
        int const yint  = ydelta_up >> 2;
        int const cxx  = (best_up.x + xint) >> 1;
        int const cyy  = (best_up.y + yint) >> 1;
        int const zfrac = 4 * (ydelta_up & 3) + (xdelta_up & 3);
        int const xfrac = (me->best_mv[0].x + xdelta_up) & 7;
        int const yfrac = (me->best_mv[0].y + ydelta_up) & 7;
        taa_h264_save_best_16x8(
          stride,
          zfrac,
          &full_y[0 + (best_up.y + yint)*stride  + best_up.x + xint],
          recon_y);
        taa_h264_save_best_chroma_8x4(
          xfrac,
          yfrac,
          cstride,
          &full_u[cyy*cstride + cxx],
          &full_v[cyy*cstride + cxx],
          inter_uv);
      }
      // 16x8 down
      {
        cost_down = taa_h264_quarter_search_16x8 (
          stride,
          &mblock[8 * MB_WIDTH_Y],
          &full_y[0+(best_down.y+8)*stride + best_down.x],
          &half_h[0+(best_down.y+8)*stride + best_down.x],
          &half_v[0+(best_down.y+8)*stride + best_down.x],
#if HPEL_DIAG_ENABLED
          &half_d[0+(best_down.y+8)*stride + best_down.x],
#endif
          cost_down,
          &ydelta_down,
          &xdelta_down,
          me->best_mv[1].y - pred.y,
          me->best_mv[1].x - pred.x,
          lambda);
        TAA_H264_DEBUG_ASSERT (ydelta_down > -4 && ydelta_down < 4 && xdelta_down > -4 && xdelta_down < 4);
      }
      // Save 16x8 down
      {
        int const xint  = xdelta_down >> 2;
        int const yint  = ydelta_down >> 2;
        int const cxx  = ((best_down.x + 0 + xint) >> 1);
        int const cyy  = ((best_down.y + 8 + yint) >> 1);
        int const zfrac = 4 * (ydelta_down & 3) + (xdelta_down & 3);
        int const xfrac = (me->best_mv[1].x + xdelta_down) & 7;
        int const yfrac = (me->best_mv[1].y + ydelta_down) & 7;
        taa_h264_save_best_16x8(
          stride,
          zfrac,
          &full_y[0 + (best_down.y + 8 + yint)*stride  + best_down.x + xint],
          &recon_y[8 * MB_WIDTH_Y]
          );
        taa_h264_save_best_chroma_8x4(
          xfrac,
          yfrac,
          cstride,
          &full_u[cyy*cstride + cxx],
          &full_v[cyy*cstride + cxx],
          &inter_uv[4 * MB_STRIDE_UV]
          );
      }
      // Check 16x8 up + down
      uint16_t sad8x8[4];
      taa_h264_split_sad_aligned_4x8x8(recon_y, mblock, sad8x8);
      const int sad_check = sad8x8[0] + sad8x8[1] + sad8x8[2] + sad8x8[3];
      const int cost_check = sad_check +
                             MV_COST (lambda, me->best_mv[0].x - pred.x + xdelta_up,
                                                me->best_mv[0].y - pred.y + ydelta_up) +
                             MV_COST (lambda, me->best_mv[1].x - pred.x + xdelta_down,
                                                me->best_mv[1].y - pred.y + ydelta_down);

# if SANITY_MV_SEARCH
      if (cost_check != (cost_up + cost_down))
      {
        fprintf(stderr, "error 16x8 cost check: check=%d != (up + down)=%d\n", cost_check, (cost_up + cost_down));
        fflush(stderr);
        TAA_H264_DEBUG_ASSERT (cost_check == (cost_up + cost_down));
      }
# endif
      //================================================
      me->sad8x8[0] = sad8x8[0];
      me->sad8x8[1] = sad8x8[1];
      me->sad8x8[2] = sad8x8[2];
      me->sad8x8[3] = sad8x8[3];
      me->best_cost = cost_check;
      me->best_mv[0].x += xdelta_up;
      me->best_mv[0].y += ydelta_up;
      me->best_mv[1].x += xdelta_down;
      me->best_mv[1].y += ydelta_down;
    }
    else if (mode == P_8x16)
    {
      // 8x16 left
      {
        cost_left = taa_h264_quarter_search_8x16 (
          stride,
          mblock,
          &full_y[0+best_left.y*stride + best_left.x],
          &half_h[0+best_left.y*stride + best_left.x],
          &half_v[0+best_left.y*stride + best_left.x],
#if HPEL_DIAG_ENABLED
          &half_d[0+best_left.y*stride + best_left.x],
#endif
          cost_left,
          &ydelta_left,
          &xdelta_left,
          me->best_mv[0].y - pred.y,
          me->best_mv[0].x - pred.x,
          lambda);
        TAA_H264_DEBUG_ASSERT (ydelta_left > -4 && ydelta_left < 4 && xdelta_left > -4 && xdelta_left < 4);
      }
      // Save 8x16 left
      {
        int const xint  = xdelta_left >> 2;
        int const yint  = ydelta_left >> 2;
        int const cxx  = (best_left.x + xint) >> 1;
        int const cyy  = (best_left.y + yint) >> 1;
        int const zfrac = 4 * (ydelta_left & 3) + (xdelta_left & 3);
        int const xfrac = (me->best_mv[0].x + xdelta_left) & 7;
        int const yfrac = (me->best_mv[0].y + ydelta_left) & 7;

        taa_h264_save_best_8x16(
          stride,
          zfrac,
          &full_y[0 + (best_left.y + yint)*stride  + best_left.x + xint],
          recon_y);
        taa_h264_save_best_chroma_4x8(
          xfrac,
          yfrac,
          cstride,
          &full_u[cyy*cstride + cxx],
          &full_v[cyy*cstride + cxx],
          inter_uv);
      }
      // 8x16 right
      {
        cost_right = taa_h264_quarter_search_8x16 (
          stride,
          &mblock[8],
          &full_y[0+8 + best_right.y*stride + best_right.x],
          &half_h[0+8 + best_right.y*stride + best_right.x],
          &half_v[0+8 + best_right.y*stride + best_right.x],
#if HPEL_DIAG_ENABLED
          &half_d[0+8 + best_right.y*stride + best_right.x],
#endif
          cost_right,
          &ydelta_right,
          &xdelta_right,
          me->best_mv[1].y - pred.y,
          me->best_mv[1].x - pred.x,
          lambda);
        TAA_H264_DEBUG_ASSERT (ydelta_right > -4 && ydelta_right < 4 && xdelta_right > -4 && xdelta_right < 4);
      }
      // Save 8x16 right
      {
        int const xint  = xdelta_right >> 2;
        int const yint  = ydelta_right >> 2;
        int const cxx  = (best_right.x + 8 + xint) >> 1;
        int const cyy  = (best_right.y + 0 + yint) >> 1;
        int const zfrac = 4 * (ydelta_right & 3) + (xdelta_right & 3);
        int const xfrac = (me->best_mv[1].x + xdelta_right) & 7;
        int const yfrac = (me->best_mv[1].y + ydelta_right) & 7;
        taa_h264_save_best_8x16(
          stride,
          zfrac,
          &full_y[0+8 + (best_right.y + yint)*stride  + best_right.x + xint],
          &recon_y[8]
          );
        taa_h264_save_best_chroma_4x8(
          xfrac,
          yfrac,
          cstride,
          &full_u[cyy*cstride + cxx],
          &full_v[cyy*cstride + cxx],
          &inter_uv[4]   // Strange, makes things complicated
          );
      }
      // Check 8x16 left + right
      uint16_t sad8x8[4];
      taa_h264_split_sad_aligned_4x8x8(recon_y, mblock, sad8x8);
      const int sad_check = sad8x8[0] + sad8x8[1] + sad8x8[2] + sad8x8[3];
      const int cost_check = sad_check +
                             MV_COST (lambda, me->best_mv[0].x - pred.x + xdelta_left,
                                                me->best_mv[0].y - pred.y + ydelta_left) +
                             MV_COST (lambda, me->best_mv[1].x - pred.x + xdelta_right,
                                                me->best_mv[1].y - pred.y + ydelta_right);

# if SANITY_MV_SEARCH
      if (cost_check != (cost_left + cost_right))
      {
        fprintf(stderr, "error 8x16 cost check: check=%d != (left + right)=%d\n", cost_check, (cost_left + cost_right));
        fflush(stderr);
        TAA_H264_DEBUG_ASSERT (cost_check == (cost_left + cost_right));
      }
# endif
      me->sad8x8[0] = sad8x8[0];
      me->sad8x8[1] = sad8x8[1];
      me->sad8x8[2] = sad8x8[2];
      me->sad8x8[3] = sad8x8[3];
      me->best_cost = cost_check;
      me->best_mv[0].x += xdelta_left;
      me->best_mv[0].y += ydelta_left;
      me->best_mv[1].x += xdelta_right;
      me->best_mv[1].y += ydelta_right;
    }
#endif
  }
  me->best_mode = mode;
}


void taa_h264_meinfo_init_frame (
  meinfo_t *   me,
  int          width,
  int          height,
  framebuf_t * ref_full,
  framebuf_t * ref_half_diag,
  framebuf_t * ref_half_horz,
  framebuf_t * ref_half_vert,
  int          ref_num)
{
  me->width = width;
  me->height = height;
  me->ref_full = ref_full;
  me->ref_half_diag = ref_half_diag;
  me->ref_half_horz = ref_half_horz;
  me->ref_half_vert = ref_half_vert;
  me->ref_num = ref_num;
}



uint32_t taa_h264_motion_estimate (
  const int     stride,
  meinfo_t *    me,
  const int     qp,
  mv_t *        best_mv1,
  mv_t *        best_mv2,
  mbtype_t *    best_mode,
  const mv_t    pred,
  const mv_t    cand[4],
  const uint8_t mblock_y[256],
  uint8_t       recon_y[256],
  uint8_t       recon_uv[128]
  )
{
  int best_cost;

  mvpos pred_tmp;
  pred_tmp.x = pred.x;
  pred_tmp.y = pred.y;

  const int xref = me->xpos + PAD_HORZ_Y;
  const int yref = me->ypos + PAD_VERT_Y;
  const int cyref = yref >> 1;
  const int cxref = xref >> 1;
  const int cstride = stride >> 1;

  taa_h264_motion_estimate_mb(
    stride,
    cstride,
    qp,
    me->lambda,
    me->thr,
    me,
    xref,
    yref,
    mblock_y,
    cand,
    pred_tmp,
    &me->ref_full->y[yref * stride  + xref],
    &me->ref_full->u[cyref * cstride  + cxref],
    &me->ref_full->v[cyref * cstride  + cxref],
#if HPEL_DIAG_ENABLED
    &me->ref_half_diag->y [yref * stride  + xref],
#endif
    &me->ref_half_horz->y [yref * stride  + xref],
    &me->ref_half_vert->y [yref * stride  + xref],
    recon_y,
    recon_uv
    );

  best_mv1->x = (int16_t) me->best_mv[0].x;
  best_mv1->y = (int16_t) me->best_mv[0].y;
  best_mv2->x = (int16_t) me->best_mv[1].x;
  best_mv2->y = (int16_t) me->best_mv[1].y;
  best_mv1->ref_num = (int8_t) me->ref_num;
  best_mv2->ref_num = (int8_t) me->ref_num;

  *best_mode = me->best_mode;
  best_cost = me->best_cost;

#if SANITY_INTERPOLATION
  mv_t motion_vectors[16] = {0, };
  static void taa_h264_save_motion_vectors (mv_t *,
                                            mv_t *,
                                            mv_t *,
                                            mbtype_t);
  taa_h264_save_motion_vectors (motion_vectors, best_mv1, best_mv2, me->best_mode);

  //==============================================================
  //                      DEBUG: Check LUMA
  //==============================================================
  const int xpos = me->xpos;
  const int ypos = me->ypos;

  uint8_t predbuf_y[MB_SIZE_Y];
  for (int m = 0; m < 4; m++)
  {
    for (int n = 0; n < 4; n++)
    {
      int raster_idx = m * 4 + n;
      int offset_pos = m * 4 * 16 + n * 4;
      const mv_t * mv = &motion_vectors[raster_idx];

      int yvec = (ypos + m * 4) * 4 + mv->y;
      int xvec = (xpos + n * 4) * 4 + mv->x;

      taa_h264_get_block_luma_ref (
        me->ref_full->y, stride, yvec, xvec, &predbuf_y[offset_pos], 16, 4, 4);
    }
  }
  //memcpy(recon_y, predbuf_y, 256);



  TAA_H264_DEBUG_ASSERT (memcmp(predbuf_y, recon_y, MB_SIZE_Y) == 0);

  //==============================================================
  //                      DEBUG: Check CHROMA
  //==============================================================

  const uint8_t * full_u = me->ref_full->u;
  const uint8_t * full_v = me->ref_full->v;
  const int cxpos = me->xpos>>1;
  const int cypos = me->ypos>>1;

  uint8_t predbuf_uv[MB_SIZE_UV * 2];

  for (int uv = 0; uv < 2; uv++)
  {
    const uint8_t * ref_uv = uv ? full_v : full_u;

    for (int i = 0; i < 8; i += 4)
    {
      for (int j = 0; j < 8; j += 4)
      {
        int offset_pos  = i * MB_STRIDE_UV + j;
        uint8_t * pred_ptr = &predbuf_uv[uv * MB_WIDTH_UV + offset_pos];
        int luma_raster_idx = i * 2 + j / 2;
        const mv_t * mv = &motion_vectors[luma_raster_idx];

        taa_h264_get_block_chroma_ref (ref_uv,
                                       cstride,
                                       pred_ptr,
                                       4, 4, MB_STRIDE_UV,
                                       cypos + i, cxpos + j,
                                       mv->y,
                                       mv->x);
      }
    }
  }
  TAA_H264_DEBUG_ASSERT (memcmp(predbuf_uv, recon_uv, MB_SIZE_UV * 2) == 0);

#endif

  return best_cost;
}


meinfo_t * taa_h264_meinfo_create (
void)
{
  meinfo_t * me = TAA_H264_MM_MALLOC (sizeof (meinfo_t), 64);
  return me;
}

void taa_h264_meinfo_delete (
  meinfo_t * me)
{
  TAA_H264_MM_FREE (me);
}



#if SANITY_INTERPOLATION
# define NUM_MVS_MB 16

static void taa_h264_save_motion_vectors (
  mv_t *   motion_vectors,
  mv_t *   mv1,
  mv_t *   mv2,
  mbtype_t mbtype)
{
  switch (mbtype)
  {
  case P_SKIP:
  case P_16x16:
  {
    for (int k = 0; k < NUM_MVS_MB; k++)
    {
      motion_vectors[k].y = (int16_t) mv1->y;
      motion_vectors[k].x = (int16_t) mv1->x;
      motion_vectors[k].ref_num = mv1->ref_num;
    }
    break;
  }
  case P_16x8:
  {
    for (int k = 0; k < 8; k++)
    {
      motion_vectors[k].y = (int16_t) mv1->y;
      motion_vectors[k].x = (int16_t) mv1->x;
      motion_vectors[k].ref_num = mv1->ref_num;

      motion_vectors[k + 8].y = (int16_t) mv2->y;
      motion_vectors[k + 8].x = (int16_t) mv2->x;
      motion_vectors[k + 8].ref_num = mv2->ref_num;
    }
    break;
  }
  case P_8x16:
  {
    for (int k = 0; k < NUM_MVS_MB; k += 4)
    {
      motion_vectors[k].y = (int16_t) mv1->y;
      motion_vectors[k].x = (int16_t) mv1->x;
      motion_vectors[k].ref_num = mv1->ref_num;

      motion_vectors[k + 1].y = (int16_t) mv1->y;
      motion_vectors[k + 1].x = (int16_t) mv1->x;
      motion_vectors[k + 1].ref_num = mv1->ref_num;

      motion_vectors[k + 2].y = (int16_t) mv2->y;
      motion_vectors[k + 2].x = (int16_t) mv2->x;
      motion_vectors[k + 2].ref_num = mv2->ref_num;

      motion_vectors[k + 3].y = (int16_t) mv2->y;
      motion_vectors[k + 3].x = (int16_t) mv2->x;
      motion_vectors[k + 3].ref_num = mv2->ref_num;
    }
    break;
  }
  default:
  {
    break;
  }
  }
}
#endif
